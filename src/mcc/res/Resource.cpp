#include "mcc/res/Resource.h"

#include <bmcl/Bytes.h>

#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>


namespace mccres {

bmcl::Bytes _generated_loadResource(ResourceKind kind); //generated

bmcl::Bytes loadResource(ResourceKind kind)
{
    return _generated_loadResource(kind);
}

QPixmap loadPixmap(ResourceKind kind)
{
    return QPixmap::fromImage(loadImage(kind));
}

QImage loadImage(ResourceKind kind)
{
    bmcl::Bytes bytes = loadResource(kind);
    return QImage::fromData(bytes.data(), bytes.size());
}

QIcon loadIcon(ResourceKind kind)
{
    return loadPixmap(kind);
}

QImage renderSvg(const QString& path, std::size_t width, std::size_t height)
{
    return renderSvg(path, width, height, QColor(Qt::transparent));
}

QImage renderSvg(const QString& path, std::size_t width, std::size_t height, const QColor& color)
{
    QSvgRenderer renderer(path);
    QImage pm(width, height, QImage::Format_ARGB32_Premultiplied);
    pm.fill(color);
    QPainter painter(&pm);
    renderer.render(&painter, pm.rect());
    return pm;
}

QImage renderSvg(const QString& path, float scale)
{
    return renderSvg(path, scale, QColor(Qt::transparent));
}

QImage renderSvg(const QString& path, float scale, const QColor& color)
{
    QSvgRenderer renderer(path);
    QImage pm(renderer.defaultSize()*scale, QImage::Format_ARGB32_Premultiplied);
    pm.fill(color);
    QPainter painter(&pm);
    renderer.render(&painter, pm.rect());
    return pm;
}

QImage renderSvg(const bmcl::Bytes& bytes, std::size_t width, std::size_t height)
{
    return renderSvg(bytes, width, height, QColor(Qt::transparent));
}

QImage renderSvg(const bmcl::Bytes& bytes, std::size_t width, std::size_t height, const QColor& color)
{
    QSvgRenderer renderer(QByteArray::fromRawData((const char*)bytes.data(), bytes.size()));
    QImage pm(width, height, QImage::Format_ARGB32_Premultiplied);
    pm.fill(color);
    QPainter painter(&pm);
    renderer.render(&painter, pm.rect());
    return pm;
}

QImage renderSvg(const bmcl::Bytes& bytes, float scale)
{
    return renderSvg(bytes, scale, QColor(Qt::transparent));
}

QImage renderSvg(const bmcl::Bytes& bytes, float scale, const QColor& color)
{
    QSvgRenderer renderer(QByteArray::fromRawData((const char*)bytes.data(), bytes.size()));
    QImage pm(renderer.defaultSize()*scale, QImage::Format_ARGB32_Premultiplied);
    pm.fill(color);
    QPainter painter(&pm);
    renderer.render(&painter, pm.rect());
    return pm;
}

MCC_RES_DECLSPEC QImage renderSvg(const bmcl::Bytes& bytes, std::size_t width, std::size_t height, bool keepAspectRatio, const QColor& color)
{
    QSvgRenderer renderer(QByteArray::fromRawData((const char*)bytes.data(), bytes.size()));
    float xScale = (float)width / renderer.defaultSize().width();
    float yScale = (float)height/ renderer.defaultSize().height();
    QImage pm(renderer.defaultSize()*std::min(xScale, yScale), QImage::Format_ARGB32_Premultiplied);
    pm.fill(color);
    QPainter painter(&pm);
    renderer.render(&painter, pm.rect());
    return pm;
}

}
