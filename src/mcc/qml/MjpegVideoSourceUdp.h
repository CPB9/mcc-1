#pragma once

#include "mcc/Config.h"
#include "mcc/qml/MjpegVideoSource.h"

#include <QUdpSocket>
#include <QString>

namespace mccqml {

class MCC_QML_DECLSPEC MjpegVideoSourceUdp : public MjpegVideoSource
{
    Q_OBJECT
public:
    MjpegVideoSourceUdp(const QString& boundary, int port);

private slots:
    void readDatagram();

    void socketError(QAbstractSocket::SocketError socketError);

private:
    void processDatagram(const QByteArray& data);

private:
    QString    _boundary;
    QUdpSocket _socket;
    QByteArray _buffer;
};
}
