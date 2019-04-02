#pragma once

#include <cstddef>

#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Objects.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

class QMenu;
class QScrollArea;

class ChannelsPropertiesWidget;

namespace mccide {
class AddEntityWidget;
}

class ChannelsPropertiesDialog : public mccui::Dialog
{
    Q_OBJECT

public:
    explicit ChannelsPropertiesDialog(mccuav::ChannelsController* channelsController,
                                      mccuav::GlobalActions* actions,
                                      QWidget *parent = nullptr);
    ~ChannelsPropertiesDialog() override;

public slots:
    void updateChannels();
    void updateProtocolsIcons();
    void updateDescription(const mccmsg::Channel& channel, const mccmsg::ChannelDescription& description);
    void updateChannelStats(const mccmsg::Channel& channel, const mccmsg::StatChannel& stat);
    void updateChannelActivation(const mccmsg::Channel& channel, bool activated);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void updateChannelIcon(ChannelsPropertiesWidget* widget, const mccmsg::ChannelDescription& description);

    mccui::Rc<mccuav::ChannelsController>   _channelsController;
    mccui::Rc<mccuav::GlobalActions>        _actions;

    std::vector<ChannelsPropertiesWidget*>  _channelWidgets;
    std::map<mccmsg::Protocol, QPixmap>     _protocolIcons;

    mccide::AddEntityWidget*                _addChannelWidget;

    QScrollArea*                            _view;

    QMenu*                                  _channelMenu;
    mccmsg::Channel                         _menuChannel;

    Q_DISABLE_COPY(ChannelsPropertiesDialog)
};
