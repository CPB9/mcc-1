#include "mcc/ide/dialogs/AddUavDialog.h"
#include "mcc/ide/view/SerialConnectionWidget.h"
#include "mcc/ide/view/UdpConnectionWidget.h"

#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ExchangeService.h"

#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/obj/Protocol.h"
#include "mcc/msg/ptr/Advanced.h"

#include "mcc/res/Resource.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QPainter>

#include <fmt/format.h>

Q_DECLARE_METATYPE(mccmsg::Device);
Q_DECLARE_METATYPE(mccmsg::Channel);
//Q_DECLARE_METATYPE(mccmsg::ProtocolDescription);

namespace mccide {

AddUavDialog::AddUavDialog(mccuav::ChannelsController* chanController,
                                   mccuav::RoutesController* routesController,
                                   mccuav::UavController* uavController,
                                   QWidget* parent)
    : mccui::Dialog(parent)
    , _protocolIcon(new QLabel(this))
    , _chanController(chanController)
    , _routesController(routesController)
    , _uavController(uavController)
{
    setWindowTitle("Добавить новый аппарат");
    QGridLayout* mainLayout = new QGridLayout();
    setLayout(mainLayout);

    _devicesList = new QComboBox(this);

    _deviceName = new QLineEdit(this);
    _deviceName->setMaxLength(64);

    _channelsList = new QComboBox(this);

    _protocolName = new QComboBox(this);
    _protocolId = new QLineEdit(this);
    _protocolIdValidator = new QIntValidator(0, std::numeric_limits<uint16_t>::max(), _protocolId);
    _protocolId->setValidator(_protocolIdValidator);

    _connectionType = new QComboBox(this);
    _channelName = new QLineEdit(this);
    _channelName->setMaxLength(64);

    {
        _devicesDetails = new QWidget(this);

        QGridLayout* layout = new QGridLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        _devicesDetails->setLayout(layout);

        layout->addWidget(new QLabel("Название аппарата:", this), 0, 0);
        layout->addWidget(_deviceName, 0, 1, 1, 3);

        _protocolIcon->setMinimumSize(_iconWidth, _iconWidth);
        QHBoxLayout* protocolLayout = new QHBoxLayout();
        protocolLayout->addWidget(new QLabel("Протокол:", this));
        protocolLayout->addStretch();
        protocolLayout->addWidget(_protocolIcon);
        layout->addLayout(protocolLayout, 1, 0);
        layout->addWidget(_protocolName, 1, 1);
        layout->addWidget(new QLabel("ID:", this), 1, 2);
        layout->addWidget(_protocolId, 1, 3);
    }

    int row = 0;
    mainLayout->addWidget(new QLabel("Аппарат:", this), row, 0);
    mainLayout->addWidget(_devicesList, row, 1, 1, 3);
    row++;

    mainLayout->addWidget(_devicesDetails, row++, 0, 1 ,4);

    // row 2
    mainLayout->addWidget(new QLabel("Канал обмена:", this), row, 0);
    mainLayout->addWidget(_channelsList, row, 1, 1, 3);
    row++;

    {
        //channels editor
        _channelsDetails = new QWidget(this);
        QGridLayout* channelDetailsLayout = new QGridLayout();
        channelDetailsLayout->setContentsMargins(0, 0, 0, 0);
        int editorRow = 0;
        channelDetailsLayout->addWidget(new QLabel("Название канала:", this), editorRow, 0);
        channelDetailsLayout->addWidget(_channelName, editorRow, 1, 1, 3);
        editorRow++;

        _channelsDetails->setLayout(channelDetailsLayout);
        // row 3
        channelDetailsLayout->addWidget(new QLabel("Тип соединения:", this), editorRow, 0);
        channelDetailsLayout->addWidget(_connectionType, editorRow, 1, 1, 3);
        editorRow++;

        // row 4
        QGroupBox* propertiesGroupBox = new QGroupBox("Параметры соединения", this);
        channelDetailsLayout->addWidget(propertiesGroupBox, editorRow, 0, 1, 4);
        QVBoxLayout* propertiesLayout = new QVBoxLayout();
        propertiesLayout->setContentsMargins(0, 0, 0, 0);
        propertiesGroupBox->setLayout(propertiesLayout);
        editorRow++;

        const QString udpName("Udp");
        _connectionProperties[udpName] = new UdpConnectionWidget(mccmsg::NetTransport::Udp);
        _connectionProperties["Tcp"] = new UdpConnectionWidget(mccmsg::NetTransport::Tcp);
        _connectionProperties["Serial"] = new SerialConnectionWidget();
        for (auto w : _connectionProperties)
        {
            _connectionType->addItem(w.first);
            propertiesLayout->addWidget(w.second);
        }
        setConnectionType(udpName);
        connect(_connectionType, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AddUavDialog::setConnectionType);

        _manualTimeout = new QSpinBox(this);
        _manualTimeout->setMinimum(1);
        _manualTimeout->setMaximum(100000);
        _manualTimeout->setSuffix(" мс");

        _automaticTimeout = new QCheckBox("Авто", this);

        // row 5
        channelDetailsLayout->addWidget(new QLabel("Таймаут обмена:", this), editorRow, 0);
        channelDetailsLayout->addWidget(_manualTimeout, editorRow, 1);
        channelDetailsLayout->addWidget(_automaticTimeout, editorRow, 2, 1, 2);
        editorRow++;

        _logExchange = new QCheckBox("Журналировать обмен", this);
        _readOnly = new QCheckBox("Только прослушивать", this);
        _readOnly->setToolTip("Работа канала в режиме «Только чтение»");

        // row 6, 7
        channelDetailsLayout->addWidget(_logExchange, editorRow, 0, 1, 4);
        editorRow++;
        channelDetailsLayout->addWidget(_readOnly, editorRow, 0, 1, 4);
    }

    mainLayout->addWidget(_channelsDetails, row++, 0, 1, 4);

    // row 7
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::StandardButton::Ok);
    buttons->buttons().back()->setIcon(mccres::loadIcon(mccres::ResourceKind::OkButtonIcon));
    buttons->addButton(QDialogButtonBox::StandardButton::Cancel);
    buttons->buttons().back()->setIcon(mccres::loadIcon(mccres::ResourceKind::CancelButtonIcon));

