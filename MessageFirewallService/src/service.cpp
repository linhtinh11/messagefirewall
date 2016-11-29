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
#include <bb/pim/account/Account>

#include <QTimer>

using namespace bb::system;
using namespace bb::data;
using namespace bb::pim::message;

Service::Service() :
        QObject(),
        m_invokeManager(new InvokeManager(this))
{
    m_invokeManager->connect(m_invokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)),
            this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));
    MFRule rule;
    rule.addCondition(MFCondition(MF_KEY_SENDER, "no-reply@backlog.jp", MF_OPERATOR_EQ, MF_OPERATOR_AND));
    rule.addAction(MFAction(MF_ACTION_DELETE));
    MFRule rule2;
    rule2.addCondition(MFCondition(MF_KEY_SUBJECT, "[BEAT-", MF_OPERATOR_SW, MF_OPERATOR_AND));
    rule2.addAction(MFAction(MF_ACTION_DELETE));
    m_Rules.append(rule);
    m_Rules.append(rule2);

}

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
            MessageService service;
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
                Message msg = service.message(accId, msgId);
                QList<MFRule>::iterator iRule;
                for (iRule = m_Rules.begin(); iRule != m_Rules.end(); iRule++) {
                    iRule->apply(service, msg);
                }
            }
        }
    }
}
