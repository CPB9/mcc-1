#include "mcc/map/FlagRenderer.h"
#include "mcc/res/Resource.h"

#include <bmcl/Math.h>
#include <bmcl/Bytes.h>

#include <QImage>
#include <QPainter>

#include <cmath>
#include <functional>

namespace mccmap {

static void drawFlag(const QRectF& rect, QPainter* painter)
{
    double min = std::min(rect.width(), rect.height());
    double centerX = rect.center().x(); //, centerY(rect.center().y());
    double radius = min / 2.0 - 1.0;
    QRectF circleRect(centerX - radius, rect.top() + 1, radius * 2.0, radius * 2.0);
    painter->drawEllipse(circleRect);
    QPolygonF poly;
    double angleFromRad = bmcl::degreesToRadians(30.0);
    double angleToRad = bmcl::degreesToRadians(150.0);
    poly << QPointF(centerX, rect.bottom())
         << QPointF(centerX + radius * std::cos(angleFromRad), circleRect.center().y() + radius * std::sin(angleFromRad))
         << QPointF(centerX + radius * std::cos(angleToRad), circleRect.center().y() + radius * std::sin(angleToRad));
    poly << poly.at(0);
    painter->drawPolygon(poly);
}

QImage FlagRenderer::renderFlag(std::size_t width, std::size_t height, const QColor& color)
{
    QImage flagImg((int)width, (int)height, QImage::Format_ARGB32_Premultiplied);
    flagImg.fill(Qt::transparent);
    QPainter painter(&flagImg);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing);

    QRectF outerFlagRect(0.0, 0.0, width + 1, height + 1);
    double min = std::min(width, height);
    QPointF circleCenter((width + 1) / 2.0, min / 2.0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    drawFlag(outerFlagRect, &painter);

    QRectF innerFlagRect = outerFlagRect.adjusted(1, 1, -1, -1);
    QLinearGradient colorGradient(innerFlagRect.topLeft(), innerFlagRect.bottomRight());
    colorGradient.setColorAt(0, color.lighter(150));
    colorGradient.setColorAt(1, color.darker(150));
    painter.setBrush(colorGradient);
    drawFlag(innerFlagRect, &painter);

    double radius = min / 3.0;
    QRectF innerCircleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius * 2., radius * 2.);
    painter.setBrush(Qt::gray);
    painter.drawEllipse(innerCircleRect);

    innerCircleRect = innerCircleRect.adjusted(1, 1, -1, -1);
    QLinearGradient blackGradient(innerCircleRect.topLeft(), innerCircleRect.bottomRight());
    blackGradient.setColorAt(0, Qt::gray);
    blackGradient.setColorAt(1, Qt::white);
    painter.setBrush(blackGradient);
    painter.drawEllipse(innerCircleRect);

    return flagImg;
}

static QRectF computeFlagRect(const QRectF& rect)
{
    return QRectF(rect.width() / 4.0, rect.height() / 4.0, rect.width() / 2.0, rect.height() * 0.75);
}

static QPointF computeCircleCenterForFlagRect(const QRectF& rect)
{
    double flagMin = std::min(rect.width(), rect.height());
    return QPointF(rect.left() + (rect.width() + 1) / 2.0, rect.top() + flagMin / 2.0);
}

typedef std::function<void(const QRectF&, QPainter*)> WaypointFlagPainter;

// ReadOnly = 1 << 0,  //! Только для чтения
// Target = 1 << 1,  //! Цель
// Parashute = 1 << 2,  //! Парашют
// TurnBack = 1 << 3,  //! Разворот назад
// TurnForward = 1 << 4,  //! Разворот вперед
// Waiting = 1 << 5,  //! Ожидание
// SwitchRoute = 1 << 6,  //! Переход на другой маршрут
// Home = 1 << 7,  //! Дом
// Landing = 1 << 8, //! Посадка
// Reynolds = 1 << 9,
// Formation = 1 << 10,
// Snake = 1 << 11,
// Loop = 1 << 12

