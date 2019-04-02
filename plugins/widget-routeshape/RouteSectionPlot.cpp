#include "RouteSectionPlot.h"
#include "CanvasPicker.h"
#include "RouteCurve.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/Route.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/HeightmapController.h"
#include "mcc/uav/Uav.h"
#include "mcc/vis/RadarGroup.h"
#include "mcc/geo/Constants.h"

#include <bmcl/Logging.h>
#include <bmcl/DoubleEq.h>

#include <cfloat>
#include <limits>

#include <qwt.h>
#include <qwt_painter.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_point_mapper.h>
#include <qwt_symbol.h>

#include <QRectF>

const QString colorSurfBrush("#B8B799");
const QString colorSurfPen("#7f0000");
const QString colorFrenelVis("#67E667");

using mccuav::Route;
using namespace mccui;

class PointVectorRefData : public QwtSeriesData<QPointF> {
public:
    PointVectorRefData(const std::vector<QPointF>* ref)
        : _ref(ref)
    {
    }

    QRectF boundingRect() const override
    {
        if (_boundingRect.width() < 0) {
            _boundingRect = qwtBoundingRect(*this);
        }

        return _boundingRect;
    }

    size_t size() const override
    {
        return _ref->size();
    }

    QPointF sample(size_t i) const override
    {
        return (*_ref)[i];
    }

private:
    const std::vector<QPointF>* _ref;
    mutable QRectF _boundingRect;
};

class DeviceMarker : public QwtPlotMarker {
public:
    DeviceMarker() :
        isOnColor(QColor(255, 0, 0, 255))
       ,isOutColor(QColor(255, 0, 0, 100))
       ,sym(new QwtSymbol(QwtSymbol::Ellipse, Qt::red, Qt::NoPen, QSize(10, 10)))
    {
        setSymbol(sym);
        setVisible(false);
        setLabelAlignment(Qt::AlignLeft);
    }

    void set(mccuav::Uav* device, double onRouteDist, double fromRouteDist)
    {
        if (onRouteDist < 0 || fromRouteDist < 0) {
            return;
        }

        setVisible(true);
        setXValue(onRouteDist);
        setYValue(device->motion()->position.altitude());
        setStatusLabel(device, fromRouteDist);
    }

    void setStatusLabel(mccuav::Uav* device, double fromRouteDist)
    {
        QString x;
        //x += QString("Имя: %1\n").arg(device->getInfo(), -10);
        x += QString("Высота: %2м\n").arg(QString::number(device->motion()->position.altitude(), 'f', 2), 0);
        x += QString("Удаленность: %1км\n").arg(QString::number(fromRouteDist, 'f', 2), 0);
        setLabel(x);
    }

    void drawLabel(QPainter *painter, const QRectF& canvasRect, const QPointF& pos) const override{
        painter->setFont(QFont("family", 7, 10, false));
        QRectF rect = QRectF(pos.x() - 50, pos.y() - 75, 100, 100);
        label().setRenderFlags(Qt::AlignLeft);
        label().draw(painter, rect);
    }

    QColor isOnColor;
    QColor isOutColor;
    QwtSymbol* sym;
};

class Picker : public QwtPlotPicker {
public:
    explicit Picker(QWidget* canvas)
        : QwtPlotPicker(canvas)
    {
    }

    QwtText trackerTextF(const QPointF&) const override
    {
        return text;
    }

    QString text;
};

