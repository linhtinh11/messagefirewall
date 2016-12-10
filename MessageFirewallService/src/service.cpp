/*
 * Copyright (c) 2013-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "service.hpp"
#include "models/MFAction.h"
#include "models/MFCondition.h"

#include <bb/Application>
#include <bb/system/InvokeManager>
#include <bb/data/JsonDataAccess>
#include <bb/pim/message/MessageService>
#include <bb/pim/message/MessageContact>
#include <bb/pim/message/MessageBuilder>
#include <bb/pim/message/Message>
#include <bb/pim/account/AccountService>
#include <bb/pim/message/MimeTypes>
#include <bb/pim/contacts/ContactService>
#include <bb/pim/contacts/Contact>

#include <QTimer>

using namespace bb::system;
using namespace bb::data;
using namespace bb::pim::message;
using namespace bb::pim::contacts;

Service::Service() :
        QObject(),
        m_invokeManager(new InvokeManager(this)),
        m_MessageService(new MessageService()),
        m_AccountService(new bb::pim::account::AccountService()),
        m_ContactService(new bb::pim::contacts::ContactService())
{
    // invoke for email
    m_invokeManager->connect(m_invokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)),
            this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

    // invoke for sms
    connect(m_MessageService,
                SIGNAL(messageAdded(bb::pim::account::AccountKey, bb::pim::message::ConversationKey, bb::pim::message::MessageKey)),
                SLOT(messageReceived(bb::pim::account::AccountKey, bb::pim::message::ConversationKey, bb::pim::message::MessageKey)));

    qRegisterMetaType<QList<int> >("QList<int>");
    // invoke for contact
    connect(m_ContactService,
            SIGNAL(contactsAdded(QList<int>)),
            SLOT(mfContactsAdded(QList<int>)));
    connect(m_ContactService,
            SIGNAL(contactsChanged(QList<int>)),
            SLOT(mfContactsChanged(QList<int>)));
    connect(m_ContactService,
            SIGNAL(contactsDeleted(QList<int>)),
            SLOT(mfContactsDeleted(QList<int>)));
    connect(m_ContactService,
            SIGNAL(contactsReset()),
            SLOT(mfContactsReset()));

    // get sms account id
    qDebug() << "MF getting SMS-MMS account";
    QList<bb::pim::account::Account> account_list = m_AccountService->accounts(bb::pim::account::Service::Messages, "sms-mms");
    qDebug() << "MF account size: " << account_list.size();
    m_SmsAccountId = 0;
    if (account_list.size() > 0) {
        m_SmsAccountId = account_list.first().id();
        qDebug() << "MF got SMS-MMS account. id=" << m_SmsAccountId;
    }

    // built-in email rules
    MFRule rule(MF_RULE_EMAIL);
    rule.addCondition(MFCondition(MF_KEY_SENDER, "no-reply@backlog.jp", MF_OPERATOR_EQ, MF_OPERATOR_AND));
    rule.addAction(MFAction(MF_ACTION_DELETE));
    MFRule rule2(MF_RULE_EMAIL);
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[BEAT-", MF_OPERATOR_SW, MF_OPERATOR_AND));
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[BEAT] ", MF_OPERATOR_SW, MF_OPERATOR_OR));
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[LGBT-", MF_OPERATOR_SW, MF_OPERATOR_OR));
    rule2.addAction(MFAction(MF_ACTION_DELETE));
    m_RulesEmail.append(rule);
    m_RulesEmail.append(rule2);

    // built-in sms rules
    MFRule sRule1(MF_RULE_SMS);
    // set action
    sRule1.addAction(MFAction(MF_ACTION_DELETE));
    // set condition
    MFCondition con(MF_KEY_SENDER, "", MF_OPERATOR_NINW, MF_OPERATOR_AND);
    buildWhiteListContact(con);
    sRule1.addCondition(con);
    m_RulesSms.append(sRule1);
}

void Service::buildWhiteListContact(MFCondition &con)
{
    QList<Contact> contactPage;
    ContactListFilters options;
    const int maxLimit = 20;
    options.setLimit(maxLimit);
    do {
        contactPage = m_ContactService->contacts(options);
        QList<Contact>::iterator it;
        for (it=contactPage.begin(); it!=contactPage.end(); it++) {
            addContactToCondition(it->id(), con);
        }
        if (contactPage.size() == maxLimit) {
            options.setAnchorId(contactPage[maxLimit-1].id());
        } else {
            break;
        }
    } while (true);
}

void Service::addContactToCondition(int smsId, MFCondition &con)
{
    Contact ct;
    ct = m_ContactService->contactDetails(smsId);
    QList<ContactAttribute> attrs = ct.phoneNumbers();
    qDebug() << "MF contact: " << ct.displayName() << " phones count: " << attrs.size();
    QList<ContactAttribute>::iterator ait;
    for (ait=attrs.begin(); ait!=attrs.end(); ait++) {
        QString no = ait->value();
        if (no.startsWith("+")) {
            con.addWhiteList(no);
        } else if (no.startsWith("0")) {
            no = "+84" + no.remove(0, 1);
            con.addWhiteList(no);
        } else {
            no = "+84" + no;
            con.addWhiteList(no);
        }
        qDebug() << " with phone: " << no;
    }
}

/*
 * this function handles received emails
 */
