

#include "mcc/qml/MjpegVideoSourceTcp.h"
#include <bmcl/TimeUtils.h>

#include <bmcl/MemReader.h>
#include <bmcl/MemWriter.h>

#include <QTcpSocket>
#include <QTimer>

#include <cstdint>

namespace mccqml {

static constexpr int MAX_IMAGE_SIZE = 1000000;

MjpegVideoSourceTcp::MjpegVideoSourceTcp(const QString& address, int port, bool dropConnection)
    : _address(address)
    , _port(port)
    , _dropConnection(dropConnection)
{
    connect(&_socket, &QTcpSocket::readyRead, this, &MjpegVideoSourceTcp::read);
    connect(&_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &MjpegVideoSourceTcp::socketError);
    connect(&_socket, &QTcpSocket::disconnected, this, &MjpegVideoSourceTcp::tryConnect);
    tryConnect();
}

void MjpegVideoSourceTcp::tryConnect()
{
    if (_socket.state() == QAbstractSocket::ConnectingState)
        return;

    _socket.disconnectFromHost();
    _socket.connectToHost(_address, _port);
}

void MjpegVideoSourceTcp::read()
{
    QByteArray data = _socket.readAll();
    if (_buffer.size() + data.size() > MAX_IMAGE_SIZE)
    {
        _socket.close();
        _buffer.clear();

        qDebug() << "TCP buffer overflow. Closing connection";
        return;
    }
    _buffer.append(data);

    if (_dropConnection)
        return;

    const QString _boundary = "--7b3cc56e5f51db803f790dad720ed50a";

    int firstDelimiter = _buffer.indexOf(_boundary);
    if (firstDelimiter == -1)
    {
        if (_buffer.size() > 2 * _boundary.size())
        {
            _buffer.remove(0, _buffer.size() - _boundary.size());
        }
        return;
    }

    int secondDelimiter = _buffer.indexOf(_boundary, firstDelimiter + 1);
    if (secondDelimiter == -1)
    {
        if (_buffer.size() >= (firstDelimiter + _boundary.size() + MAX_IMAGE_SIZE))
        {
            _buffer.remove(0, _buffer.size() - _boundary.size());
        }
        else if ((firstDelimiter >= MAX_IMAGE_SIZE) || (firstDelimiter >= _buffer.size() / 2))
        {
            _buffer.remove(0, firstDelimiter);
        }

        return;
    }

    auto imageStart = firstDelimiter + _boundary.size();
    auto imageSize = secondDelimiter - imageStart;

    bmcl::MemReader reader(_buffer.data() + imageStart, imageSize);
    if (reader.sizeLeft() < 4)
    {
        _buffer.remove(0, secondDelimiter);
        qDebug() << "garbage between delimiters " << reader.sizeLeft();
        return;
    }

    emit packetFound(QByteArray(_buffer.data() + imageStart, imageSize));
    _buffer.remove(0, secondDelimiter);
}

void MjpegVideoSourceTcp::socketError(QAbstractSocket::SocketError socketError)
{
    if (_dropConnection && socketError == QAbstractSocket::SocketError::RemoteHostClosedError)
    {
        if (!_buffer.isEmpty())
        {
            emit packetFound(_buffer.mid(1));
            _buffer.clear();
        }
        return;
    }
    _buffer.clear();
    qDebug() << "Socket error " << _socket.errorString();
    qDebug() << "Waiting 1 sec to reconnect";
    QTimer::singleShot(1000, this, &MjpegVideoSourceTcp::tryConnect);
}
}
