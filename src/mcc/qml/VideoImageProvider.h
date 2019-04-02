#pragma once

#include "mcc/Config.h"

#include <QObject>
#include <QQuickImageProvider>
#include <QString>
#include <QPixmap>

#include <chrono>
#include <memory>

class QSize;
class QByteArray;

namespace mccqml {

class MjpegVideoSource;

class MCC_QML_DECLSPEC VideoImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    VideoImageProvider(const QString& name, MjpegVideoSource* source);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize) override;

    inline const QString& name() const { return _name; }

signals:
    void dataChanged(const QList<int>& data);

private:
    void processPacket(const QByteArray& data);

private:
    QString _name;
    QPixmap _last;

    std::chrono::steady_clock::time_point _lastUpdate;
};

typedef std::shared_ptr<VideoImageProvider> VideoImageProviderPtr;
}

