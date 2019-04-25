#pragma once

#include <QWidget>
#include <QListView>

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"

#include "mcc/uav/ExchangeService.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"

class QLabel;
class DeviceListView;
class VideoRecordWidget;

class SessionTool : public QWidget
{
public:
    SessionTool(const mccui::Rc<mccui::Settings>& settings,
                const mccui::Rc<mccuav::ExchangeService>& exchangeService,
                const mccui::Rc<mccuav::UavController>& uavController,
                const mccui::Rc<mccuav::ChannelsController>& channelsController);
    ~SessionTool() override;

    void setSessionDescription(const bmcl::Option<mccmsg::TmSessionDescription>& session);
private:
    void addSeparator();
    VideoRecordWidget* _videoWidget;
    QLabel*         _devicesLogLabel;
    DeviceListView* _devicesList;

    bmcl::Option<mccmsg::TmSessionDescription> _session;
protected:
    virtual void showEvent(QShowEvent *event) override;

};

