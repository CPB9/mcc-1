#include "mcc/uav/UavIconGenerator.h"
#include "mcc/res/Resource.h"

#include <bmcl/Bytes.h>

#include <QPixmap>
#include <QColor>
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QDebug>

namespace mccuav {

const QString LIGHT_MARKER = "#ffcc00";
const QString NORMAL_MARKER = "#806600";
const QString DARK_MARKER = "#2b2200";

UavIconGenerator::UavIconGenerator()
{
}

void UavIconGenerator::setContents(const QString& fileContents)
{
    _fileContents = fileContents;
}

QPixmap UavIconGenerator::generate(const QColor& baseColor, float scale) const
{
    QByteArray data = replaceColor(baseColor);
    return QPixmap::fromImage(mccres::renderSvg(bmcl::Bytes((const uint8_t*)data.data(), data.size()), scale));
}

QPixmap UavIconGenerator::generate(const QColor& baseColor, int width, int height) const
{
    QByteArray data = replaceColor(baseColor);
    return QPixmap::fromImage(mccres::renderSvg(bmcl::Bytes((const uint8_t*)data.data(), data.size()), width, height, true, Qt::transparent));
}

QByteArray UavIconGenerator::replaceColor(const QColor& baseColor) const
{
    QString svgImg = _fileContents;

    QColor lightColor = baseColor.darker(150);
    QColor darkColor = baseColor.darker(200);

    svgImg.replace(LIGHT_MARKER, baseColor.name(), Qt::CaseInsensitive);
    svgImg.replace(NORMAL_MARKER, lightColor.name(), Qt::CaseInsensitive);
    svgImg.replace(DARK_MARKER, darkColor.name(), Qt::CaseInsensitive);
    return svgImg.toUtf8();
}

}