RouteSectionPlot::RouteSectionPlot(mccui::Settings* settings,
                       mccuav::RoutesController* routesController,
                       mccuav::UavController* uavController,
                       const bmcl::Option<std::shared_ptr<mccvis::RadarGroup>>& radarController,
                       QWidget* parent)
    : QwtPlot(parent)
    , _route(0)
    , _canvas(new QwtPlotCanvas)
    , _curve(new RouteCurve)
    , _grid(new QwtPlotGrid)
    , _curve_srtm(new QwtPlotCurve)
    , _track(new QwtPlotCurve)
    , _trackLen(666)
    , _recalc(false)
    , _deviceMarker(new DeviceMarker)
    , _radarGroup(radarController)
    , _settings(settings)
    , _routesController(routesController)
    , _uavController(uavController)
    , _currentUav(nullptr)
    , _geod(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>())
{
    _hmReader = new mcchm::EmptyHmReader;

    _track->setSamples(new PointVectorRefData(&_trackPoints));
    setObjectName("Профиль маршрута");
    setWindowTitle("Профиль маршрута");
    setWindowIcon(QIcon(":/routeshape/icon.png"));

    setCanvas(_canvas.get());
    setCanvasBackground(QColor(245, 245, 245));
    _canvas->setCursor(Qt::ArrowCursor);

    _curve_srtm->attach(this);
    QColor srtmColor(colorSurfBrush);
    srtmColor.setAlpha(255 * 0.5);
    _curve_srtm->setBrush(srtmColor);
    QColor srtmPenColor(colorSurfPen);
    srtmPenColor.setAlpha(255 * 0.5);
    _curve_srtm->setPen(srtmPenColor, 2);
    _curve_srtm->setRenderHint(QwtPlotItem::RenderAntialiased);

    _curve->attach(this);
    _curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    _curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse));
    enableAxis(QwtPlot::xBottom, true);
    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(0.5);
    _grid->attach(this);
    _grid->setMajorPen(pen);
    QwtText bottomTitle("Длина маршрута (км)");
    QwtText leftTitle("Высота (м)");
    bottomTitle.setFont(this->font());
    leftTitle.setFont(this->font());
    setAxisTitle(QwtPlot::xBottom, bottomTitle);
    setAxisTitle(QwtPlot::yLeft, leftTitle);

    _currentPos = new QwtPlotMarker();
    _currentPos->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
    _currentPos->setLabelOrientation(Qt::Vertical);
    _currentPos->setLineStyle(QwtPlotMarker::VLine);
    _currentPos->setLinePen(QPen(Qt::red, 1, Qt::SolidLine));
    _currentPos->setXValue(0);
    _currentPos->attach(this);
    _currentPos->setVisible(false);

    _hintMarker = new QwtPlotMarker();
    _hintMarker->setSymbol(new QwtSymbol(QwtSymbol::Rect, Qt::green, QPen(Qt::black), QSize(10, 10)));
    _hintMarker->setVisible(false);
    _hintMarker->attach(this);
    _hintMarker->setZ(100);

    _picker = new CanvasPicker(this);
    _picker->setCurve(_curve.get());

    setAxisAutoScale(QwtPlot::yLeft, false);

    connect(_routesController.get(), &mccuav::RoutesController::selectedRouteChanged, this, &RouteSectionPlot::setRoute);
    connect(_routesController.get(), &mccuav::RoutesController::routeEditingChanged, this,
            [this]()
    {
        setRoute(_routesController->selectedRoute());
    });
}

RouteSectionPlot::~RouteSectionPlot()
{
    delete _deviceMarker;
}

void RouteSectionPlot::setHeightmapController(bmcl::OptionPtr<const mccui::HeightmapController> hmController)
{
    if (_hmController.isSome()) {
        disconnect(_hmController.unwrap().get(), &mccui::HeightmapController::heightmapReaderChanged, this, 0);
    }
    if (hmController.isSome()) {
        _hmController = hmController;
        _hmReader = _hmController->cloneHeightmapReader();
        connect(_hmController.unwrap().get(), &mccui::HeightmapController::heightmapReaderChanged, this, [this](const bmcl::Rc<const mcchm::HmReader>& reader) {
            _hmReader = reader;
            recalcData();
        });
    }
}

void RouteSectionPlot::setRoute(mccuav::Route* route)
{
    if (!_recalc)
        return;

    if(_routesController->isEditing() && route != nullptr && route->buffer() != nullptr)
        route = route->buffer();

    if (route && route->buffer() == nullptr)
        _picker->setRoute(route);
    else
        _picker->setRoute(nullptr);

    disconnectAll();
    _route = route;

    reconnectDevice();
    recalcData();
    connectAll();
}

