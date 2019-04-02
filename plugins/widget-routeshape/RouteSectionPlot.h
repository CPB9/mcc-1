#pragma once

#include "mcc/Config.h"
#include "mcc/vis/Point.h"
#include "mcc/geo/Position.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"
#include "mcc/ide/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"
#include "mcc/vis/Fwd.h"
#include "mcc/hm/Fwd.h"
#include "mcc/msg/Nav.h"
#include "mcc/geo/Geod.h"

#include <bmcl/Option.h>
#include <bmcl/OptionRc.h>

#include <qwt_plot.h>

#include <memory>
#include <vector>

class QwtPlotCurve;
class QwtPlotCanvas;
class QwtPlotGrid;
class QwtPlotMarker;
class QwtSymbol;

class CanvasPicker;
class DeviceMarker;
class RouteCurve;

namespace mccui { class Uav; }

class RouteSectionPlot : public QwtPlot {
    Q_OBJECT
public:
    explicit RouteSectionPlot(mccui::Settings* settings,
                       mccuav::RoutesController* routesController,
                       mccuav::UavController* uavController,
                       const bmcl::Option<std::shared_ptr<mccvis::RadarGroup>>& radarController, //TODO: optionrc
                       QWidget* parent = nullptr);
    ~RouteSectionPlot() override;

    void setRoute(mccuav::Route* route);
    void setEmpty();

    void setRadarController(const bmcl::Option<std::shared_ptr<mccvis::RadarGroup>>& radarController);
    void setHeightmapController(bmcl::OptionPtr<const mccui::HeightmapController> hmController);

signals:
    void latLonChanged(const bmcl::Option<mccgeo::LatLon>& latLon);

private:
    void disconnectAll();
    void connectAll();
    void reconnectDevice();
    void drawDevice();
    void relativeProfileDistanses(const mccgeo::Position& p, const mccuav::Route& r, double* onRouteDist1, double* fromRouteDist1);
    void recalcData();
    void recalcDataOnlyAlt();
    void resetPlotData();
    void rescale();
    void clearTrack();
    void updateSelectedWaypoints();

    void drawRoute(const std::vector<std::vector<mccgeo::PositionAndDistance>>& routeProfiles);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    const mccuav::Route* _route;
    std::unique_ptr<QwtPlotCanvas> _canvas;
    std::unique_ptr<RouteCurve> _curve;
    std::unique_ptr<QwtPlotGrid> _grid;
    std::vector<double> _heights;
    std::vector<double> _distances;
    std::unique_ptr<QwtPlotCurve> _curve_srtm;
    std::unique_ptr<QwtPlotCurve> _track;
    std::vector<double> _srtmAlt;
    std::vector<double> _srtmDistance;

    struct VisCurve {
        VisCurve(std::vector<mccvis::Point>&& points, QColor&& color)
            : points(std::move(points))
            , color(std::move(color))
        {
        }

        std::vector<mccvis::Point> points;
        QColor color;
    };

    std::vector<VisCurve> _frenelVisZs;
    std::vector<mccvis::Point> _trackPoints;
    std::vector<std::unique_ptr<QwtPlotMarker>> _inclines;
    bmcl::Option<QPointF> _waypointHint;
    double _currentDistance;
    int _trackLen;
    bool _recalc;
    bmcl::Option<mccgeo::Position> _lastPosition;

    QwtPlotMarker* _currentPos;
    CanvasPicker* _picker;
    DeviceMarker* _deviceMarker;
    QwtPlotMarker* _hintMarker;

    bmcl::Option<std::shared_ptr<mccvis::RadarGroup>> _radarGroup;
    mccui::Rc<mccui::Settings> _settings;
    mccui::Rc<mccuav::RoutesController> _routesController;
    mccui::Rc<mccuav::UavController> _uavController;
    bmcl::OptionRc<const mccui::HeightmapController> _hmController;
    mccuav::Uav* _currentUav;
    mccgeo::Geod _geod;
    mccui::Rc<const mcchm::HmReader> _hmReader;

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;

};
