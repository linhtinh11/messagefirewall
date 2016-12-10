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

#ifndef SERVICE_H_
#define SERVICE_H_

#include <QObject>
#include "models/MFRule.h"
#include <bb/pim/message/Keys>
#include <bb/pim/account/Account>

namespace bb {
    class Application;
    namespace system {
        class InvokeManager;
        class InvokeRequest;
    }
    namespace pim {
        namespace message {
            class MessageService;
        }
        namespace account {
            class AccountService;
        }
        namespace contacts {
            class ContactService;
        }
    }
}

class Service: public QObject
{
    Q_OBJECT
public:
    Service();
    virtual ~Service() {}
private slots:
    // email received
    void handleInvoke(const bb::system::InvokeRequest &);
    // sms received
    void messageReceived(bb::pim::account::AccountKey account_key, bb::pim::message::ConversationKey conv, bb::pim::message::MessageKey message_key);
    // contact handlers
    void mfContactsReset();
    void mfContactsDeleted(QList<int> contactIds);
    void mfContactsChanged(QList<int> contactIds);
    void mfContactsAdded(QList<int> contactIds);

private:
    bb::system::InvokeManager *m_invokeManager;
    bb::pim::message::MessageService *m_MessageService;
    bb::pim::account::AccountService *m_AccountService;
    bb::pim::contacts::ContactService *m_ContactService;

    int m_SmsAccountId;

    QList<MFRule> m_RulesEmail;
    QList<MFRule> m_RulesSms;

    void addContactToCondition(int smsId, MFCondition &con);
    void buildWhiteListContact(MFCondition &con);
};

#endif /* SERVICE_H_ */
