#include "mcc/vis/RegionViewer.h"
#include "mcc/vis/Ticks.h"

#include <QPainter>
#include <QDebug>
#include <QWheelEvent>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QScreen>
#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainterPath>
#include <bmcl/Math.h>

#include <chrono>
#include <cmath>

namespace mccvis {

RegionViewer::RegionViewer(QWidget* parent)
    : QWidget(parent)
    , _minViewAngle(0)
    , _maxViewAngle(0)
    , _mouseScale(1)
    , _isMousePressed(false)
    , _mode(RegionViewer::RegionMode)
{
    _angleTicks.min = 0;
    _angleTicks.max = 0;
    _angleTicks.step = 0;

    _backgroundCheckbox = new QCheckBox("Фон");
    _backgroundCheckbox->setChecked(_renderConfig.drawBackground);
    connect(_backgroundCheckbox, &QCheckBox::toggled, this, [this](bool flag) {
        _renderConfig.drawBackground = flag;
        update();
    });

    QHBoxLayout* cbLayout = new QHBoxLayout;
    cbLayout->addWidget(_backgroundCheckbox);
    cbLayout->addStretch();
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(cbLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
    setMouseTracking(true);
}

RegionViewer::~RegionViewer()
{
}

constexpr double pi = bmcl::pi<double>();

static inline QPointF fromPolar(Point p)
{
    double phi = p.x() * pi / 180.0;
    double r = p.y();
    return QPointF(r * std::sin(phi), r * std::cos(phi));
};

static inline QPointF toPolar(Point p)
{
    double x = p.x();
    double y = p.y();
    if (x == 0) {
        if (y > 0) {
            return QPointF(0, p.y());
        } else {
            return QPointF(-180, p.y());
        }
    }
    double r = std::sqrt(x * x + y * y);
    double phi = std::atan(y / x) * 180.0 / pi;
    if (x > 0) {
        phi = 90.0 - phi;
    } else {
        phi = 270.0 - phi;
    }
    return QPointF(phi, r);
};

void RegionViewer::setRegion(const Region* region)
{
    _paths.clear();
    _hitPaths.clear();
    _region.reset(region);
    _anglePath.clear();
    _selectedProfile = bmcl::None;
    const ViewParams& params = region->params();
    _maxDistance = std::max(params.maxBeamDistance, params.maxHitDistance) * (params.additionalDistancePercent / 100.0 + 1.0);
    for (const auto& curve : _region->curves()) {
        _paths.emplace_back();
        QPainterPath& path = _paths.back();
        if (curve.empty()) {
            continue;
        }
        path.moveTo(fromPolar(curve[0]));
        for (std::size_t i = 1; i < curve.size(); i++) {
            path.lineTo(fromPolar(curve[i]));
        }
    }
    for (const auto& curve : _region->hitCurves()) {
        _hitPaths.emplace_back();
        QPainterPath& path = _hitPaths.back();
        if (curve.empty()) {
            continue;
        }
        path.moveTo(fromPolar(curve[0]));
        for (std::size_t i = 1; i < curve.size(); i++) {
            path.lineTo(fromPolar(curve[i]));
        }
    }

    _minViewAngle = 90;
    _maxViewAngle = -90;
    for (const Rc<Profile>& prof : _region->profiles()) {
        double dir = prof->direction();
        double angle = prof->viewAngle();
        _minViewAngle = std::min(_minViewAngle, angle);
        _maxViewAngle = std::max(_maxViewAngle, angle);
        _anglePath.emplace_back(dir, angle);
    }

    if (qFuzzyCompare(_minViewAngle, _maxViewAngle)) {
        _angleTicks = Ticks::fromMinMax(_minViewAngle - 0.1, _maxViewAngle + 0.1);
    } else {
        _angleTicks = Ticks::fromMinMax(_minViewAngle, _maxViewAngle);
    }


    update();
}

void RegionViewer::setSelectedProfile(bmcl::Option<std::size_t> idx)
{
    _selectedProfile = idx;
    update();
}

void RegionViewer::wheelEvent(QWheelEvent* event)
{
    double zoom;
    double offtZoom;
    if (event->delta() > 0) {
        zoom = 2.0;
        offtZoom = 1.0;
    } else if (event->delta() < 0) {
        zoom = 0.5;
        offtZoom = -zoom;
    } else {
        return;
    }
    QPoint center = rect().center();
    QPoint offset = center - event->pos() + _drawDelta;
    _drawDelta += offset * offtZoom;
    _mouseScale *= zoom;
    updatePosLabel(event->pos());
    update();
}

bmcl::Option<std::size_t> RegionViewer::findProf(const QPointF& mousePos)
{
    double deltaPhi = _region->params().angleStep;
    QPointF pos = toPolar(calcTransform(this).inverted().map(mousePos));

    if (isRegionView()) {
        if (pos.y() > _maxDistance) {
            return bmcl::None;
        }
    } else {
        if (pos.y() > (_angleTicks.max - _angleTicks.min)) {
            return bmcl::None;
        }
    }

    for (std::size_t i = 0; i < _region->profiles().size(); i++) {
        double phi = _region->profiles()[i]->direction();
        double p1 = phi - deltaPhi;
        double p2 = phi + deltaPhi;
        if (pos.x() >= p1 && pos.x() <= p2) {
            return bmcl::Option<std::size_t>(i);
        }
    }

    return bmcl::None;
}

void RegionViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        _drawDelta = QPoint(0, 0);
        _mousePos = QPoint(0, 0);
        _mouseScale = 1;
        _hoverProfile = findProf(event->pos());
        update();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        _mousePos = event->pos();
        _isMousePressed = true;
        return;
    }

