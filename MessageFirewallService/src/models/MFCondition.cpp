/*
 * MFCondition.cpp
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#include "MFCondition.h"

#include <bb/pim/message/MessageContact>
#include <bb/pim/message/Message>
#include <bb/pim/contacts/ContactService>
#include <bb/pim/contacts/Contact>

using namespace bb::pim::message;
using namespace bb::pim::contacts;

MFCondition::MFCondition():
            m_Key(0),
            m_Operator(MF_OPERATOR_EQ),
            m_CondOperator(MF_OPERATOR_AND),
            m_Value("")
{
    // TODO Auto-generated constructor stub
    m_ContactService = NULL;
}

MFCondition::MFCondition(MF_KEY key, QString value, MF_OPERATOR op, MF_OPERATOR conOp):
        m_Key(key),
        m_Operator(op),
        m_CondOperator(conOp),
        m_Value(value)
{
    m_ContactService = NULL;
}

MFCondition::~MFCondition()
{
    // TODO Auto-generated destructor stub
    if (m_ContactService != NULL) {
        delete m_ContactService;
        m_ContactService = NULL;
    }
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
        case MF_OPERATOR_NINCT:
            result = !isInContact(value);
            break;
        case MF_OPERATOR_INCT:
            result = isInContact(value);
            break;
        case MF_OPERATOR_NINWL:
            result = !isInWhiteList(value);
            break;
        case MF_OPERATOR_INWL:
            result = isInWhiteList(value);
            break;
        case MF_OPERATOR_NINBL:
            result = !isInBlackList(value);
            break;
        case MF_OPERATOR_INBL:
            result = isInBlackList(value);
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

void MFCondition::addWhiteList(const QString &value)
{
    m_WhiteList.insert(value, "");
    qDebug() << "count: " << m_WhiteList.size();
}

void MFCondition::resetWhiteList()
{
    m_WhiteList.clear();
}

void MFCondition::addBlackList(const QString &value)
{
    m_BlackList.insert(value, "");
}

void MFCondition::resetBlackList()
{
    m_BlackList.clear();
}

bool MFCondition::isInWhiteList(const QString &value)
{
    qDebug() << "check value: " << value;
    qDebug() << "count: " << m_WhiteList.size();
    return m_WhiteList.contains(value);
}

bool MFCondition::isInBlackList(const QString &value)
{
    qDebug() << "check value: " << value;
    qDebug() << "count: " << m_WhiteList.size();
    return m_BlackList.contains(value);
}

bool MFCondition::isInContact(const QString &value)
{
    if (m_ContactService == NULL) {
        m_ContactService = new ContactService();
    }
    ContactSearchFilters filter;
    filter.setSearchValue(value);
    return (m_ContactService->searchContactsByPhoneNumber(filter).size() > 0);
}

int MFCondition::getOperator()
{
    return m_Operator;
}
