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
#include <bb/system/phone/Phone>
#include <QSettings>

#include <QTimer>

using namespace bb::system;
using namespace bb::data;
using namespace bb::pim::message;
using namespace bb::pim::contacts;
using namespace bb::system::phone;

const QString Service::m_author = "ht"; // for creating settings
const QString Service::m_appName = "MessageFirewall"; // for creating settings

// keys for setting file
const QString Service::m_s_activeEmail = "ActiveEmail";
const QString Service::m_s_activeSMS = "ActiveSMS";
const QString Service::m_s_activePhone = "ActivePhone";

Service::Service() :
        QObject(),
        m_invokeManager(new InvokeManager(this)),
        m_MessageService(new MessageService()),
        m_Phone(new Phone()),
        m_settingsWatcher(new QFileSystemWatcher(this))
{
    // read settings file
    // set the initial qsettings keys/values upon startup
    QSettings settings(m_author, m_appName);
    settings.setValue(m_s_activeEmail, true);
    settings.setValue(m_s_activeSMS, true);
    settings.setValue(m_s_activePhone, true);
    settings.sync();
    readSettings(settings);
    // Watcher for changes in the settings file.
    qDebug() << "settings file: " << settings.fileName();
    m_settingsWatcher->addPath(settings.fileName());
    connect(m_settingsWatcher,
            SIGNAL(fileChanged(const QString&)),
            SLOT(settingsChanged(const QString&)));

    // invoke for email
    m_invokeManager->connect(m_invokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)),
            this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

    // invoke for sms
    connect(m_MessageService,
                SIGNAL(messageAdded(bb::pim::account::AccountKey, bb::pim::message::ConversationKey, bb::pim::message::MessageKey)),
                SLOT(messageReceived(bb::pim::account::AccountKey, bb::pim::message::ConversationKey, bb::pim::message::MessageKey)));

    // invoke for phone call
    connect(m_Phone,
            SIGNAL(callUpdated(bb::system::phone::Call)),
            SLOT(callReceived(bb::system::phone::Call)));

    // get sms account id
    qDebug() << "MF getting SMS-MMS account";
    bb::pim::account::AccountService accService;
    QList<bb::pim::account::Account> account_list = accService.accounts(bb::pim::account::Service::Messages, "sms-mms");
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
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[LGBT] ", MF_OPERATOR_SW, MF_OPERATOR_OR));
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[DAITO_LOOPS-", MF_OPERATOR_SW, MF_OPERATOR_OR));
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[DAITO_LOOPS] ", MF_OPERATOR_SW, MF_OPERATOR_OR));
    rule2.addAction(MFAction(MF_ACTION_DELETE));
    m_RulesEmail.append(rule);
    m_RulesEmail.append(rule2);

    // built-in sms rules
    MFRule sRule1(MF_RULE_SMS);
    // set condition
    sRule1.addCondition(MFCondition(MF_KEY_SENDER, "", MF_OPERATOR_NINCT, MF_OPERATOR_AND));
    MFCondition whiteList = MFCondition(MF_KEY_SENDER, "", MF_OPERATOR_NINWL, MF_OPERATOR_AND);
    whiteList.addWhiteList("Apple");
    sRule1.addCondition(whiteList);
    // set action
    sRule1.addAction(MFAction(MF_ACTION_DELETE));
    m_RulesSms.append(sRule1);

    MFRule sRule2(MF_RULE_SMS);
    sRule2.addCondition(MFCondition(MF_KEY_SENDER_NAME, "spammer", MF_OPERATOR_EQ, MF_OPERATOR_AND));
    sRule2.addAction(MFAction(MF_ACTION_DELETE));
    m_RulesSms.append(sRule2);

    // built-in phone call rules
    MFRule pRule1(MF_RULE_PHONE);
    pRule1.addCondition(MFCondition(MF_KEY_NUMBER, "spammer", MF_OPERATOR_BLTOACC, MF_OPERATOR_AND));
    pRule1.addAction(MFAction(MF_ACTION_ENDCALL));
    m_RulesPhoneCall.append(pRule1);
}

void Service::buildWhiteListContact(MFCondition &con)
{
    Q_UNUSED(con);
//    QList<Contact> contactPage;
//    ContactListFilters options;
//    const int maxLimit = 20;
//    options.setLimit(maxLimit);
//    do {
//        contactPage = m_ContactService->contacts(options);
//        QList<Contact>::iterator it;
//        for (it=contactPage.begin(); it!=contactPage.end(); it++) {
//            addContactToCondition(it->id(), con);
//        }
//        if (contactPage.size() == maxLimit) {
//            options.setAnchorId(contactPage[maxLimit-1].id());
//        } else {
//            break;
//        }
//    } while (true);
}

