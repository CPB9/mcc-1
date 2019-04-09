#include "mcc/ide/view/NetStatisticsWidget.h"

#include <QPainter>

#include <memory>
#include <array>
#include <cmath>
#include <QDebug>

#include <bmcl/TimeUtils.h>

#include "mcc/msg/Stats.h"

namespace mccide {

constexpr size_t CURVE_COUNT = 6;
constexpr size_t REFRESH_RATE = 10;           // 1/10 s chart update
constexpr size_t HISTORY = 10 * REFRESH_RATE; // curves history
constexpr size_t WATCH_DOG = 2;               // 2 s watchdog

class NetCurve
{
public:
    enum Kind
    {
        SentBytes,
        SentPackets,
        ReceivedBytes,
        ReceivedPackets,
        ReceivedBadBytes,
        ReceivedBadPackets,
        Count
    };

    NetCurve()
        : _offset(0)
        , _lastValue(0.0)
        , _speed(0.0)
        , _max(0.0)
        , _isVisible(true)
    {
        clearData();
    }

    virtual ~NetCurve() //WTF??
    {
    }

    void setStyle(const QPen& pen, const QBrush& brush)
    {
        _pen = pen;
        _brush = brush;
        _pen.setCosmetic(true);
    }

    void update(const bmcl::SystemTime& time, const bmcl::SystemTime& now, double value)
    {
        if (_offset == HISTORY - 1) {
            _offset = 0;
        } else {
            _offset++;
        }
        constexpr qint64 watchdog = WATCH_DOG * 1000;

        auto valuesDelta = bmcl::toMsecs(time - _lastTime).count();
        auto nowDelta    = bmcl::toMsecs(now - time).count();

        if (nowDelta > watchdog)
        {
            _data[_offset] = 0.0;
            _lastTime = time;
            _lastValue = value;
            _speed = 0.0;
            _max = std::ceil(*std::max_element(_data.begin(), _data.end()));
            return;
        }

        if (valuesDelta > 0.0)
        {
            if(_lastValue != 0.0)
                _speed = 1000.0 * (value - _lastValue) / valuesDelta;
            else
                _speed = 0.0;
        }

        _data[_offset] = _speed;
        _lastTime = time;
        _lastValue = value;
        _max = std::ceil(*std::max_element(_data.begin(), _data.end()));
    }

    void setVisible(bool visible)
    {
        _isVisible = visible;
    }

    double speed() const
    {
        return _speed;
    }

    unsigned int maxElement() const
    {
        return _max;
    }

    bool isVisible() const
    {
        return _isVisible;
    }

    void clearData(bmcl::SystemTime time = bmcl::SystemClock::now())
    {
        _data.fill(0.0);
        _offset = 0;
        _lastTime = time;
        _lastValue = 0.0;
        _speed = 0.0;
        _max = 0.0;
    }