    mainLayout->setRowStretch(row++, 100);
    mainLayout->addWidget(buttons, row, 0, 1, 4);

    connect(buttons, &QDialogButtonBox::accepted, this, &AddUavDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &AddUavDialog::reject);

    connect(_chanController.get(), &mccuav::ChannelsController::protocolsChanged, this, &AddUavDialog::updateProtocols);
    //connect(_chanController.get(), &mccuav::ChannelsController::channelsChanged, this, &AddDeviceDialog::updateProtocols);
    updateProtocols();

    connect(_protocolName, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int index)
            {
                if(index < 0)
                    return;
                updateChannelsList();
                updateExpectedProtocolId();
                auto protocolDescription = _protocolName->currentData(Qt::UserRole).value<mccmsg::ProtocolDescription>();
                _manualTimeout->setValue(protocolDescription->timeout().count());
                _logExchange->setChecked(protocolDescription->logging());
                _readOnly->setChecked(false);
                _protocolIdValidator->setTop(static_cast<int>(protocolDescription->maxDeviceId()));
                if(_protocolIcons.find(protocolDescription->name()) != _protocolIcons.end())
                    _protocolIcon->setPixmap(_protocolIcons.at(protocolDescription->name()));
            }
    );

    connect(_channelsList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                updateChannelsDetails();
            }
    );

    connect(_protocolId, &QLineEdit::textChanged, this, &AddUavDialog::updateChannelsDetails);

    connect(_devicesList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                if (_devicesList->count() > 0)
                    updateDevicesDetails();
            }
    );

    connect(this, &AddUavDialog::accepted, this, &AddUavDialog::addDevice);
}

AddUavDialog::~AddUavDialog() {}


void AddUavDialog::setFixedChannel(const mccmsg::Channel& channel)
{
    auto index = _channelsList->findData(QVariant::fromValue(channel), Qt::UserRole);
    if (index == -1)
    {
        assert(false);
        return;
    }
    _channelsList->setCurrentIndex(index);
    _channelsList->setDisabled(true);
}