void RouteSectionPlot::recalcData()
{
    _distances.clear();
    _heights.clear();
    _srtmDistance.clear();
    _srtmAlt.clear();
    _frenelVisZs.clear();
    _trackPoints.clear();

    if (!_route || _route->waypointsCount() == 0) {
        resetPlotData();
        return;
    }
    _curve->setLoop(_route->isLoop());

    int rSize = _route->waypointsCount();
    if (rSize == 0) {
        resetPlotData();
        return;
    }

    const mccmsg::Waypoints& lst = _route->waypointsList();
    std::vector<std::vector<mccgeo::PositionAndDistance>> routeProfiles;
    routeProfiles.reserve(rSize + 1);
    _heights.reserve(rSize + 2);
    _distances.reserve(rSize + 2);
    double totalDistance = 0;

    for (int i = 1; i < rSize; i++) {
        double d = 0;
        _geod.inverse(lst[i-1].position, lst[i].position, &d, 0, 0);
        totalDistance += d;
    }

    if (rSize >= 2 && _route->isLoop()) {
        double d = 0;
        _geod.inverse(lst.back().position, lst.front().position, &d, 0, 0);
        totalDistance += d;
    }

    double metersPerPixel = totalDistance / canvas()->width(); //примерно

    totalDistance = 0;

    for (int i = 1; i < rSize; i++) {
        std::vector<mccgeo::PositionAndDistance> pAds = _hmReader->profile(lst[i - 1].position, lst[i].position, metersPerPixel);
        _heights.push_back(lst[i-1].position.altitude());
        _distances.push_back(totalDistance / 1000);
        totalDistance += pAds.back().distance();
        routeProfiles.push_back(std::move(pAds));
    }
    _heights.push_back(lst.back().position.altitude());
    _distances.push_back(totalDistance / 1000);

    if (rSize >= 2 && _route->isLoop()) {
        std::vector<mccgeo::PositionAndDistance> pAds = _hmReader->profile(lst.back().position, lst.front().position, metersPerPixel);
        _heights.push_back(lst.front().position.altitude());
        totalDistance += pAds.back().distance();
        _distances.push_back(totalDistance / 1000);
        routeProfiles.push_back(std::move(pAds));
    }

    drawRoute(routeProfiles);
}

void RouteSectionPlot::recalcDataOnlyAlt()
{
    if (!_route) {
        resetPlotData();
        return;
    }
    if (_route->waypointsCount() == 0) {
        resetPlotData();
        return;
    }

    _heights.clear();
    int rSize = _route->waypointsCount();
    _heights.reserve(rSize + 2);
    const mccmsg::Waypoints& lst = _route->waypointsList();
    for (int i = 1; i < rSize; i++) {
        _heights.push_back(lst[i - 1].position.altitude());
    }
    _heights.push_back(lst.back().position.altitude());

    if (rSize >= 2 && _route->isLoop()) {
        _heights.push_back(lst.front().position.altitude());
    }
    if (_route->waypointHint().isSome())
    {
        int idx = _route->waypointHint()->indexAfter;
        double x0 = _distances[idx];
        double x1 = _distances[idx + 1];
        double y0 = _heights[idx];
        double y1 = _heights[idx + 1];
        double dx = _route->waypointHint()->distance / 1000;
        double x = x0 + dx;;
        double y = 0;
        if (y1 > y0)
            y = dx * (y1 - y0) / (x1 - x0) + y0;
        else
            y = (x1 - x0 - dx)*(y0 - y1) / (x1 - x0) + y1;

        _hintMarker->setXValue(x);
        _hintMarker->setYValue(y);

        _hintMarker->setVisible(true);
    }
    else
    {
        _hintMarker->setVisible(false);
        _waypointHint = bmcl::None;
    }

    resetPlotData();
}


static std::vector<mccvis::Point> visionArea(const mccvis::RadarPtr& radar,
                                             const mcchm::HmReader* handler,
                                             const mccgeo::Geod& geod,
                                             const std::vector<mccgeo::PositionAndDistance>& profile)
{
    std::vector<mccvis::Point> top;
    top.reserve(profile.size()); //worst case
    std::vector<mccvis::Point> bot;
    bot.reserve(profile.size()); //worst case

    const auto& _params = radar->viewParams();
    const auto& _position = radar->position();

    for (const mccgeo::PositionAndDistance& pAd : profile) {
        const mccgeo::Position& point = pAd.position();
        double d = 0;
        double a1 = 0;
        double a2 = 0;
        geod.inverse(_position, point.latLon(), &d, &a1, &a2);
        if (d > _params.maxBeamDistance || d < _params.minBeamDistance) {
            continue;
        }
        double a1norm = a1;
        while (a1norm < _params.minAzimuth) {
            a1norm += 360;
        }
        if (!(a1norm >= _params.minAzimuth && a1norm <= _params.maxAzimuth)) {
            continue;
        }
        mccgeo::LatLon newEnd;
        geod.direct(_position, a1, d, &newEnd, &a2);

        std::vector<mccvis::Point> slice;
        if (_params.useCalcStep) {
            double step = std::max(90.0, _params.calcStep);
            slice = handler->relativePointProfile(_position, newEnd, step);
        } else {
            slice = handler->relativePointProfileAutostep(_position, newEnd);
        }

        slice.push_back(mccvis::Point(d, point.altitude()));
        mccvis::Profile vI(a1norm, slice, _params);
        bmcl::Option<std::pair<double, double>> interval = vI.verticalVisionIntervalAt(d);

        if (interval.isSome()) {
            double botP = interval->first;
            double topP = interval->second;
            bot.emplace_back(pAd.distance() / 1000, botP);
            top.emplace_back(pAd.distance() / 1000, topP);
        }
    }
    bot.insert(bot.end(), top.rbegin(), top.rend());
    return bot;
}


