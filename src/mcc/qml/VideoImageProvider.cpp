#include "mcc/qml/VideoImageProvider.h"
#include "mcc/qml/MjpegVideoSource.h"

#include <QObject>
#include <QQuickImageProvider>
#include <QUdpSocket>

#include <bmcl/MemReader.h>
#include <bmcl/TimeUtils.h>

namespace mccqml {

VideoImageProvider::VideoImageProvider(const QString& name, MjpegVideoSource* source)
    : QQuickImageProvider(QQmlImageProviderBase::Pixmap)
    , _name(name)
{
    connect(source, &MjpegVideoSource::packetFound, this, &VideoImageProvider::processPacket);
}

QPixmap VideoImageProvider::requestPixmap(const QString &id, QSize *size, const QSize& requestedSize)
{
    Q_UNUSED(id);
    Q_UNUSED(requestedSize);

    if (size)
        *size = _last.size();

    return _last;
}

void VideoImageProvider::processPacket(const QByteArray& data)
{
    auto msecsPast = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _lastUpdate).count();

    if (data.isEmpty() || msecsPast < 1000 / 30)
        return;

    bmcl::MemReader reader(data.data(), data.size());
    if (reader.sizeLeft() < 4)
    {
        return;
    }

    reader.skip(2);
    uint16_t commentMark = reader.readUint16();

    QList<int> bytes;
    if (commentMark == 0xfeff && reader.sizeLeft() > sizeof(uint16_t))
    {
        auto commentLen = reader.readUint16Be();
        if (commentLen > reader.sizeLeft())
        {
            Q_ASSERT(false);
            qDebug() << "Not enough mjpeg comment data";
        }
        else
        {
            for (int i = 0; i < commentLen; ++i)
                bytes.append(reader.readUint8());
        }
    }

    QPixmap img;
    if (img.loadFromData(data, "JPEG"))
    {
        _last = img;
        _lastUpdate = std::chrono::steady_clock::now();
        emit dataChanged(bytes);
    }
    else
    {
        QByteArray soiMarker;
        soiMarker.append(0xff);
        soiMarker.append(0xd8);

        //unused int soi = data.indexOf(soiMarker, 0);
        //unused int soi2 = data.indexOf(soiMarker, soi+1);
    }
}
}
