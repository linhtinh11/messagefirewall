/*
 * MFAction.cpp
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#include "MFAction.h"
#include "../common.h"

#include <bb/pim/account/Account>
#include <bb/pim/message/MessageService>
#include <bb/system/phone/Phone>

using namespace bb::pim::message;

MFAction::MFAction():
m_Action(0)
{
    // TODO Auto-generated constructor stub

}

MFAction::MFAction(int action):
        m_Action(action)
{
}

MFAction::~MFAction()
{
    // TODO Auto-generated destructor stub
}

bool MFAction::doAction(MessageService &service, const QVariantMap &data)
{
    bool result = false;

    switch (m_Action) {
        case MF_ACTION_DELETE:
            if (data.contains(ACCOUNT_ID) && data.contains(MESSAGE_ID)) {
                bool ok_acc, ok_msg;
                bb::pim::account::AccountKey accId = data.value(ACCOUNT_ID).toLongLong(&ok_acc);
                MessageKey msgId = data.value(MESSAGE_ID).toLongLong(&ok_msg);
                if (ok_acc && ok_msg) {
                    service.remove(accId, msgId);
                    result = true;
                }
                if (data.contains(CONVERSATION_ID)) {
                    ConversationKey conKey = data.value(CONVERSATION_ID).toString();
                    if (ok_acc) {
                        service.remove(accId, conKey);
                    }
                }
            }
            break;
        default:
            break;
    }

    return result;
}

bool MFAction::doAction(bb::system::phone::Phone &phone, const QVariantMap &data)
{
    bool result = false;

    switch (m_Action) {
        case MF_ACTION_ENDCALL:
            if (data.contains(CALL_ID)) {
                bool ok_call;
                int callId = data.value(CALL_ID).toInt(&ok_call);
                if (ok_call) {
                    phone.endCall(callId);
                    result = true;
                }
            }
            break;
        default:
            break;
    }

    return result;
}
