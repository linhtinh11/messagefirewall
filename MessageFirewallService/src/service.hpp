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
#include <QFileSystemWatcher>

namespace bb {
    class Application;

    namespace system {
        class InvokeManager;
        class InvokeRequest;
        namespace phone {
            class Phone;
            class Call;
        }
    }
    namespace pim {
        namespace message {
            class MessageService;
            class Messsage;
        }
    }
}
class QSettings;

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
    void callReceived(const bb::system::phone::Call &call);
    void settingsChanged(const QString & path);

private:
    bb::system::InvokeManager *m_invokeManager;
    bb::pim::message::MessageService *m_MessageService;
    bb::system::phone::Phone *m_Phone;
    QFileSystemWatcher* m_settingsWatcher;

    int m_SmsAccountId;
    bool m_activatedEmail;
    bool m_activatedSMS;
    bool m_activatedPhone;

    QList<MFRule> m_RulesEmail;
    QList<MFRule> m_RulesSms;
    QList<MFRule> m_RulesPhoneCall;

    // settings init
    static const QString m_author; // for creating settings
    static const QString m_appName; // for creating settings

    // keys used in setting
    static const QString m_s_activeEmail;
    static const QString m_s_activeSMS;
    static const QString m_s_activePhone;

    void addContactToCondition(int smsId, MFCondition &con);
    void buildWhiteListContact(MFCondition &con);
    void applyEmailRules(bb::pim::message::Message msg);
    void applySmsRules(bb::pim::message::Message msg);
    void readSettings(const QSettings &settings);
};

#endif /* SERVICE_H_ */