void RouteSectionPlot::drawRoute(const std::vector<std::vector<mccgeo::PositionAndDistance>>& routeProfiles)
{
    double totalDistance = 0;
    std::vector<mccgeo::PositionAndDistance> totalProfile;
    totalProfile.reserve(routeProfiles.size());
    for (const auto& profile : routeProfiles) {
        for (const mccgeo::PositionAndDistance& pAd : profile) {
            totalProfile.emplace_back(pAd.latitude(), pAd.longitude(), pAd.altitude(), pAd.distance() + totalDistance);
        }
        if (profile.size() > 0) {
            totalDistance += profile.back().distance();
        }
    }

    _srtmAlt.reserve(totalProfile.size());
    _srtmDistance.reserve(totalProfile.size());
    for (mccgeo::PositionAndDistance& pAd : totalProfile) {
        auto& point = pAd.position();
        _srtmAlt.push_back(point.altitude());
        _srtmDistance.push_back(pAd.distance()/ 1000);
    }

    if (_radarGroup.isSome()) {
        for (auto& rad : _radarGroup.unwrap()->radars()) {
            QColor color = QColor::fromRgba(rad->viewParams().viewZonesColorArgb);
            _frenelVisZs.emplace_back(visionArea(rad, _hmReader.get(), _geod, totalProfile), std::move(color));
        }
    }

    _currentDistance = totalDistance / 2;
    resetPlotData();
}

void RouteSectionPlot::resetPlotData()
{
    auto size = _distances.size();
    _curve->setRawSamples(_distances.data(), _heights.data(), (int)size);
    for (auto& z : _frenelVisZs) {
        std::unique_ptr<QwtPlotCurve> curve(new RouteCurve);
        curve->setBrush(z.color);
        curve->setPen(z.color, 1);
        curve->setRenderHint(QwtPlotItem::RenderAntialiased);
        curve->setSamples(new PointVectorRefData(&z.points));
        curve->attach(this);
    }

    _curve_srtm->setRawSamples(_srtmDistance.data(), _srtmAlt.data(), (int)_srtmDistance.size());
    _currentPos->setXValue(_currentDistance);
    _curve_srtm->detach();
    _curve_srtm->attach(this);

    _curve->detach();
    if (_route && _route->waypointsCount() > 0) {
        mccuav::RouteState state = _route->state();
        QPen pen;
        if (state == mccuav::RouteState::UnderEdit) {
            pen = _route->style()->editable.linePen;
        }
        else
        {
            pen = _route->pen();
        }

        pen.setWidth(1);
        _curve->setPen(pen);
        _curve->setActiveIndex((int)_route->activePointIndex().unwrapOr(-1));

        //draw incline
        _inclines.clear();
        assert(_distances.size() == _heights.size());
        if (_distances.size() > 1) {
            for (size_t i = 0; i < _distances.size() - 1; i++) {
                double x0 = _distances[i];
                double y0 = _heights[i];
                double x1 = _distances[i + 1];
                double y1 = _heights[i + 1]* 1.15;
                if (bmcl::doubleEq(x1, x0))
                    continue;
                double incline = (y1 - y0) / (x1 - x0);
                if (fabs(incline) < 10) continue;
                QwtPlotMarker *m = new QwtPlotMarker();
                m->setXValue(x0 + 0.5*(x1 - x0));
                m->setYValue(y0 + 0.5*(y1 - y0));
                QString note = QString("Наклон: %1 м/км").arg(QString::number(incline, 'f', 2), 0);
                m->setLabel(QwtText(note));
                _inclines.emplace_back(m);
                m->attach(this);
            }
        }
        _curve->attach(this);

        _deviceMarker->detach();
        _deviceMarker->attach(this);
    }
    else
    {
        _trackPoints.clear();
        _deviceMarker->detach();
    }

    rescale();
    replot();
}