WaypointFlagPainter waypointFlagPainters[] = {
    // ReadOnly = 1 << 0,  //! Только для чтения
    [](const QRectF& rect, QPainter* painter) {
        (void)painter;
        (void)rect;
    },
    // Target = 1 << 1,  //! Цель
    [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/target1.svg"), rect.width(), rect.height());
        painter->translate(0, rect.height() / 2);
        painter->drawImage(rect, img);
    },
    // Parashute = 1 << 2,  //! Парашют
    [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/parachute.svg"), rect.width() / 2, rect.height() / 2);
        painter->drawImage(QPointF(0, 0), img);
    },
    // TurnBack = 1 << 3,  //! Разворот назад
    [](const QRectF& rect, QPainter* painter) {
        double rectWidth = rect.width();
        double rectHeight = rect.height();
        double min = std::min(rectWidth, rectHeight);
        QPointF circleCenter = computeCircleCenterForFlagRect(computeFlagRect(rect));
        double radius = min / 5.0;
        QRectF innerCircleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius * 2, radius * 2);
        painter->setPen(QPen(Qt::red, rectWidth / 14.0));
        painter->drawArc(innerCircleRect, -45 * 16, 270 * 16);
        double angleRad = bmcl::degreesToRadians(135.0);
        const QPointF start(circleCenter.x() + radius * std::cos(angleRad) + rectWidth / 64.0,
                            circleCenter.y() + radius * std::sin(angleRad) + rectWidth / 64.0);
        double arrowAngleRad = bmcl::degreesToRadians(180.0);
        double arrowSecondAngleRad = bmcl::degreesToRadians(180.0 + 90.0);
        double arrowLength = radius / 3;
        painter->drawLine(start, start + QPointF(arrowLength * std::cos(arrowAngleRad), arrowLength * std::sin(arrowAngleRad)));
        painter->drawLine(start, start + QPointF(arrowLength * std::cos(arrowSecondAngleRad), arrowLength * std::sin(arrowSecondAngleRad)));
    },
    // TurnForward = 1 << 4,  //! Разворот вперед
    [](const QRectF& rect, QPainter* painter) {
        double rectWidth = rect.width();
        double rectHeight = rect.height();
        double min = std::min(rectWidth, rectHeight);
        QPointF circleCenter = computeCircleCenterForFlagRect(computeFlagRect(rect));
        double radius = min / 5.0;
        QRectF innerCircleRect(circleCenter.x() - radius, circleCenter.y() - radius, radius * 2, radius * 2);
        painter->setPen(QPen(Qt::red, rectWidth / 14.0));
        painter->drawArc(innerCircleRect, -45 * 16, 270 * 16);
        double angleRad = bmcl::degreesToRadians(45.0);
        const QPointF start(circleCenter.x() + radius * std::cos(angleRad) - rectWidth / 64.0,
                            circleCenter.y() + radius * std::sin(angleRad) + rectWidth / 64.0);
        double arrowAngleRad = bmcl::degreesToRadians(270.0);
        double arrowSecondAngleRad = bmcl::degreesToRadians(270.0 + 90.0);
        double arrowLength = radius / 3;
        painter->drawLine(start, start + QPointF(arrowLength * std::cos(arrowAngleRad), arrowLength * std::sin(arrowAngleRad)));
        painter->drawLine(start, start + QPointF(arrowLength * std::cos(arrowSecondAngleRad), arrowLength * std::sin(arrowSecondAngleRad)));
    },
    //! Ожидание
    [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/pause.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(rect.width() / 2.0, 0), img);
    },
    //! Переключение
    [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/switch_route.svg"), rect.width(), rect.height());
        painter->drawImage(rect, img);
    },
    //! Дом
    [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/home.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(rect.width() / 2.0, rect.height() / 2.0), img);
    },
    //! Посадка
    [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/landing.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(0, rect.height() / 2.0), img);
    },
    // Reynolds = 1 << 9,
        [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/reynolds.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(0, 0), img);
    },
    // Formation = 1 << 10,
        [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/formation.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(0, 0), img);
    },
    // Snake = 1 << 11
        [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/snake.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(0, 0), img);
    },
    // Loop = 1 << 12,
        [](const QRectF& rect, QPainter* painter) {
        QImage img = mccres::renderSvg(QString(":/resources/waypoints/loop.svg"), rect.width() / 2.0, rect.height() / 2.0);
        painter->drawImage(QPointF(0, 0), img);
    }
};