void Service::addContactToCondition(int smsId, MFCondition &con)
{
    Q_UNUSED(smsId);
    Q_UNUSED(con);
//    Contact ct;
//    ct = m_ContactService->contactDetails(smsId);
//    QList<ContactAttribute> attrs = ct.phoneNumbers();
//    qDebug() << "MF contact: " << ct.displayName() << " phones count: " << attrs.size();
//    QList<ContactAttribute>::iterator ait;
//    for (ait=attrs.begin(); ait!=attrs.end(); ait++) {
//        QString no = ait->value();
//        if (no.startsWith("+")) {
//            con.addWhiteList(no);
//        } else if (no.startsWith("0")) {
//            no = "+84" + no.remove(0, 1);
//            con.addWhiteList(no);
//        } else {
//            con.addWhiteList(no);
//        }
//        qDebug() << " with phone: " << no;
//    }
}

/*
 * this function handles received emails
 */
void Service::handleInvoke(const bb::system::InvokeRequest & request)
{
    if (request.action().compare("bb.action.email.RECEIVED") == 0) {
        // actived email firewall or not
        if (!m_activatedEmail) {
            return;
        }

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
                applyEmailRules(msg);
            }
        }
    }
}

void Service::messageReceived(bb::pim::account::AccountKey account_key, bb::pim::message::ConversationKey conv, bb::pim::message::MessageKey message_key)
{
    qDebug() << "MF message received";
    Q_UNUSED(conv);
    if (m_SmsAccountId == account_key) {
        // get msg info
        Message message = m_MessageService->message(account_key, message_key);
        qDebug() << "MF Message mime type:" << message.mimeType();
        // it's not sms account
        qDebug() << "MF it's sms account";
        if (message.mimeType() == MimeTypes::Sms && message.isInbound()) {
            qDebug() << "MF It's a SMS!!! and an incoming sms";
            applySmsRules(message);
        }
    } else {
//        qDebug() << "MF it may be email account";
//        if (message.isInbound()) {
//            qDebug() << "MF it may be incoming email";
//            applyEmailRules(message);
//        }
    }
}

void Service::applyEmailRules(bb::pim::message::Message msg)
{
    QList<MFRule>::iterator iRule;
    // loop through the rules
    for (iRule = m_RulesEmail.begin(); iRule != m_RulesEmail.end(); iRule++) {
        // apply rule on the message
        iRule->apply(*m_MessageService, msg);
    }
}

void Service::applySmsRules(bb::pim::message::Message msg)
{
    // it's an incoming sms message
    qDebug() << "MF sender displayableName:" << msg.sender().displayableName();
    qDebug() << "MF sender address:" << msg.sender().address();
    qDebug() << "MF sender name:" << msg.sender().name();

    // actived sms firewall or not
    if (!m_activatedSMS) {
        return;
    }

    QList<MFRule>::iterator iRule;
    // loop through the rules
    for (iRule = m_RulesSms.begin(); iRule != m_RulesSms.end(); iRule++) {
        // apply rule on the message
        iRule->apply(*m_MessageService, msg);
    }
}

void Service::callReceived(const bb::system::phone::Call &call)
{
    qDebug() << "MF call from:" << call.phoneNumber();
    // actived phone firewall or not
    if (!m_activatedPhone) {
        return;
    }

    if (call.callLine() == LineType::Cellular
            && call.callType() == CallType::Incoming
            && call.phoneNumber().length() > 0) {
        qDebug() << "MF it's an incoming call on cellular line";
        QList<MFRule>::iterator iRule;
        // loop through the rules
        for (iRule = m_RulesPhoneCall.begin(); iRule != m_RulesPhoneCall.end(); iRule++) {
            // apply rule on the call
            iRule->apply(*m_Phone, call);
        }
    }
}

void Service::settingsChanged(const QString & path)
{
    qDebug() << "MF settings file changed: " << path;
    QSettings settings(m_author, m_appName);
    readSettings(settings);
}

void Service::readSettings(const QSettings &settings)
{
    m_activatedEmail = settings.value(m_s_activeEmail, true).toBool();
    m_activatedSMS = settings.value(m_s_activeSMS, true).toBool();
    m_activatedPhone = settings.value(m_s_activePhone, true).toBool();
    qDebug() << "active email: " << m_activatedEmail;
    qDebug() << "active sms: " << m_activatedSMS;
    qDebug() << "active phone: " << m_activatedPhone;
}
