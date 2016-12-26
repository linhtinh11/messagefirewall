/*
 * MFAction.h
 *
 *  Created on: Nov 27, 2016
 *      Author: trung
 */

#ifndef MFACTION_H_
#define MFACTION_H_

#include <QVariant>

enum MF_ACTION {
    MF_ACTION_DELETE = 0,
    MF_ACTION_ENDCALL,
};

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
        }
    }
}

class MFAction
{
public:
    MFAction();
    MFAction(int action);
    virtual ~MFAction();
    bool doAction(bb::pim::message::MessageService &service, const QVariantMap &data);
    bool doAction(bb::system::phone::Phone &phone, const QVariantMap &data);

private:
    int m_Action;
};

#endif /* MFACTION_H_ */
