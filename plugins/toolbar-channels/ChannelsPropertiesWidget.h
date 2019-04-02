#pragma once

#include "mcc/msg/Objects.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"
#include "mcc/uav/Fwd.h"

#include <QWidget>

class QLabel;

namespace mccide{
class NetStatisticsWidget;
}
namespace mccmsg{
class StatChannel;
}

class ChannelsPropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelsPropertiesWidget(mccuav::ChannelsController* channelsController,
                                      const mccmsg::Channel& channel,
                                      QWidget *parent = nullptr);
    ~ChannelsPropertiesWidget() override;

    const mccmsg::Channel& channel() const {return _channel;}

public slots:
    void setChannel(const mccmsg::Channel& channel);

    void updateName();
    void updateActivation();
    void setProtocolIcon(const QPixmap& pixmap);
    void setStatistics(const mccmsg::StatChannel& stat);

signals:
    void channelMenuClicked(const mccmsg::Channel& channel);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const QSize                             _iconsSize;

    mccui::Rc<mccuav::ChannelsController>   _channelsController;
    mccmsg::Channel                         _channel;

    QLabel*                                 _protocolIcon;
    QLabel*                                 _name;
    mccide::NetStatisticsWidget*            _statistics;
    mccui::ClickableLabel*                  _menu;
    mccui::OnOffSliderCheckBox*             _slider;
};