    if (event->button() == Qt::RightButton) {
        _hoverProfile = findProf(event->pos());
        if (_hoverProfile.isSome()) {
            emit profileClicked(_hoverProfile.unwrap());
            _hoverProfile.clear();
            update();
            return;
        }
    }
}

void RegionViewer::mouseReleaseEvent(QMouseEvent* event)
{
    _isMousePressed = false;
    update();
}

void RegionViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (_isMousePressed) {
        _drawDelta += event->pos() - _mousePos;
        _mousePos = event->pos();
        update();
        return;
    }
    _hoverProfile = findProf(event->pos());
    updatePosLabel(event->pos());
    update();
}

void RegionViewer::updatePosLabel(const QPoint& mousePos)
{
    QPointF pos = toPolar(calcTransform(this).inverted().map(QPointF(mousePos)));
    if (isRegionView()) {
        if (pos.y() > _maxDistance) {
            _positionText.clear();
            return;
        }
        _positionText = QString("φ = %1°, d = %2м").arg(pos.x()).arg(pos.y());
    } else {
        if (pos.y() > (_angleTicks.max - _angleTicks.min)) {
            _positionText.clear();
            return;
        }
        _positionText = QString("φ = %1°, α = %2°").arg(pos.x()).arg(pos.y() + _angleTicks.min);
    }
}

bool RegionViewer::isRegionView() const
{
    return _mode == RegionViewer::RegionMode;
}

QTransform RegionViewer::calcTransform(const QPaintDevice* dev) const
{
    double _deltay;
    double _deltax;

    if (isRegionView()) {
        double r = _maxDistance;
        auto ticks = Ticks::fromMinMax(r * 0.02, r);
        r = ticks.max;
        _deltay = r * 2.0;
        _deltax = _deltay;
    } else {
        _deltay = (_angleTicks.max - _angleTicks.min) * 2.0;
        _deltax = _deltay;
    }

    int plotXmargin = 0;
    int plotYmargin = 0;

    int m = std::min(plotXmargin, plotYmargin);
    int s = std::min(dev->width(), dev->height());

    double xscale = ((s - 2 * m) / _deltax) * 0.9;
    double yscale = ((s - 2 * m) / _deltay) * 0.9;

    QTransform plotTransform;
    plotTransform.translate(dev->width() / 2, dev->height() / 2);
    plotTransform.translate(plotXmargin, -plotYmargin);
    if (dev == this) {
        QPointF offset = QPointF(_drawDelta);
        plotTransform.translate(offset.x(), offset.y());
        plotTransform.scale(_mouseScale, _mouseScale);
    }
    plotTransform.scale(xscale, -yscale);

    return plotTransform;
}

