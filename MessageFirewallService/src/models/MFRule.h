/*
 * MFRule.h
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#ifndef MFRULE_H_
#define MFRULE_H_

#include <QObject>

class MFAction;
class MFCondition;
namespace bb {
    namespace system {
        namespace phone {
            class Phone;
            class Call;
        }
    }
    namespace pim {
        namespace message {
            class MessageService;
            class Message;
        }
    }
}

enum MF_RULE_TYPE {
    MF_RULE_EMAIL = 0,
    MF_RULE_SMS,
    MF_RULE_PHONE,
};

class MFRule
{
public:
    MFRule(int type);
    virtual ~MFRule();
    void addCondition(const MFCondition &con);
    void addAction(const MFAction &action);
    bool apply(bb::pim::message::MessageService &service, bb::pim::message::Message &message);
    bool apply(bb::system::phone::Phone &phone, const bb::system::phone::Call &call);
    QList<MFCondition> getConditions();

private:
    QList<MFCondition> m_Conditions;
    QList<MFAction> m_Actions;
    int m_Type;
};

#endif /* MFRULE_H_ */
