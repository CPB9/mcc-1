#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Profile.h"
#include "mcc/vis/Ticks.h"

#include <bmcl/Fwd.h>

#include <QWidget>
#include <QPen>

class QCheckBox;

namespace mccvis {

class Profile;

class MCC_VIS_DECLSPEC ProfileViewer : public QWidget {
    Q_OBJECT
public:
    struct RenderConfig {
        RenderConfig()
            : drawBackground(true)
            , drawGround(true)
            , drawViewArea(true)
        {
        }

        bool drawBackground;
        bool drawGround;
        bool drawViewArea;
    };

    struct Data {
        Data(const Profile* prof)
            : profile(prof)
        {
        }

        Data(const Rc<const Profile>& prof)
            : profile(prof)
        {
        }

        Rc<const Profile> profile;
        QPainterPath earthFillPath;
        QPainterPath earthStrokePath;
        QPainterPath targetStrokePath;
        QPainterPath viewRegionPath;
        Ticks yticks;
        double ymin;
        double ymax;
        double deltay;
    };

    ProfileViewer(QWidget* parent = nullptr);
    ~ProfileViewer();

    void setProfile(bmcl::OptionPtr<const Profile> profile);
    void setProfiles(bmcl::ArrayView<Rc<const Profile>> profiles);
    void renderPlot(QPaintDevice* paintDevice, const RenderConfig& cfg);
    void setTitle(const QString& title);

    const RenderConfig& renderConfig() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void onProfileReset();
    void drawTriangle(QPainter* p, const QPointF& pos, int size, bool isUp);
    void drawStar(QPainter* p, const QPointF& pos, int radius);

    QColor _blue;
    QColor _black;
    QColor _red;
    QColor _grey;
    QColor _green;
    QColor _viewBlue;
    QColor _fillBlue;
    QColor _fillGreen;
    QColor _fillRed;

    QPen _earthPen;
    QPen _borderPen;
    QPen _rayVisibleLimitsPen;
    QPen _rayInvisibleLimitsPen;
    QPen _raysPen;
    QPen _targetPen;
    QPen _gridPen;


    Ticks _xticks;

    QString _title;
    QString _positionText;

    std::vector<Data> _data;

    double _xmin;
    double _xmax;
    double _deltax;

    double _totalDeltay;

    QCheckBox* _backgroundCheckbox;
    QCheckBox* _groundCheckbox;
    QCheckBox* _viewAreaCheckbox;

    RenderConfig _renderConfig;
};

}
