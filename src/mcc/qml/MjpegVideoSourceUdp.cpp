#include "mcc/qml/MjpegVideoSourceUdp.h"
#include <bmcl/TimeUtils.h>

#include <bmcl/MemReader.h>
#include <bmcl/MemWriter.h>

#include <cstdint>

namespace mccqml {

static constexpr int MAX_IMAGE_SIZE = 1000000;

MjpegVideoSourceUdp::MjpegVideoSourceUdp(const QString& boundary, int port)
    : _boundary(boundary)
{
    _socket.bind(QHostAddress::Any, port);

    _socket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 65536);
    int realBufferSize = _socket.socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption).toInt();
    qDebug() << "Real Udp buffer size: " << realBufferSize;

    connect(&_socket, &QUdpSocket::readyRead, this, &MjpegVideoSourceUdp::readDatagram);
    connect(&_socket, static_cast<void (QUdpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &MjpegVideoSourceUdp::socketError);
}

void MjpegVideoSourceUdp::readDatagram()
{
    while (_socket.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(_socket.pendingDatagramSize());
        QHostAddress sender;
        quint16 port;

        _socket.readDatagram(datagram.data(), datagram.size(), &sender, &port);

        processDatagram(datagram);
    }
}

void MjpegVideoSourceUdp::socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qDebug() << _socket.errorString();
}

void MjpegVideoSourceUdp::processDatagram(const QByteArray& data)
{
    _buffer.append(data);

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
}
