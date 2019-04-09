#pragma once
#include <QWidget>
#include <QPen>

#include "mcc/Config.h"
#include "mcc/msg/Stats.h"
#include "mcc/Rc.h"

namespace mccide {

class NetCurve;

class MCC_IDE_DECLSPEC NetStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    enum class StatView
    {
        Bytes,
        Packets
    };

    NetStatisticsWidget(QWidget* parent = nullptr);
    ~NetStatisticsWidget() override;

    void updateStats(mccmsg::Stat sent, mccmsg::Stat rcvd, mccmsg::Stat bad);
    void clear();
    void setTextColor(const QColor& textColor);
    void setTextFont(const QFont& textFont);

    double sentBytesSpeed()          const;
    double sentPacketsSpeed()        const;
    double receivedBytesSpeed()      const;
    double receivedPacketsSpeed()    const;
    double receivedBadBytesSpeed()   const;
    double receivedBadPacketsSpeed() const;

    inline StatView statView() const { return _statView; }

    static QString formatSpeed(double speed);
    static QString formatPackets(double speed);
    void showDetails(bool showDetails);

    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    const QFont& textFont() const {return _detailsFont;}

    QWidget* canvas()
    {
        return this;
    }

private:
    void timerEvent(QTimerEvent*) override;

    void rescalePlot();

    void mousePressEvent(QMouseEvent*) override;

    void setStatView(StatView view);
private:
    int                                _timerId;
    double                             _maxValue;
    StatView                           _statView;
    mccmsg::Stat                       _sentStat;
    mccmsg::Stat                       _rcvdStat;
    mccmsg::Stat                       _badStat;

    NetCurve*                          _curves;
    QTransform                         _transform;
    bool                               _drawDetails;
    QFont                              _detailsFont;
    QPen                               _detailsPen;

    Q_DISABLE_COPY(NetStatisticsWidget)
};
}
