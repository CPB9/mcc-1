#pragma once

#include "mcc/res/Config.h"
#include "mcc/res/ResourceKind.h"

#include <bmcl/Fwd.h>

class QImage;
class QPixmap;
class QIcon;
class QString;
class QColor;

namespace mccres {

MCC_RES_DECLSPEC bmcl::Bytes loadResource(ResourceKind kind);

MCC_RES_DECLSPEC QPixmap loadPixmap(ResourceKind kind);
MCC_RES_DECLSPEC QImage loadImage(ResourceKind kind);
MCC_RES_DECLSPEC QIcon loadIcon(ResourceKind kind);

MCC_RES_DECLSPEC QImage renderSvg(const QString& path, float scale);
MCC_RES_DECLSPEC QImage renderSvg(const QString& path, float scale, const QColor& color);

MCC_RES_DECLSPEC QImage renderSvg(const QString& path, std::size_t width, std::size_t height);
MCC_RES_DECLSPEC QImage renderSvg(const QString& path, std::size_t width, std::size_t height, const QColor& color);

MCC_RES_DECLSPEC QImage renderSvg(const bmcl::Bytes& bytes, float scale);
MCC_RES_DECLSPEC QImage renderSvg(const bmcl::Bytes& bytes, float scale, const QColor& color);

MCC_RES_DECLSPEC QImage renderSvg(const bmcl::Bytes& bytes, std::size_t width, std::size_t height);
MCC_RES_DECLSPEC QImage renderSvg(const bmcl::Bytes& bytes, std::size_t width, std::size_t height, const QColor& color);
MCC_RES_DECLSPEC QImage renderSvg(const bmcl::Bytes& bytes, std::size_t width, std::size_t height, bool keepAspectRatio, const QColor& color);

}
