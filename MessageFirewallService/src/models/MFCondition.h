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
    MF_OPERATOR_EQ = 0,     // equal
    MF_OPERATOR_NEQ,        // not equal
    MF_OPERATOR_CT,         // contain
    MF_OPERATOR_NCT,        // not contain
    MF_OPERATOR_AND,        // and
    MF_OPERATOR_OR,         // or
    MF_OPERATOR_SW,         // start with
    MF_OPERATOR_EW,         // end with
    MF_OPERATOR_NINW,       // not in white list
    MF_OPERATOR_INW,        // in white list
    MF_OPERATOR_NINB,       // not in black list
    MF_OPERATOR_INB,        // in black list
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
    void addWhiteList(const QList<QString> &list);
    void addWhiteList(const QString &value);
    void resetWhiteList();
    void addBlackList(const QList<QString> &list);
    void addBlackList(const QString &value);
    void resetBlackList();
    int getOperator();

private:
    int m_Key;
    int m_Operator;
    int m_CondOperator;
    QString m_Value;
    QList<QString> m_WhiteList;
    QList<QString> m_BlackList;

private:
    bool doOperator(const QString &value);
    bool isInWhiteList(const QString &value);
    bool isNotInWhiteList(const QString &value);
    void bfContain(const QString &value);
    void bfBuild();
};

#endif /* MFCONDITION_H_ */