void RouteSectionPlot::rescale()
{
    double yMax = qMax(_deviceMarker->yValue(), qMax(_curve->maxYValue(), _curve_srtm->maxYValue()))* 1.35;
    double yMin = qMin(_deviceMarker->yValue(), qMin(_curve->minYValue(), _curve_srtm->minYValue()));
    if (yMax < 1.0)
        yMax = 5;

    if (yMin > 0)
        yMin *= 0.7;
    else if (yMin > -1.0)
        yMin = -5;
    else
        yMin *= 1.3;

    double delta = 0;
    if (yMax < 100)
        delta = 50;
    else if (yMax < 1000)
        delta = 100;
    else
        delta = 500;

    int c = yMax / delta + 1;
    yMax = c * delta;

    double deltaMin = 0;
    if (yMin < 0)
        deltaMin = -10;
    else if (yMin < -100)
        deltaMin = -50;
    else
        deltaMin = -100;

    if (yMin < 0) {
        c = yMin / deltaMin + 1;
        yMin = deltaMin * c;
    }

    setAxisScale(QwtPlot::yLeft, yMin, yMax);
}

void RouteSectionPlot::clearTrack()
{
    _trackPoints.clear();
}

void RouteSectionPlot::updateSelectedWaypoints()
{
    auto selectedIndexes = _route->selectedPointIndexes();

    auto containsFn = [&selectedIndexes](int index) {
        return std::find(selectedIndexes.begin(), selectedIndexes.end(), index) != selectedIndexes.end();
    };

    if (_route->isLoop() && _route->isPointSelected(0))
    {
        if (!containsFn(_route->waypointsCount()))
            selectedIndexes.push_back(_route->waypointsCount());
    }
    _curve->setSelectedIndices(selectedIndexes);
}

void RouteSectionPlot::connectAll()
{
    using mccuav::Route;
    if (!_route) {
        return;
    }
    connect(_route, &Route::waypointChanged, this, [this](const mccmsg::Waypoint&, int) { recalcData(); clearTrack(); });
    connect(_route, &Route::waypointMoved, this, [this](int, int) { recalcData(); clearTrack(); });
    connect(_route, &Route::waypointRemoved, this, [this](int) { recalcData(); clearTrack(); });
    connect(_route, &Route::waypointInserted, this, [this](const mccmsg::Waypoint&, int) { recalcData(); clearTrack(); });
    connect(_route, &Route::allWaypointsChanged, this, [this]() { recalcData(); clearTrack(); });
    connect(_route, &Route::closedPathFlagChanged, this, [this](bool) { recalcData(); });
    connect(_route, &Route::waypointOnlyAltChanged, this, [this](void) { recalcDataOnlyAlt(); });
    connect(_route, &Route::activeWaypointChanged, this, [this](void) { recalcDataOnlyAlt(); });
//    connect(_route, &Route::styleChagned, this, [this](const mccuav::RouteStylePtr& style) { _picker->_selectedPoint = -1; recalcDataOnlyAlt();});
    connect(_route, &Route::waypointHintChanged, this, [this]() { recalcDataOnlyAlt(); });
    connect(_route, &Route::selectedWaypointsChanged, this, [this]() { updateSelectedWaypoints(); });

    connect(_picker, &CanvasPicker::markerSelectionChanged, this, [this](void) { recalcDataOnlyAlt(); });

    if (_radarGroup.isSome()) {
        auto r = _radarGroup->get();
        connect(r, &mccvis::RadarGroup::radarAdded, this, [this]() { recalcData(); });
        connect(r, &mccvis::RadarGroup::radarRemoved, this, [this]() { recalcData(); });
        connect(r, &mccvis::RadarGroup::radarsReset, this, [this]() { recalcData(); });
        connect(r, &mccvis::RadarGroup::radarUpdated, this, [this]() { recalcData(); });
    }

    _settings->onChange("map/heightMapCachePath", this, [this](const QVariant& value) {
        recalcData();
    });
    connect(_uavController.get(), &mccuav::UavController::selectionChanged, this, [this]() { reconnectDevice(); });
}

