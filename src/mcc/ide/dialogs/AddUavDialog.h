#pragma once
#include "mcc/msg/Objects.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Dialog.h"

class QCheckBox;
class QComboBox;
class QIntValidator;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace mccide {

class AbstractConnectionWidget;

class MCC_IDE_DECLSPEC AddUavDialog : public mccui::Dialog
{
    Q_OBJECT
public:
    AddUavDialog(mccuav::ChannelsController* chanController,
                 mccuav::RoutesController* routesController,
                 mccuav::UavController* uavController,
                 QWidget* parent = nullptr);
    ~AddUavDialog() override;

    void setFixedChannel(const mccmsg::Channel& channel);
private slots:
    void setConnectionType(const QString& name);
    void updateProtocols();
    void addDevice();
    void setVisible(bool visible) override;
    void updateChannelsDetails();
    void updateDevicesDetails();

private:
    void updateDevicesList();
    void updateChannelsList();
    void updateExpectedProtocolId();
    void updateProtocolIcon(const mccmsg::ProtocolDescription& protocolDesc);
    size_t findProtocolId() const;

private:
    QComboBox*      _devicesList;
    QWidget*        _devicesDetails;
    QLineEdit*      _deviceName;
    QLabel*         _protocolIcon;
    QComboBox*      _protocolName;
    QLineEdit*      _protocolId;
    QIntValidator*  _protocolIdValidator;
    QLineEdit*      _channelName;

    QComboBox*      _channelsList;

    QWidget*        _channelsDetails;
    QComboBox*      _connectionType;
    std::map<QString, AbstractConnectionWidget*> _connectionProperties;

    QCheckBox*      _logExchange;
    QSpinBox*       _manualTimeout;
    QCheckBox*      _automaticTimeout;
    QCheckBox*      _readOnly;

    std::map<mccmsg::Protocol, QPixmap>     _protocolIcons;

    mccuav::Rc<mccuav::ChannelsController>  _chanController;
    mccuav::Rc<mccuav::RoutesController>    _routesController;
    mccuav::Rc<mccuav::UavController>       _uavController;

    static constexpr int                    _iconWidth = 32;

    Q_DISABLE_COPY(AddUavDialog)
};
}
