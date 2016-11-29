/*
 * MFCondition.h
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#ifndef MFCONDITION_H_
#define MFCONDITION_H_

#include <QObject>

namespace bb {
    namespace pim {
        namespace message {
            class Message;
        }
    }
}

enum MF_OPERATOR {
    MF_OPERATOR_EQ = 0,
    MF_OPERATOR_NEQ,
    MF_OPERATOR_CT,
    MF_OPERATOR_NCT,
    MF_OPERATOR_AND,
    MF_OPERATOR_OR,
    MF_OPERATOR_SW,
    MF_OPERATOR_EW
};

enum MF_KEY {
    MF_KEY_SENDER = 0,
    MF_KEY_SUBJECT
};

class MFCondition
{
public:
    MFCondition();
    MFCondition(MF_KEY key, QString value, MF_OPERATOR op, MF_OPERATOR conOp);
    virtual ~MFCondition();
    bool match(const bb::pim::message::Message &message);
    bool isAndCondition();

private:
    int m_Key;
    int m_Operator;
    int m_CondOperator;
    QString m_Value;

private:
    bool doOperator(const QString &value);
};

#endif /* MFCONDITION_H_ */
