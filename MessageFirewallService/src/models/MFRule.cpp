/*
 * MFRule.cpp
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#include "MFRule.h"
#include "../common.h"
#include "MFCondition.h"
#include "MFAction.h"
#include <bb/pim/message/MessageService>
#include <bb/pim/message/Message>
#include <bb/system/phone/Phone>

using namespace bb::pim::message;
using namespace bb::system::phone;

MFRule::MFRule(int type):
        m_Type(type)
{
    // TODO Auto-generated constructor stub

}

MFRule::~MFRule()
{
    // TODO Auto-generated destructor stub
}

void MFRule::addCondition(const MFCondition &con)
{
    m_Conditions.append(con);
}

void MFRule::addAction(const MFAction &action)
{
    m_Actions.append(action);
}

QList<MFCondition> MFRule::getConditions()
{
    return m_Conditions;
}

bool MFRule::apply(MessageService &service, Message &message)
{
    bool result = false;

    if (m_Conditions.size() > 0 && m_Actions.size() > 0) {
        QList<MFCondition>::iterator iCon;
        bool matchCons = true, tmp;
        for (iCon = m_Conditions.begin(); iCon != m_Conditions.end(); iCon++) {
            tmp = iCon->match(message);
            if (iCon == m_Conditions.begin()) {
                matchCons = tmp;
            } else {
                if (iCon->isAndCondition()) {
                    matchCons = matchCons && tmp;
                } else {
                    matchCons = matchCons || tmp;
                }
            }
        }
        if (matchCons) {
            QList<MFAction>::iterator iAct;
            for (iAct = m_Actions.begin(); iAct != m_Actions.end(); iAct++) {
                QVariantMap data;
                data.insert(ACCOUNT_ID, QVariant(message.accountId()));
                data.insert(MESSAGE_ID, QVariant(message.id()));
                if (m_Type == MF_RULE_SMS) {
                    data.insert(CONVERSATION_ID, QVariant(message.conversationId()));
                }
                result = iAct->doAction(service, data);
            }
        }
    }

    return result;
}

bool MFRule::apply(bb::system::phone::Phone &phone, const bb::system::phone::Call &call)
{
    bool result = false;

    if (m_Conditions.size() > 0 && m_Actions.size() > 0) {
        QList<MFCondition>::iterator iCon;
        bool matchCons = true, tmp;
        for (iCon = m_Conditions.begin(); iCon != m_Conditions.end(); iCon++) {
            tmp = iCon->match(call);
            if (iCon == m_Conditions.begin()) {
                matchCons = tmp;
            } else {
                if (iCon->isAndCondition()) {
                    matchCons = matchCons && tmp;
                } else {
                    matchCons = matchCons || tmp;
                }
            }
        }
        if (matchCons) {
            QList<MFAction>::iterator iAct;
            for (iAct = m_Actions.begin(); iAct != m_Actions.end(); iAct++) {
                QVariantMap data;
                data.insert(CALL_ID, QVariant(call.callId()));
                result = iAct->doAction(phone, data);
            }
        }
    }

    return result;
}