void Service::handleInvoke(const bb::system::InvokeRequest & request)
{
    if (request.action().compare("bb.action.email.RECEIVED") == 0) {
        // get request data
        QByteArray data = request.data();
#ifdef QT_DEBUG
        QString str = QString::fromUtf8(data.data());
        qDebug() << "full json: " << str;
#endif
        JsonDataAccess jda;
        QVariant obj = jda.loadFromBuffer(data);
        if (!jda.hasError()) {
            QVariantMap map = obj.toMap();
            bool ok;
            // get account id
            bb::pim::account::AccountKey accId = map.value("account_id").toLongLong(&ok);
            if (!ok) {
                // cannot get account id, ignore all
                return;
            }

            // get message ids
            QVariantList list = map.value("trigger_data").toList();
            QVariantList::iterator i;
            for (i = list.begin(); i != list.end(); i++) {
                QVariantList list2 = i->toList();
                // data format [msgId, convId, unknown_number]
                // get message id
                MessageKey msgId = list2.at(0).toLongLong(&ok);
                if (!ok) {
                    // cannot get message id, ignore it
                    continue;
                }
                // get message info
                Message msg = m_MessageService->message(accId, msgId);
                QList<MFRule>::iterator iRule;
                // loop through the rules
                for (iRule = m_RulesEmail.begin(); iRule != m_RulesEmail.end(); iRule++) {
                    // apply rule on the message
                    iRule->apply(*m_MessageService, msg);
                }
            }
        }
    }
}

void Service::messageReceived(bb::pim::account::AccountKey account_key, bb::pim::message::ConversationKey conv, bb::pim::message::MessageKey message_key)
{
    qDebug() << "MF message received";
    Q_UNUSED(conv);
    if (m_SmsAccountId == 0 && m_SmsAccountId != account_key) {
        // it's not sms account
        qDebug() << "MF it's not sms";
        return;
    }

    // get msg info
    Message message = m_MessageService->message(account_key, message_key);
    qDebug() << "MF Message mime type:" << message.mimeType();
    if (message.mimeType() != MimeTypes::Sms || !message.isInbound()) {
        qDebug() << "MF It's not an SMS!!! or not an incoming sms";
        return;
    }

    // it's an incoming sms message
    qDebug() << "MF sender displayableName:" << message.sender().displayableName();
    qDebug() << "MF sender address:" << message.sender().address();
    qDebug() << "MF sender name:" << message.sender().name();
    QList<MFRule>::iterator iRule;
    // loop through the rules
    for (iRule = m_RulesSms.begin(); iRule != m_RulesSms.end(); iRule++) {
        // apply rule on the message
        iRule->apply(*m_MessageService, message);
    }
}

void Service::mfContactsReset()
{
    qDebug() << "MF contact reset";
    QList<MFRule>::iterator iRule;
    // loop through the rules
    for (iRule = m_RulesSms.begin(); iRule != m_RulesSms.end(); iRule++) {
        QList<MFCondition> cons = iRule->getConditions();
        QList<MFCondition>::iterator it;
        for (it=cons.begin(); it!=cons.end(); it++) {
            if (it->getOperator() == MF_OPERATOR_NINW) {
                qDebug() << "MF reset white list";
                // reset white list
                it->resetWhiteList();
                // rebuild white list
                buildWhiteListContact(*it);
            }
        }
    }
}

void Service::mfContactsDeleted(QList<int> contactIds)
{
    qDebug() << "MF contact changed";
    Q_UNUSED(contactIds);
    mfContactsReset();
}

void Service::mfContactsChanged(QList<int> contactIds)
{
    qDebug() << "MF contact changed";
    Q_UNUSED(contactIds);
    mfContactsReset();
}

void Service::mfContactsAdded(QList<int> contactIds)
{
    qDebug() << "MF contact added";
    QList<MFRule>::iterator iRule;
    // loop through the rules
    for (iRule = m_RulesSms.begin(); iRule != m_RulesSms.end(); iRule++) {
        QList<MFCondition> cons = iRule->getConditions();
        QList<MFCondition>::iterator it;
        for (it=cons.begin(); it!=cons.end(); it++) {
            if (it->getOperator() == MF_OPERATOR_NINW) {
                // add more item to white list
                QList<int>::iterator cit;
                for (cit=contactIds.begin(); cit!=contactIds.end(); cit++) {
                    qDebug() << "MF add new contact";
                    addContactToCondition(*cit, *it);
                }
            }
        }
    }
}