void RouteSectionPlot::reconnectDevice()
{
    mccuav::Uav* curDevice = _uavController->selectedUav();
    if (_currentUav == curDevice)
        return;
    if (_currentUav)
        disconnect(_currentUav, 0, this, 0);
    _currentUav = curDevice;
    if (_currentUav)
        connect(_currentUav, &mccuav::Uav::motionChanged, this, &RouteSectionPlot::drawDevice);
}

void RouteSectionPlot::drawDevice()
{
    if (!_currentUav || !_route || _currentUav->motion().isNone())
        return;
    if (_lastPosition == _currentUav->motion()->position)
        return;

    _lastPosition = _currentUav->motion()->position;
    if (_lastPosition.isNone())
        return;

    const auto& pos = *_lastPosition;

    _deviceMarker->detach();
    _deviceMarker->attach(this);
    double fromRouteDist = -1;
    double onRouteDist = -1;
    relativeProfileDistanses(pos, *_route, &onRouteDist, &fromRouteDist);
    _deviceMarker->set(_currentUav, onRouteDist, fromRouteDist);

    //draw track
    mccvis::Point trackP(onRouteDist, pos.altitude());
    if (_trackPoints.size() >= _trackLen) {
        _trackPoints.erase(_trackPoints.begin());
    }
    _trackPoints.push_back(trackP);
    _track->setPen(Qt::red, 1);
    _track->attach(this);

    rescale();
    replot();
};

void RouteSectionPlot::relativeProfileDistanses(const mccgeo::Position& p, const mccuav::Route& r, double* onRouteDist1, double* fromRouteDist1)
{
    double minDist = std::numeric_limits<double>::max();
    double additionalDist = 0;
    const mcchm::HmReader* reader = _hmReader.get();
    for (int k = 1; k < r.waypointsCount(); k++) {
        const mccmsg::Waypoint &wp1 = r.waypointAt(k - 1);
        const mccmsg::Waypoint &wp2 = r.waypointAt(k);
        double wpDist = 0;
        double wp1PDist = 0;
        double wp2PDist = 0;
        double wpAlpha = 0;
        _geod.inverse(wp1.position, wp2.position, &wpDist, &wpAlpha, 0);
        _geod.inverse(wp1.position, p, &wp1PDist, 0, 0);
        _geod.inverse(p, wp2.position, &wp2PDist, 0, 0);
        if (!std::isnormal(wpDist) || !std::isnormal(wp1PDist) || !std::isnormal(wp2PDist))
            continue;

        double onRouteDist = (wp1PDist / (wp1PDist + wp2PDist)) * wpDist;
        if (!std::isnormal(onRouteDist))
            continue;

        mccgeo::Position onProfilePoint;
        _geod.direct(wp1.position, wpAlpha, onRouteDist, &onProfilePoint, 0);

        double fromRouteDist = 0;
        _geod.inverse(onProfilePoint, p, &fromRouteDist, 0, 0);

        onRouteDist += additionalDist;
        if (minDist > fromRouteDist) {
            minDist = fromRouteDist;
            *fromRouteDist1 = fromRouteDist / 1000;
            *onRouteDist1 = onRouteDist / 1000;
        }

        additionalDist += wpDist;
    }

}

void RouteSectionPlot::disconnectAll()
{
    if (_route) {
        disconnect(_route, 0, this, 0);
    }
//    _picker->_selectedPoint = -1;
    recalcDataOnlyAlt();
    if (_radarGroup.isSome()) {
        disconnect(_radarGroup->get(), 0, this, 0);
    }
}

void RouteSectionPlot::setEmpty()
{
    _picker->setRoute(nullptr);
    _route = nullptr;
    recalcData();
    _currentPos->setVisible(false);
}

QSize RouteSectionPlot::sizeHint() const
{
    return QSize(100, 100);
}

QSize RouteSectionPlot::minimumSizeHint() const
{
    return QSize(100, 100);
}

void RouteSectionPlot::showEvent(QShowEvent *event)
{
    _recalc = true;
    setRoute(const_cast<mccuav::Route*>(_route));
}

void RouteSectionPlot::hideEvent(QHideEvent *event)
{
    _recalc = false;
    setRoute(const_cast<mccuav::Route*>(_route));
}

void RouteSectionPlot::setRadarController(const bmcl::Option<std::shared_ptr<mccvis::RadarGroup>>& radarController)
{
    disconnectAll();
    _radarGroup = radarController;
    connectAll();
}
