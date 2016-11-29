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
    MF_ACTION_DELETE = 0
};

namespace bb {
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

private:
    int m_Action;
};

#endif /* MFACTION_H_ */