    void draw(QPainter* p) const
    {
        if (!_isVisible) {
            return;
        }
        p->setPen(_pen);
        p->setBrush(_brush);

        std::size_t j = 0;
        auto drawLine = [&, this](std::size_t i) {
            p->drawLine(j, _data[i], (j + 1) % HISTORY, _data[(i + 1) % HISTORY]);
            j++;
        };

        for (std::size_t i = _offset + 1; i < (HISTORY - 1); i++) {
            drawLine(i);
        }
        for (std::size_t i = 0; i < _offset; i++) {
            drawLine(i);
        }
    }

private:
    QPen                         _pen;
    QBrush                       _brush;
    std::array<double, HISTORY>  _data;
    std::size_t                  _offset;
    bmcl::SystemTime             _lastTime;
    double                       _lastValue;
    double                       _speed;
    double                       _max;
    bool                         _isVisible;
};

NetStatisticsWidget::NetStatisticsWidget(QWidget* parent)
    : QWidget(parent)
    , _maxValue(0.0)
    , _drawDetails(true)
{
    _curves = new NetCurve[CURVE_COUNT];

    _curves[NetCurve::SentBytes].setStyle(QPen(QColor("#e68a5c"), 1.0, Qt::DashLine), QColor(255, 255, 255, 0));
    _curves[NetCurve::SentPackets].setStyle(QPen(QColor("#e68a5c"), 1.0, Qt::DashLine), QColor(255, 255, 255, 0));

    _curves[NetCurve::ReceivedBytes].setStyle(QPen(QColor("#55aa00"), 2.0), QColor(255, 255, 255, 0));
    _curves[NetCurve::ReceivedPackets].setStyle(QPen(QColor("#55aa00"), 2.0), QColor(255, 255, 255, 0));

    _curves[NetCurve::ReceivedBadBytes].setStyle(QPen(Qt::red, 1.0, Qt::DashDotDotLine), QColor(255, 255, 255, 0));
    _curves[NetCurve::ReceivedBadPackets].setStyle(QPen(Qt::red, 1.0, Qt::DashDotDotLine), QColor(255, 255, 255, 0));

    _detailsFont.setPointSize(8);
    _detailsFont.setBold(true);
    _detailsPen.setColor(Qt::white);

    _timerId = startTimer(1000 / REFRESH_RATE);

    setStatView(StatView::Packets);

    if (_statView == StatView::Bytes) {
        setStatView(StatView::Packets);
        this->setToolTip("Скорость обмена (пакетов в сек.)");
    }
    else
    {
        this->setToolTip("Скорость обмена ([...]байтов в сек.)");
    }
}

NetStatisticsWidget::~NetStatisticsWidget()
{
    killTimer(_timerId);
    delete [] _curves;
}

void NetStatisticsWidget::updateStats(mccmsg::Stat sent, mccmsg::Stat rcvd, mccmsg::Stat bad)
{
    _sentStat = sent;
    _rcvdStat = rcvd;
    _badStat = bad;
}

void NetStatisticsWidget::clear()
{
    _maxValue = 0.0;

    _sentStat = mccmsg::Stat();
    _rcvdStat = _sentStat;
    _badStat  = _sentStat;

    bmcl::SystemTime now = bmcl::SystemClock::now();
    _curves[NetCurve::SentBytes         ].clearData(now);
    _curves[NetCurve::ReceivedBytes     ].clearData(now);
    _curves[NetCurve::ReceivedBadBytes  ].clearData(now);

    _curves[NetCurve::SentPackets       ].clearData(now);
    _curves[NetCurve::ReceivedPackets   ].clearData(now);
    _curves[NetCurve::ReceivedBadPackets].clearData(now);
}

void NetStatisticsWidget::setTextColor(const QColor& textColor)
{
    _detailsPen.setColor(textColor);
    update();
}

void NetStatisticsWidget::setTextFont(const QFont& textFont)
{
    _detailsFont = textFont;
    update();
}

double NetStatisticsWidget::sentBytesSpeed()          const { return _curves[NetCurve::SentBytes].speed();         }
double NetStatisticsWidget::sentPacketsSpeed()        const { return _curves[NetCurve::SentPackets].speed();       }
double NetStatisticsWidget::receivedBytesSpeed()      const { return _curves[NetCurve::ReceivedBytes].speed();     }
double NetStatisticsWidget::receivedPacketsSpeed()    const { return _curves[NetCurve::ReceivedPackets].speed();   }
double NetStatisticsWidget::receivedBadBytesSpeed()   const { return _curves[NetCurve::ReceivedBadBytes].speed();  }
double NetStatisticsWidget::receivedBadPacketsSpeed() const { return _curves[NetCurve::ReceivedBadPackets].speed();}

QString NetStatisticsWidget::formatSpeed(double speed)
{
    if (speed > 1024 * 1024)
        return QString("%1 Мб").arg(speed / 1024 * 1024, 1, 'f', 0);
    else if (speed > 1024)
        return QString("%1 Кб").arg(speed / 1024, 1, 'f', 0);

    return QString("%1 б").arg(speed, 1, 'f', 0);
}

QString NetStatisticsWidget::formatPackets(double speed)
{
    return QString("%1 п").arg(speed, 1, 'f', 0);
}

void NetStatisticsWidget::showDetails(bool showDetails)
{
    _drawDetails = showDetails;
}

void NetStatisticsWidget::timerEvent(QTimerEvent*)
{
    auto now = bmcl::SystemClock::now();
    _curves[NetCurve::SentBytes         ].update(_sentStat._time, now, _sentStat._bytes);
    _curves[NetCurve::ReceivedBytes     ].update(_rcvdStat._time, now, _rcvdStat._bytes);
    _curves[NetCurve::ReceivedBadBytes  ].update(_badStat._time, now,  _badStat._bytes);

    _curves[NetCurve::SentPackets       ].update(_sentStat._time, now, _sentStat._packets);
    _curves[NetCurve::ReceivedPackets   ].update(_rcvdStat._time, now, _rcvdStat._packets);
    _curves[NetCurve::ReceivedBadPackets].update(_badStat._time, now,  _badStat._packets);

    rescalePlot();
    update();
}

void NetStatisticsWidget::resizeEvent(QResizeEvent* event)
{
    rescalePlot();
    update();
}

void NetStatisticsWidget::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.save();
    p.setTransform(_transform);
    for (const NetCurve* curve = &_curves[0]; curve < &_curves[CURVE_COUNT]; curve++) {
        curve->draw(&p);
    }
    p.restore();
    double sentSpeed = _curves[NetCurve::SentBytes].speed();
    double recvSpeed = _curves[NetCurve::ReceivedBytes].speed();

