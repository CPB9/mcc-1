#pragma once

#include "mcc/Config.h"
#include "mcc/qml/MjpegVideoSource.h"

#include <QTcpSocket>
#include <QString>

namespace mccqml {

class MCC_QML_DECLSPEC MjpegVideoSourceTcp : public MjpegVideoSource
{
    Q_OBJECT
public:
    MjpegVideoSourceTcp(const QString& address, int port, bool dropConnection);

private slots:
    void tryConnect();
    void read();

    void socketError(QAbstractSocket::SocketError socketError); //TODO: убрать завязку

private:
    void dropFrame();

    QString         _address;
    int             _port;
    bool            _dropConnection;

    QTcpSocket      _socket;
    QByteArray      _buffer;
};
}
