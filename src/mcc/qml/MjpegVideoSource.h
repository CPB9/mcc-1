#pragma once

#include "mcc/Config.h"

#include <QObject>

class QByteArray;

namespace mccqml {

class MCC_QML_DECLSPEC MjpegVideoSource : public QObject
{
    Q_OBJECT
signals:
    void packetFound(const QByteArray& packet);
};
}