    QString detailsText;

    if (_statView == StatView::Bytes)
    {
        detailsText = QString("%1\n%2").arg(NetStatisticsWidget::formatSpeed(sentSpeed)).arg(NetStatisticsWidget::formatSpeed(recvSpeed));
    }
    else if (_statView == StatView::Packets)
    {
        detailsText = QString("%1\n%2").arg(NetStatisticsWidget::formatPackets(sentPacketsSpeed())).arg(NetStatisticsWidget::formatPackets(receivedPacketsSpeed()));
    }
    p.setPen(_detailsPen);
    p.setFont(_detailsFont);
    p.drawText(rect() - QMargins(2, 0, 0, 0), Qt::AlignTop| Qt::AlignLeft, detailsText);
}

void NetStatisticsWidget::rescalePlot()
{
    unsigned int maxValue = 0;

    for (const NetCurve* curve = &_curves[0]; curve < &_curves[CURVE_COUNT]; curve++)
    {
        if (!curve->isVisible())
            continue;

        auto maxElement = curve->maxElement();
        if (maxElement > maxValue)
            maxValue = maxElement;
    }

    _maxValue = maxValue * 1.1;

    int margin = 1;
    _transform.reset();
    _transform.translate(margin, height() - margin);
    double xscale = double(width() - 2 * margin) / HISTORY;
    double yscale = double(height() - 2 * margin) / _maxValue;
    _transform.scale(xscale, -yscale);
}

void NetStatisticsWidget::mousePressEvent(QMouseEvent*)
{
    if (_statView == StatView::Bytes) {
        setStatView(StatView::Packets);
        this->setToolTip("Скорость обмена (пакетов в сек.)");
    }
    else
    {
        setStatView(StatView::Bytes);
        this->setToolTip("Скорость обмена ([...]байтов в сек.)");
    }
}

void NetStatisticsWidget::setStatView(StatView view)
{
    if (view == StatView::Bytes)
    {
        _curves[NetCurve::SentBytes].setVisible(true);
        _curves[NetCurve::ReceivedBytes].setVisible(true);
        _curves[NetCurve::ReceivedBadBytes].setVisible(true);

        _curves[NetCurve::SentPackets].setVisible(false);
        _curves[NetCurve::ReceivedPackets].setVisible(false);
        _curves[NetCurve::ReceivedBadPackets].setVisible(false);
    }
    else if (view == StatView::Packets)
    {
        _curves[NetCurve::SentBytes].setVisible(false);
        _curves[NetCurve::ReceivedBytes].setVisible(false);
        _curves[NetCurve::ReceivedBadBytes].setVisible(false);

        _curves[NetCurve::SentPackets].setVisible(true);
        _curves[NetCurve::ReceivedPackets].setVisible(true);
        _curves[NetCurve::ReceivedBadPackets].setVisible(true);
    }

    _statView = view;
}

}
