#pragma once

#include "mcc/Config.h"

#include <QString>

class QPixmap;
class QColor;

namespace mccuav {

class MCC_UAV_DECLSPEC UavIconGenerator
{
public:
    UavIconGenerator();
    void setContents(const QString& fileContents);
    QPixmap generate(const QColor& baseColor, float scale = 1.0f) const;
    QPixmap generate(const QColor& baseColor, int width, int height) const;
private:
    QByteArray replaceColor(const QColor& baseColor) const;

    QString _fileContents;
};
}
