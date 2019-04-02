#include "mcc/qml/QmlController.h"

#include "mcc/uav/UavController.h"

namespace mccqml {

QmlController::~QmlController()
{
}

Q_INVOKABLE QStringList QmlController::getActionsList() const
{
    return QStringList() << "aaa" << "bbb";
}

}

