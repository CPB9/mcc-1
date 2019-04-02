#include "mcc/vis/ProfileViewer.h"
#include "mcc/vis/Profile.h"

#include <bmcl/OptionPtr.h>
#include <bmcl/ArrayView.h>
#include <bmcl/Math.h>

#include <QPainter>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDebug>

#include <cmath>

namespace mccvis {

ProfileViewer::ProfileViewer(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);

    QVector<qreal> rayPattern = {4, 4};
    QVector<qreal> gridPattern = {1, 3};

    _blue.setRgb(31, 120, 180);
    _green.setRgb(51, 160, 44);
    _black = Qt::black;
    _grey = Qt::gray;
    _red.setRgb(227, 26, 28);
    _viewBlue.setRgb(166, 206, 227, 0.2 * 255);
    _fillBlue.setRgb(166, 206, 227, 0.8 * 255);
    _fillGreen.setRgb(178, 223, 138, 0.8 * 255);
    _fillRed.setRgb(251, 154, 153, 0.8 * 255);

    _gridPen.setColor(_black);
    _gridPen.setWidthF(0.5);
    _gridPen.setStyle(Qt::DashLine);
    _gridPen.setDashPattern(gridPattern);
    _gridPen.setCosmetic(true);

    _earthPen.setColor(_blue);
    _earthPen.setWidth(2);
    _earthPen.setCosmetic(true);

    _targetPen.setColor(_green);
    _targetPen.setWidth(2);
    _targetPen.setCosmetic(true);

    _borderPen.setBrush(_black);
    _borderPen.setWidth(1);
    _borderPen.setCosmetic(true);

    _rayVisibleLimitsPen.setColor(_black);
    _rayVisibleLimitsPen.setStyle(Qt::DashLine);
    _rayVisibleLimitsPen.setDashPattern(rayPattern);
    _rayVisibleLimitsPen.setWidth(2);
    _rayVisibleLimitsPen.setCosmetic(true);

    _rayInvisibleLimitsPen.setColor(Qt::gray);
    _rayInvisibleLimitsPen.setStyle(Qt::DashLine);
    _rayInvisibleLimitsPen.setDashPattern(rayPattern);
    _rayInvisibleLimitsPen.setWidth(2);
    _rayInvisibleLimitsPen.setCosmetic(true);

    _raysPen.setColor(_red);
    _raysPen.setStyle(Qt::DashLine);
    _raysPen.setDashPattern(rayPattern);
    _raysPen.setWidth(2);
    _raysPen.setCosmetic(true);

    _backgroundCheckbox = new QCheckBox("Фон");
    _groundCheckbox = new QCheckBox("Земля");
    _viewAreaCheckbox = new QCheckBox("Зона видимости");

    connect(_backgroundCheckbox, &QCheckBox::toggled, this, [this](bool flag) {
        _renderConfig.drawBackground = flag;
        update();
    });
    connect(_groundCheckbox, &QCheckBox::toggled, this, [this](bool flag) {
        _renderConfig.drawGround = flag;
        update();
    });
    connect(_viewAreaCheckbox, &QCheckBox::toggled, this, [this](bool flag) {
        _renderConfig.drawViewArea = flag;
        update();
    });

