/*
 * MFCondition.cpp
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#include "MFCondition.h"

#include <bb/pim/message/MessageContact>
#include <bb/pim/message/Message>

using namespace bb::pim::message;

MFCondition::MFCondition():
            m_Key(0),
            m_Operator(MF_OPERATOR_EQ),
            m_CondOperator(MF_OPERATOR_AND),
            m_Value("")
{
    // TODO Auto-generated constructor stub

}

MFCondition::MFCondition(MF_KEY key, QString value, MF_OPERATOR op, MF_OPERATOR conOp):
        m_Key(key),
        m_Operator(op),
        m_CondOperator(conOp),
        m_Value(value)
{
}

MFCondition::~MFCondition()
{
    // TODO Auto-generated destructor stub
}

bool MFCondition::match(const Message &message)
{
    bool result = false;

    switch (m_Key) {
        case MF_KEY_SENDER:
        {
            MessageContact sender = message.sender();
            result = doOperator(sender.address());
        }
            break;
        case MF_KEY_SUBJECT:
            {
                result = doOperator(message.subject());
            }
                break;
        default:
            qDebug() << "Invalid key: " << m_Key;
            break;
    }

    return result;
}

bool MFCondition::doOperator(const QString &value)
{
    bool result = false;

    switch (m_Operator) {
        case MF_OPERATOR_EQ:
            if (value.compare(m_Value) == 0) {
                result = true;
            }
            break;
        case MF_OPERATOR_NEQ:
            if (value.compare(m_Value) == 0) {
                result = true;
            }
            break;
        case MF_OPERATOR_SW:
            if (value.startsWith(m_Value)) {
                result = true;
            }
            break;
        case MF_OPERATOR_EW:
            if (value.endsWith(m_Value)) {
                result = true;
            }
            break;
        case MF_OPERATOR_CT:
            if (value.contains(m_Value)) {
                result = true;
            }
            break;
        case MF_OPERATOR_NCT:
            if (!value.contains(m_Value)) {
                result = true;
            }
            break;
        default:
            qDebug() << "Invalid operator: " << m_Operator;
            break;
    }

    return result;
}

bool MFCondition::isAndCondition()
{
    return (m_CondOperator == MF_OPERATOR_AND);
}
