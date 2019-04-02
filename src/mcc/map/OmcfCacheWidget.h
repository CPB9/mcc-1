#pragma once

#include "mcc/Config.h"
#include "mcc/map/TilePosCache.h"
#include "mcc/geo/Fwd.h"

#include <QWidget>

class QDir;
class QProgressBar;

namespace mccmap {

class MCC_MAP_DECLSPEC OmcfCacheWidget : public QWidget {
    Q_OBJECT
public:
    OmcfCacheWidget(QWidget* parent = 0);
    ~OmcfCacheWidget();

signals:
    void progressChanged(int value);
    void finished();

private:
    void createCache(const QString& dirPath, const QString& outputPath, const QString& name, const QString& description,
                     const QByteArray& format, const mccgeo::MercatorProjection& proj);
    void getFiles(int baseLen, const char* format, QDir* startDir, OrderedTilePosCache<QString>* tilePaths);

    QProgressBar* _progressBar;
};
}