void RegionViewer::paintMain(QPainter* p, const RenderConfig& cfg)
{
    if (_region.isNull()) {
        return;
    }

    double fontScale = 1;
    if (p->device()->width() < p->device()->height()) {
        fontScale = p->device()->width() / double(qApp->primaryScreen()->size().width());
    } else {
        fontScale = p->device()->height() / double(qApp->primaryScreen()->size().height());
    }
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPointSizeF(10 * fontScale);
    p->setFont(font);

    Qt::GlobalColor bgColor = Qt::white;
    if (cfg.drawBackground) {
        bgColor = Qt::gray;
    }
    p->fillRect(0, 0, p->device()->width(), p->device()->height(), bgColor);

    QPen pen;
    pen.setColor(Qt::black);
    pen.setCosmetic(true);
    p->setPen(pen);

    double maxr;
    if (isRegionView()) {
        maxr = _maxDistance;
    } else {
        maxr = _angleTicks.max - _angleTicks.min;
    }

    double minr = maxr * 0.02;
    Ticks ticks;
    double rOffset = 0;
    if (isRegionView()) {
        ticks = Ticks::fromMinMax(minr, maxr);
        maxr = ticks.max;
    } else {
        ticks = _angleTicks;
        if (_angleTicks.min < 0) {
            rOffset = -_angleTicks.min;
        }
    }

    QTransform transform = calcTransform(p->device());
    p->setTransform(transform);

    //pen.setWidth(2);
    //pen.setCosmetic(true);
    //pen.setColor(Qt::red);
    //p.setPen(pen);
    /*
    for (const auto& prof : _region->profiles()) {
        double x = prof.direction;
        if (prof.value->intersections().empty()) {
            continue;
        }
        for (size_t i = 0; i < prof.value->intersections().size(); i += 2) {
            double y1 = prof.value->intersections()[i].x;
            double y2 = prof.value->intersections()[i + 1].x;
            drawLine(QPointF(x, y1), QPointF(x, y2));
        }
    }
    */

    //auto now = std::chrono::system_clock::now();
    p->setBrush(Qt::white);
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPointF(0, 0), maxr, maxr);

    if (isRegionView()) {
        QColor fillGreen;
        fillGreen.setRgba(_region->params().viewZonesColorArgb);
        for (const QPainterPath& path : _paths) {
            //auto c = QRgba64::fromRgba(rand(), rand(), rand(), 0);
            //p->fillPath(path, QColor::fromRgba64(c));
            p->fillPath(path, fillGreen);
        }

        QColor fillRed;
        fillRed.setRgba(_region->params().hitZonesColorArgb);
        for (const QPainterPath& path : _hitPaths) {
            //p.fillPath(path, QColor::fromRgb(rand()));
            p->fillPath(path, fillRed);
        }
        //auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now);
        //p.setTransform(QTransform());
        //p.drawText(20, 20, QString::number(delta.count()));
    } else {
        if (!_anglePath.empty()) {
            QPen anglePen;
            anglePen.setCosmetic(true);
            anglePen.setWidth(2);
            QColor red;
            red.setRgb(227, 26, 28);
            anglePen.setColor(red);
            QPainterPath anglePath;

            if (_region->params().isBidirectional) {
                anglePath.moveTo(fromPolar(QPointF(_anglePath[0].x(), _anglePath[0].y() + rOffset)));
            } else {
                anglePath.moveTo(0, 0);
                anglePath.lineTo(fromPolar(QPointF(_anglePath[0].x(), _anglePath[0].y() + rOffset)));
            }

            for (std::size_t i = 0; i < _anglePath.size(); i++) {
                anglePath.lineTo(fromPolar(QPointF(_anglePath[i].x(), _anglePath[i].y() + rOffset)));
            }
            if (_region->params().isBidirectional) {
                anglePath.lineTo(fromPolar(QPointF(_anglePath[0].x(), _anglePath[0].y() + rOffset)));
            } else {
                anglePath.lineTo(0, 0);
            }

            p->strokePath(anglePath, anglePen);
        }
    }

    QPen _gridPen;
    _gridPen.setColor(Qt::darkGray);
    _gridPen.setWidthF(1);
    _gridPen.setStyle(Qt::DotLine);
    _gridPen.setCosmetic(true);
    p->setPen(_gridPen);

    for (double i = 0; i < 360; i += 45) {
        p->drawLine(fromPolar(QPointF(i + 15, minr)), fromPolar(QPointF(i + 15, maxr)));
        p->drawLine(fromPolar(QPointF(i + 30, minr)), fromPolar(QPointF(i + 30, maxr)));
    }

    p->setBrush(Qt::NoBrush);
    p->drawEllipse(QPointF(0, 0), minr, minr);
    for (double r = ticks.min; r < ticks.max; r += ticks.step) {
        p->drawEllipse(QPointF(0, 0), r + rOffset, r + rOffset);
    }

    _gridPen.setWidthF(0.5);
    _gridPen.setStyle(Qt::SolidLine);
    _gridPen.setColor(Qt::black);
    p->setPen(_gridPen);

    for (double i = 0; i < 360; i += 45) {
        p->drawLine(fromPolar(QPointF(i, minr)), fromPolar(QPointF(i, maxr)));
    }

    pen.setWidth(2);
    p->setPen(pen);
    p->drawEllipse(QPointF(0, 0), maxr, maxr);
    p->setBrush(Qt::black);
    p->drawEllipse(QPointF(0, 0), maxr * 0.001, maxr * 0.001);

    p->setBrush(Qt::NoBrush);
    if (isRegionView()) {
        pen.setStyle(Qt::DashLine);
        pen.setColor(Qt::darkGreen);
        pen.setWidth(2);
        p->setPen(pen);
        double d = _region->params().minBeamDistance;
        p->drawEllipse(QPointF(0, 0), d, d);
        d = _region->params().maxBeamDistance;
        p->drawEllipse(QPointF(0, 0), d, d);
        pen.setColor(Qt::darkRed);
        pen.setWidth(2);
        p->setPen(pen);
        d = _region->params().minHitDistance;
        p->drawEllipse(QPointF(0, 0), d, d);
        d = _region->params().maxHitDistance;
        p->drawEllipse(QPointF(0, 0), d, d);
    }

    p->resetTransform();
    p->setPen(Qt::black);
    QFontMetrics metrics(font);
    QRect posRect = metrics.boundingRect(_positionText);
    QPointF posPos(width() - posRect.width() - 2 * metrics.descent(), 2* metrics.descent() + posRect.height());
    p->drawText(posPos, _positionText);

    QPen fontPen;
    fontPen.setColor(Qt::black);
    p->setPen(fontPen);
    for (double r = ticks.min; r <= ticks.max; r += ticks.step) {
        if (qFuzzyIsNull(r)) {
            r = 0;
        }
        QString text = QString::number(r);
        if (!isRegionView()) {
            text += "°";
        }
        QPointF point = transform.map(fromPolar(QPointF(30.0, r + rOffset)));
        p->drawText(point, text);
    }
    for (double i = 0 + 15; i <= 360; i += 15) {
        QString text = QString::number(i) + "°";
        QRectF rect = metrics.tightBoundingRect(text);
        double dx = -rect.width() / 2;
        double dy = rect.height() / 2;
        QPointF point = transform.map(fromPolar(QPointF(i, maxr * 1.05)));
        point.rx() += dx;
        point.ry() += dy;

        p->drawText(point, text);
    }
    p->setTransform(transform);
}

