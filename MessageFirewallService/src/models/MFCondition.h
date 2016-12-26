/*
 * MFCondition.h
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#ifndef MFCONDITION_H_
#define MFCONDITION_H_

#include <QObject>
#include <QHash>

namespace bb {
    namespace system {
        namespace phone {
            class Call;
        }
    }
    namespace pim {
        namespace message {
            class Message;
        }
        namespace contacts {
            class ContactService;
        }
    }
}

enum MF_OPERATOR {
    MF_OPERATOR_EQ = 0,     // equal
    MF_OPERATOR_NEQ,        // not equal
    MF_OPERATOR_CT,         // contain
    MF_OPERATOR_NCT,        // not contain
    MF_OPERATOR_AND,        // and
    MF_OPERATOR_OR,         // or
    MF_OPERATOR_SW,         // start with
    MF_OPERATOR_EW,         // end with
    MF_OPERATOR_NINCT,      // not in contact
    MF_OPERATOR_INCT,       // in contact
    MF_OPERATOR_NINWL,      // not in white list
    MF_OPERATOR_INWL,       // in white list
    MF_OPERATOR_NINBL,      // not in black list
    MF_OPERATOR_INBL,       // in black list
    MF_OPERATOR_BLTOACC,    // belong to specified account
};

enum MF_KEY {
    MF_KEY_SENDER = 0,
    MF_KEY_SUBJECT,
    MF_KEY_NUMBER,
};

class MFCondition
{
public:
    MFCondition();
    MFCondition(MF_KEY key, QString value, MF_OPERATOR op, MF_OPERATOR conOp);
    virtual ~MFCondition();
    bool match(const bb::pim::message::Message &message);
    bool match(const bb::system::phone::Call &call);
    bool isAndCondition();
    void addWhiteList(const QString &value);
    void resetWhiteList();
    void addBlackList(const QString &value);
    void resetBlackList();
    int getOperator();

private:
    int m_Key;
    int m_Operator;
    int m_CondOperator;
    QString m_Value;
    QHash<QString, QString> m_WhiteList;
    QHash<QString, QString> m_BlackList;
    static bb::pim::contacts::ContactService *m_ContactService;

private:
    bool doOperator(const QString &value);
    bool isInWhiteList(const QString &value);
    bool isInBlackList(const QString &value);
    bool isInContact(const QString &value);
    bool belongToAccount(const QString &value);
};

#endif /* MFCONDITION_H_ */