void AddUavDialog::setConnectionType(const QString& name)
{
    for (const auto& kv : _connectionProperties)
    {
        kv.second->setVisible(false);
    }

    auto it = _connectionProperties.find(name);
    if (it == _connectionProperties.end())
    {
        Q_ASSERT(false);
        return;
    }

    _connectionType->setCurrentIndex(_connectionType->findText(name));
    it->second->setVisible(true);
}

void AddUavDialog::updateProtocols()
{
    if (_chanController.isNull())
        return;

    _protocolName->clear();
    _protocolIcons.clear();
    for (const auto& it : _chanController->protocols())
    {
        updateProtocolIcon(it);
        _protocolName->addItem(QString::fromStdString(it->info()), QVariant::fromValue(it));
    }

    updateExpectedProtocolId();
}

void AddUavDialog::addDevice()
{
    _routesController->stopEditRoute();

    auto protocol = _protocolName->currentData().value<mccmsg::ProtocolDescription>()->name();
    auto connectionsPropertiesWidget = _connectionProperties.find(_connectionType->currentText());
    if (connectionsPropertiesWidget == _connectionProperties.end())
    {
        Q_ASSERT(false);
        return;
    }

    QString channelInfo(_channelName->text().simplified());
    if (channelInfo.isEmpty())
        channelInfo = _channelName->placeholderText();

    auto channel = _channelsList->currentData().value<mccmsg::Channel>();

    mccmsg::ChannelDescription channel_params = connectionsPropertiesWidget->second->fillNetDescription(channel,
                                                                                                        protocol,
                                                                                                        channelInfo,
                                                                                                        _logExchange->isChecked(),
                                                                                                        _manualTimeout->value(),
                                                                                                        _automaticTimeout->isChecked(),
                                                                                                        _readOnly->isChecked(),
                                                                                                        bmcl::None);

    QString deviceName = _deviceName->text().simplified();
    if (deviceName.isEmpty())
        deviceName = _deviceName->placeholderText();

    mccmsg::Device dev;
    if (_devicesList->currentIndex() > 0)
    {
        dev = _devicesList->currentData().value<mccmsg::DeviceDescription>()->name();
    }

    bmcl::SharedBytes pixmap = bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceUnknownIcon));
    mccmsg::DeviceDescription device_params = new mccmsg::DeviceDescriptionObj(dev, deviceName.toStdString(), "", mccmsg::ProtocolId{ dev, protocol, _protocolId->text().toUInt() }, bmcl::None, pixmap, bmcl::None, false, false);
    _uavController->requestUavAndChannelRegister(device_params, channel_params, this);
}

void AddUavDialog::setVisible(bool visible)
{
    if (visible)
    {
        updateDevicesList();
        updateChannelsList();
        updateChannelsDetails();
        updateExpectedProtocolId();
    }

    mccui::Dialog::setVisible(visible);
}

void AddUavDialog::updateDevicesList()
{
    auto service = _uavController->exchangeService();

    service->requestXX(new mccmsg::device::DescriptionList_Request()).then
    (
        [this](const mccmsg::device::DescriptionList_ResponsePtr& rep)
        {
            _devicesList->clear();
            _devicesList->addItem("<Добавить новый аппарат>");

            for (const auto& desc : rep->data())
            {
                mccmsg::ProtocolDescription protocolDesc;
                for (auto i = 0; i < _protocolName->count(); i++)
                {
                    protocolDesc = _protocolName->itemData(i).value<mccmsg::ProtocolDescription>();
                    if (protocolDesc->name() == desc->protocolId().protocol())
                    {
                        QString info = QString::fromStdString(desc->info());

                        QString deviceText = QString("%1 (%2, %3)")
                            .arg(info)
                            .arg(QString::fromStdString(protocolDesc->info()))
                            .arg(desc->protocolId().id());

                        _devicesList->addItem(deviceText, QVariant::fromValue(desc));
                    }
                }
            }
        }
    );
}

