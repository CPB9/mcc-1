#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"
#include "mcc/vis/Region.h"
#include "mcc/vis/Ticks.h"

#include <bmcl/Option.h>

#include <QWidget>
#include <QPen>

class QCheckBox;

namespace mccvis {

class Region;

class MCC_VIS_DECLSPEC RegionViewer : public QWidget {
    Q_OBJECT
public:
    enum Mode {
        RegionMode,
        AnglesMode,
    };
    struct RenderConfig {
        RenderConfig()
            : drawBackground(true)
        {
        }

        bool drawBackground;
    };

    RegionViewer(QWidget* parent = nullptr);
    ~RegionViewer();

    void setRegion(const Region* region);
    void setSelectedProfile(bmcl::Option<std::size_t> idx);
    void renderPlot(QPaintDevice* paintDevice, const RenderConfig& cfg);

    const RenderConfig& renderConfig() const;

    void setMode(Mode mode);

signals:
    void profileClicked(std::size_t index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    bool isRegionView() const;

    bmcl::Option<std::size_t> findProf(const QPointF& mousePos);
    QTransform calcTransform(const QPaintDevice* dev) const;
    void paintMain(QPainter* p, const RenderConfig& cfg);
    void paintSelection(QPainter* p);

    void updatePosLabel(const QPoint& mousePos);

    Rc<const Region> _region;
    std::vector<QPainterPath> _paths;
    std::vector<QPainterPath> _hitPaths;
    std::vector<QPointF> _anglePath;
    double _minViewAngle;
    double _maxViewAngle;
    double _maxDistance;
    Ticks _angleTicks;
    QPoint _mousePos;
    QPoint _drawDelta;
    double _mouseScale;
    bool _isMousePressed;
    bmcl::Option<std::size_t> _selectedProfile;
    bmcl::Option<std::size_t> _hoverProfile;
    QString _positionText;
    QCheckBox* _backgroundCheckbox;
    RenderConfig _renderConfig;
    Mode _mode;
};
}