void RegionViewer::paintSelection(QPainter* p)
{
    double maxr;
    if (isRegionView()) {
        maxr = _maxDistance;
    } else {
        maxr = _angleTicks.max - _angleTicks.min;
    }
    double minr = maxr * 0.02;
    if (_selectedProfile.isSome()) {
        std::size_t i = _selectedProfile.unwrap();
        if (i < _region->profiles().size()) {
            double dir = _region->profiles()[i]->direction();
            QPen pen;
            pen.setCosmetic(true);
            pen.setColor(QColor::fromRgb(31, 120, 180));
            pen.setWidthF(2);
            pen.setStyle(Qt::DashDotLine);
            p->setPen(pen);
            p->drawLine(fromPolar(QPointF(dir, minr)), fromPolar(QPointF(dir, maxr)));
        }
    }

    if (_hoverProfile.isSome()) {
        std::size_t i = _hoverProfile.unwrap();
        if (i < _region->profiles().size()) {
            double dir = _region->profiles()[i]->direction();
            QPen pen;
            pen.setCosmetic(true);
            pen.setColor(QColor::fromRgb(31, 120, 180));
            pen.setWidthF(1);
            p->setPen(pen);
            p->drawLine(fromPolar(QPointF(dir, minr)), fromPolar(QPointF(dir, maxr)));
        }
    }
}

void RegionViewer::renderPlot(QPaintDevice* paintDevice, const RenderConfig& cfg)
{
    QPainter p(paintDevice);
    p.setRenderHint(QPainter::Antialiasing, true);
    paintMain(&p, cfg);
}

void RegionViewer::paintEvent(QPaintEvent* event)
{
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    paintMain(&p, _renderConfig);
    paintSelection(&p);
}

const RegionViewer::RenderConfig& RegionViewer::renderConfig() const
{
    return _renderConfig;
}

void RegionViewer::setMode(Mode mode)
{
    _mode = mode;
    if (!_region.isNull()) {
        setRegion(_region.get());
    }
}
}