    QHBoxLayout* cbLayout = new QHBoxLayout;
    cbLayout->addWidget(_backgroundCheckbox);
    cbLayout->addWidget(_groundCheckbox);
    cbLayout->addWidget(_viewAreaCheckbox);
    cbLayout->addStretch();
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(cbLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    _backgroundCheckbox->setChecked(_renderConfig.drawBackground);
    _groundCheckbox->setChecked(_renderConfig.drawGround);
    _viewAreaCheckbox->setChecked(_renderConfig.drawViewArea);
}

ProfileViewer::~ProfileViewer()
{
}

void ProfileViewer::setProfile(bmcl::OptionPtr<const Profile> profile)
{
    _data.clear();
    if (profile.isNone()) {
        return;
    }
    _data.emplace_back(profile.unwrap());
    onProfileReset();
}

void ProfileViewer::setProfiles(bmcl::ArrayView<Rc<const Profile>> profiles)
{
    _data.clear();
    _data.insert(_data.begin(), profiles.begin(), profiles.end());
    onProfileReset();
}

void ProfileViewer::onProfileReset()
{
    if (_data.empty()) {
        update();
        return;
    }

    const Profile* first = _data[0].profile.get();

    _xmin = first->rayRect().left();
    _xmax = first->rayRect().right();

    for (auto it = (_data.begin() + 1); it < _data.end(); it++) {
        _xmin = std::min(_xmin, it->profile->rayRect().left());
        _xmax = std::max(_xmax, it->profile->rayRect().right());
    }

    _xticks = Ticks::fromMinMax(_xmin, _xmax);
    _xmin = std::min(_xmin, _xticks.min);
    _xmax = std::max(_xmax, _xticks.max);
    _deltax = _xmax - _xmin;

    _totalDeltay = 0;

    for (Data& d : _data) {
        d.ymin = d.profile->rayRect().bottom();
        d.ymax = d.profile->rayRect().top();
        d.yticks = Ticks::fromMinMax(d.ymin, d.ymax);
        d.ymin = std::min(d.ymin, d.yticks.min);
        d.ymax = std::max(d.ymax, d.yticks.max);
        d.deltay = d.ymax - d.ymin;
        _totalDeltay += d.deltay;

        auto begin = d.profile->data().begin();
        auto end = d.profile->data().end();
        d.earthFillPath = QPainterPath();
        d.earthFillPath.moveTo(begin->profile.x(), begin->profile.y());
        for (auto it = begin + 1; it < end; it++) {
            d.earthFillPath.lineTo(it->profile);
        }
        d.earthStrokePath = d.earthFillPath;
        d.earthFillPath.lineTo((end - 1)->profile.x(), d.ymin);
        d.earthFillPath.lineTo(begin->profile.x(), d.ymin);
        d.earthFillPath.lineTo(begin->profile.x(), begin->profile.y());

        d.targetStrokePath = QPainterPath();
        d.targetStrokePath.moveTo(begin->target.x(), begin->target.y());
        for (auto it = begin + 1; it < end; it++) {
            d.targetStrokePath.lineTo(it->target);
        }

        d.viewRegionPath = QPainterPath();
        if (!d.profile->viewRegion().empty()) {
            d.viewRegionPath.moveTo(d.profile->viewRegion()[0]);
            for (const Point& p : d.profile->viewRegion()) {
                d.viewRegionPath.lineTo(p);
            }
        }
    }

    update();
}

void ProfileViewer::setTitle(const QString& title)
{
    _title = title;
}

void ProfileViewer::paintEvent(QPaintEvent* event)
{
    (void)event;
    renderPlot(this, _renderConfig);
    QWidget::paintEvent(event);
}

void ProfileViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

static void drawX(QPainter* p, const QPointF& pos, int r)
{
    p->drawLine(pos + QPointF(r, r), pos + QPointF(-r, -r));
    p->drawLine(pos + QPointF(-r, r), pos + QPointF(r, -r));
}

static int getXMargin(const QPaintDevice* dev)
{
    return dev->width() / 10;
}

static int getYMargin(const QPaintDevice* dev)
{
    return dev->height() / 10;
}

void ProfileViewer::drawTriangle(QPainter* p, const QPointF& pos, int size, bool isUp)
{
    p->setPen(_borderPen);
    int halfSize;
    QBrush brush;
    if (isUp) {
        brush = QBrush(Qt::green);
        halfSize = -size / 2;
    } else {
        brush = QBrush(Qt::red);
        halfSize = size / 2;
    }
    QPainterPath path;
    path.moveTo(pos + QPoint(-halfSize, -halfSize));
    path.lineTo(pos + QPoint(halfSize, -halfSize));
    path.lineTo(pos + QPoint(0, halfSize));
    path.lineTo(pos + QPoint(-halfSize, -halfSize));
    p->setBrush(brush);
    p->drawPath(path);
}

void ProfileViewer::drawStar(QPainter* p, const QPointF& pos, int radius)
{
    constexpr const double pi = bmcl::pi<double>();
    p->setPen(_borderPen);
    p->setBrush(Qt::darkGray);
    QPolygonF points;
    double currentPhi = -pi / 2.0;
    double step = 2.0 * pi / 5.0;

    points.reserve(6);
    points << pos + QPointF(0, -radius);
    for (int i = 0; i < 5; i++) {
        points << pos + QPointF(radius * std::cos(currentPhi), radius * std::sin(currentPhi));
        points << pos + QPointF(0.5 * radius * std::cos(currentPhi + step / 2), 0.5 * radius * std::sin(currentPhi + step / 2));
        currentPhi += step;
    }
    p->drawPolygon(points, Qt::OddEvenFill);
}

void ProfileViewer::renderPlot(QPaintDevice* dev, const RenderConfig& cfg)
{
    QPainter p(dev);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRect allRect(0, 0, dev->width(), dev->height());
    if (cfg.drawBackground) {
        p.fillRect(allRect, Qt::lightGray);
    } else {
        p.fillRect(allRect, Qt::white);
    }

    int plotXmargin = getXMargin(dev);
    int plotYmargin = getYMargin(dev);

    p.setPen(_borderPen);

    double fontScale = 1;
    if (dev->width() < dev->height()) {
        fontScale = dev->width() / double(qApp->primaryScreen()->size().width());
    } else {
        fontScale = dev->height() / double(qApp->primaryScreen()->size().height());
    }
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPointSizeF(10 * fontScale);
    p.setFont(font);
    QFontMetrics metrics(font);

    QRect titleRect = metrics.boundingRect(_title);
    QPointF titlePos(dev->width() / 2, plotYmargin - 2 * metrics.descent());
    p.save();
    p.translate(titlePos);
    p.drawText(-titleRect.width() / 2, 0, _title);
    p.restore();

    if (_data.empty()) {
        return;
    }

    double xscale = ((dev->width() - 2 * plotXmargin) / _deltax);
    double yOffsetPerPlot = (dev->height() - 2 * plotYmargin) / double(_data.size());
    double currentYOffset = 0;

    bool isFirst = true;

    for (auto it = _data.rbegin(); it < _data.rend(); it++) {
        const Data& d = *it;
        p.setClipping(false);
        p.setBrush(Qt::white);
        p.setPen(Qt::black);
        QRect borderRect(plotXmargin, plotYmargin, dev->width() - 2 * plotXmargin, dev->height() - 2 * plotYmargin - currentYOffset);
        p.fillRect(borderRect, Qt::white);
        p.drawRect(borderRect);

        double yscale = ((dev->height() - 2 * plotYmargin) / d.deltay) / _data.size();

        QTransform plotTransform;
        plotTransform.translate(0, dev->height());
        plotTransform.translate(plotXmargin, -(plotYmargin + currentYOffset));
        plotTransform.scale(1 * xscale, -1 * yscale);
        plotTransform.translate(-_xmin, -d.ymin);

        double maxTextHeight = 0;
        if (isFirst) {
            for (double i = _xticks.min; i <= _xticks.max; i += _xticks.step) {
                p.save();
                QString text = QString::number(i);
                QRect textRect = metrics.tightBoundingRect(text);
                maxTextHeight = std::max<double>(maxTextHeight, textRect.width());
                QPointF pos = plotTransform.map(QPointF(i, d.ymin));
                p.translate(pos);
                p.rotate(-90);
                p.drawText(-textRect.bottomRight() - QPointF(metrics.descent(), -metrics.descent()), text);
                p.restore();
            }
        }

        double maxTextWidth = 0;
        for (double i = d.yticks.min; i <= d.yticks.max; i += d.yticks.step) {
            p.save();
            QString text = QString::number(i);
            QRect textRect = metrics.tightBoundingRect(text);
            QPointF pos = plotTransform.map(QPointF(_xmin, i));
            p.translate(pos);
            maxTextWidth = std::max<double>(maxTextWidth, textRect.width());
            p.drawText(-textRect.bottomRight() - QPointF(metrics.descent(), -metrics.descent()), text);
            p.restore();
        }


        if (isFirst) {
            QString xlabel = "Дальность (м)";
            QRect xlabelRect = metrics.boundingRect(xlabel);
            QPointF xlabelPos(dev->width() / 2, dev->height() - plotYmargin + 2 * metrics.descent() + maxTextHeight);
            p.save();
            p.translate(xlabelPos);
            p.drawText(-xlabelRect.width() / 2, xlabelRect.height(), xlabel);
            p.restore();

            QString ylabel = "Высота (м)";
            QRect ylabelRect = metrics.boundingRect(ylabel);
            QPointF ylabelPos(plotXmargin - maxTextWidth - 2 * metrics.descent(), dev->height() / 2);
            p.save();
            p.translate(ylabelPos);
            p.rotate(-90);
            p.drawText(-ylabelRect.width() / 2, 0, ylabel);
            p.restore();

            QRect posRect = metrics.boundingRect(_positionText);
            QPointF posPos(dev->width() - posRect.width() - 2 * metrics.descent(), 2* metrics.descent() + posRect.height());
            p.drawText(posPos, _positionText);
        }

        {
            QString azText = "Азимут " + QString::number(d.profile->direction()) + "°";
            QRect azRect = metrics.boundingRect(azText);
            QPointF azPos = plotTransform.map(QPointF(_xmax, d.ymin + (d.ymax - d.ymin) / 2));
            azPos.rx() += azRect.height() / 2 + 2 * metrics.descent();
            p.save();
            p.translate(azPos);
            p.rotate(-90);
            p.drawText(-azRect.width() / 2, 0, azText);
            p.restore();
        }

        p.setClipRect(borderRect);
        p.setTransform(plotTransform);

        if (cfg.drawViewArea) {
            p.fillPath(d.viewRegionPath, _viewBlue);
        }
        if (cfg.drawGround) {
            p.fillPath(d.earthFillPath, _fillBlue);
        }
        p.strokePath(d.earthStrokePath, _earthPen);
        p.strokePath(d.targetStrokePath, _targetPen);

        const Rays& rays = d.profile->rays();
        p.setPen(_rayInvisibleLimitsPen);
        p.drawLine(rays.start, rays.minEdge);
        p.drawLine(rays.start, rays.maxEdge);
        p.setPen(_rayVisibleLimitsPen);
        p.drawLine(rays.start, rays.minEnd);
        p.drawLine(rays.start, rays.maxEnd);

        p.setPen(_raysPen);
        for (const auto& end : rays.ends) {
            if (end.hasTargetIntersections) {
                p.drawLine(rays.start, end.end);
            }
        }
        if (!rays.ends.empty()) {
            if (d.profile->params().canViewGround && !rays.ends.back().hasTargetIntersections) {
                QPen rayPen = _raysPen;
                QColor c = _raysPen.color();
                c.setAlpha(85);
                rayPen.setColor(c);
                p.setPen(rayPen);
                p.drawLine(rays.start, rays.ends.back().end);
            }
        }

        p.setPen(_gridPen);
        for (double i = _xticks.min; i <= _xticks.max; i += _xticks.step) {
            p.drawLine(i, d.ymin, i, d.ymax);
        }

        for (double i = d.yticks.min; i <= d.yticks.max; i += d.yticks.step) {
            p.drawLine(_xmin, i, _xmax, i);
        }

        QPen limitPen;
        limitPen.setCosmetic(true);
        limitPen.setColor(_green);
        limitPen.setStyle(Qt::DashLine);
        limitPen.setWidthF(1.5);
        p.setPen(limitPen);
        p.drawLine(d.profile->params().minBeamDistance, d.ymin, d.profile->params().minBeamDistance, d.ymax);
        p.drawLine(d.profile->params().maxBeamDistance, d.ymin, d.profile->params().maxBeamDistance, d.ymax);
        limitPen.setColor(_red);
        p.setPen(limitPen);
        p.drawLine(d.profile->params().minHitDistance, d.ymin, d.profile->params().minHitDistance, d.ymax);
        p.drawLine(d.profile->params().maxHitDistance, d.ymin, d.profile->params().maxHitDistance, d.ymax);

        p.setTransform(QTransform());

        bool isUp = !d.profile->params().isTargetDirectedTowards;
        for (const auto& i : d.profile->viewIntersections()) {
            QPointF point = plotTransform.map(i);
            drawTriangle(&p, point, 10, isUp);
            isUp = !isUp;
        }

        for (const auto& i : d.profile->hits()) {
            QPointF point = plotTransform.map(i.detection);
            drawStar(&p, point, 7);
        }
        QPen xpen;
        xpen.setColor(Qt::red);
        xpen.setCosmetic(true);
        xpen.setWidth(2);
        p.setPen(xpen);
        for (const auto& i : d.profile->hits()) {
            QPointF point = plotTransform.map(i.hit);
            drawX(&p, point, 4);
        }
        xpen.setColor(Qt::blue);
        p.setPen(xpen);
        for (const auto& i : d.profile->outOfRangeHits()) {
            QPointF point = plotTransform.map(i.hit);
            drawX(&p, point, 4);
        }
        currentYOffset += yOffsetPerPlot;
        isFirst = false;
    }
}

void ProfileViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (_data.empty()) {
        return;
    }
    event->accept();
    int plotXmargin = getXMargin(this);
    int plotYmargin = getYMargin(this);
    QRect plotRect = rect().marginsRemoved(QMargins(plotXmargin, plotYmargin, plotXmargin, plotYmargin));
    if (!plotRect.contains(event->pos())) {
        if (!_positionText.isNull()) {
            _positionText.clear();
            update();
        }
        return;
    }
    int allPlotsHeight = height() - 2 * plotYmargin;
    QPoint posInside = event->pos() - QPoint(plotXmargin, plotYmargin);
    posInside.ry() = allPlotsHeight - posInside.y();
    int yPixelsPerPlot = allPlotsHeight / _data.size();
    int plotIndex = posInside.y() / yPixelsPerPlot;
    int yPixelOffset = posInside.y() - yPixelsPerPlot * plotIndex;
    int xPixelsPerPlot = width() - 2 * plotXmargin;
    int xPixelOffset = posInside.x();
    const Data& d = _data[plotIndex];
    double distance = _xmin + xPixelOffset / double(xPixelsPerPlot) * (_xmax - _xmin);
    double height = d.ymin + yPixelOffset / double(yPixelsPerPlot) * (d.ymax - d.ymin);

    _positionText = QString("d = %1м, h = %2м").arg(distance).arg(height);
    update();
}

const ProfileViewer::RenderConfig& ProfileViewer::renderConfig() const
{
    return _renderConfig;
}
}
