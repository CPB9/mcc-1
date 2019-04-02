#pragma once

#include "mcc/msg/Objects.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"

class QCheckBox;
class QLabel;

namespace mccuav
{
class Uav;
};

class RemoveUavDialog : public mccui::Dialog
{
    Q_OBJECT
public:
    explicit RemoveUavDialog(mccuav::UavController* uavController,
                             mccuav::ChannelsController* channelsController,
                             QWidget *parent = nullptr);
    ~RemoveUavDialog() override;

    void setUav(const mccmsg::Device& deviceId);
    const mccmsg::Device& device() const;

    bool isAllowToRemoveChannel() const;

protected:
    void showEvent(QShowEvent *event) override;

public slots:
    void accept() override;
    mccuav::Uav* uav() const;

private:
    void updateUavChannels();

    mccuav::Rc<mccuav::UavController>       _uavController;
    mccuav::Rc<mccuav::ChannelsController>  _channelsController;
    mccmsg::Device                          _deviceId;
    QLabel*                                 _question;
    QCheckBox*                              _channelsBox;

    bool                                    _allowRemoveChannel;

    Q_DISABLE_COPY(RemoveUavDialog)
};
