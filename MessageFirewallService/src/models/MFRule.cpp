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

using namespace bb::pim::message;

MFRule::MFRule()
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
                iAct->doAction(service, data);
            }
        }
    }

    return result;
}