QImage FlagRenderer::renderWaypointFlag(std::size_t width, std::size_t height, const QColor& color,
                                        std::size_t waypointNumber, const mccmsg::PropertyValues& waypointFlags)
{
    (void)waypointFlags;
    QImage waypointImage((int)width, (int)height, QImage::Format_ARGB32_Premultiplied);
    waypointImage.fill(Qt::transparent);
    QImage flagImage = renderWaypointFlag(width, height, color);
    QPainter painter(&waypointImage);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.drawImage(QPointF(0.0, 0.0), flagImage);
    QPointF circleCenter = computeCircleCenterForFlagRect(computeFlagRect(QRectF(0.0, 0.0, width, height)));
    double min = std::min(width, height);
    painter.setPen(Qt::black);
    painter.drawText(QRectF(circleCenter.x() - min / 2.0, circleCenter.y() - min / 2.0, min, min * 0.98), Qt::AlignCenter,
                     QString::number(waypointNumber));

    return waypointImage;
}

QImage FlagRenderer::renderWaypointFlag(std::size_t width, std::size_t height, const QColor& color)
{
    QImage waypointImage((int)width, (int)height, QImage::Format_ARGB32_Premultiplied);
    waypointImage.fill(Qt::transparent);
    QImage flagImage = renderFlag(width / 2.0, height * 0.75, color);
    QPainter painter(&waypointImage);
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing);
    QRectF flagRect = computeFlagRect(QRect(0.0, 0.0, (int)width, (int)height));
    painter.drawImage(flagRect.topLeft(), flagImage);
    return waypointImage;
}

QImage FlagRenderer::renderWaypointOverlayFlag(std::size_t width, std::size_t height,
                                               std::size_t waypointNumber, const mccmsg::PropertyValues& waypointFlags)
{
    (void)waypointNumber;
    QImage waypointImage((int)width, (int)height * 2, QImage::Format_ARGB32_Premultiplied);
    waypointImage.fill(Qt::transparent);

    QPainter painter(&waypointImage);
    for (const auto& flag : waypointFlags.values())
    {
        if(flag->isHidden())
            continue;
        painter.setRenderHint(QPainter::Antialiasing);
        QRectF rect(0, 0, width, height);
        const std::string& pixmap = flag->property()->pixmap();
//        bmcl::Bytes bytes = bmcl::Bytes((const uint8_t*)pixmap.data(), pixmap.size());
        QImage img = mccres::renderSvg(QString::fromStdString(pixmap), rect.width() / 2, rect.height() / 2);
        painter.drawImage(QPointF(0, 0), img);
        painter.translate(5, 0);
    }
//     if (waypointFlags != 0) {
//         QPainter painter(&waypointImage);
//         painter.setRenderHint(QPainter::Antialiasing);
//         //painter.setRenderHint(QPainter::HighQualityAntialiasing);
//         QRectF rect(0, 0, width, height);
//         for (std::size_t i(0); i < sizeof(waypointFlagPainters) / sizeof(waypointFlagPainters[0]); i++) {
//             if ((waypointFlags & (1 << i)) != 0) {
//                 painter.save();
//                 waypointFlagPainters[i](rect, &painter);
//                 painter.restore();
//             }
//         }
//     }
    return waypointImage;
}
}