void AddUavDialog::updateChannelsList()
{
    auto currentProtocol = _protocolName->currentData().value<mccmsg::ProtocolDescription>()->name();
    auto lastSelectedChannel = _channelsList->currentData().value<mccmsg::Channel>();
    _channelsList->clear();
    _channelsList->addItem("<Добавить канал обмена>", QVariant::fromValue(mccmsg::Channel()));
    for (const auto& channel : _chanController->channelInformations())
    {
        if (channel.channelDescription().isNone() || channel.channelDescription()->protocol() != currentProtocol)
            continue;

        QString name = QString::fromStdString(fmt::format("{} {}", channel.channelDescription()->info(), channel.channelDescription()->params()->printableName()));
        _channelsList->addItem(name, QVariant::fromValue(channel.channel()));
    }

    int newSelectedIndex = _channelsList->findData(QVariant::fromValue(lastSelectedChannel));
    if (newSelectedIndex != -1)
        _channelsList->setCurrentIndex(newSelectedIndex);
}

void AddUavDialog::updateChannelsDetails()
{
    bool isVisible = (_channelsList->currentIndex() == 0);
    _channelsDetails->setVisible(isVisible);
    auto currentProtocol = _protocolName->currentData().value<mccmsg::ProtocolDescription>();
    auto currentId = _protocolId->text();
    QString defaultDeviceName = QString("%1 %2").arg(QString::fromStdString(currentProtocol->info())).arg(currentId);
    if (defaultDeviceName.size() > 1) // 2 and more symbols
    {
        defaultDeviceName[0] = defaultDeviceName.at(0).toUpper();
    }
    _deviceName->setPlaceholderText(defaultDeviceName);
    if (isVisible)
    {
        QString defaultChannelName = QString("%1%2").arg(QString::fromStdString(currentProtocol->info())).arg(currentId);
        _channelName->setPlaceholderText(defaultChannelName);
    }
    adjustSize();
}

void AddUavDialog::updateDevicesDetails()
{
    bool isVisible = (_devicesList->currentIndex() == 0);
    _devicesDetails->setVisible(isVisible);
    if (isVisible)
    {
        _channelName->clear();
        _deviceName->clear();

        updateChannelsList();
        updateChannelsDetails();
        updateExpectedProtocolId();
    }
    else
    {
        auto selectedDevice = _devicesList->currentData().value<mccmsg::DeviceDescription>();

        int protocolIndex = -1;
        for (auto i = 0; i < _protocolName->count(); i++)
        {
            if (_protocolName->itemData(i).value<mccmsg::ProtocolDescription>()->name() == selectedDevice->protocolId().protocol())
            {
                protocolIndex = i;
                break;
            }
        }
        if (protocolIndex == -1)
        {
            assert(false);
            return;
        }

        _protocolName->setCurrentIndex(protocolIndex);
        _protocolId->setText(QString::number(selectedDevice->protocolId().id()));

        updateChannelsList();
        updateChannelsDetails();
    }
    adjustSize();
}

size_t AddUavDialog::findProtocolId() const
{
    if (_protocolName->count() == 0)
        return -1;
    auto currentProtocol = _protocolName->currentData().value<mccmsg::ProtocolDescription>()->name();
    std::set<size_t> usedIds;
    for (auto dev : _uavController->uavsList())
    {
        if (dev->protocol() != currentProtocol)
            continue;
        usedIds.insert(dev->deviceId());
    }

    size_t expectedId = 1;
    for (auto current = usedIds.begin(), end = usedIds.end(); current != end && *current == expectedId; ++current) {
        ++expectedId;
    }
    return expectedId;
}

void AddUavDialog::updateExpectedProtocolId()
{
    _protocolId->setText(QString::number(findProtocolId()));
}

void AddUavDialog::updateProtocolIcon(const mccmsg::ProtocolDescription& protocolDesc)
{
    if(protocolDesc->pixmap().isEmpty())
        return;
    _protocolIcons[protocolDesc->name()] = QPixmap::fromImage(mccres::renderSvg(protocolDesc->pixmap().asBytes(), _iconWidth, _iconWidth));
}
}
