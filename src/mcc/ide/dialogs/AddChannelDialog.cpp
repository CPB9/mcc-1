#include "mcc/ide/dialogs/AddChannelDialog.h"

#include "mcc/ide/view/SerialConnectionWidget.h"
#include "mcc/ide/view/UdpConnectionWidget.h"
#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/obj/Protocol.h"
#include "mcc/uav/ChannelsController.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

Q_DECLARE_METATYPE(mccmsg::Protocol);
Q_DECLARE_METATYPE(mccmsg::ProtocolDescription);

namespace mccide {

AddChannelDialog::AddChannelDialog(mccuav::ChannelsController* channelsController, QWidget* parent)
    : mccui::Dialog(parent)
    , _channelsController(channelsController)
{
    setModal(true);
    setWindowTitle("Добавить канал обмена");

    _channelName    = new QLineEdit();
    _channelName->setMaxLength(64);
    _channelType    = new QComboBox();
    _protocol       = new QComboBox();
    _logExchange    = new QCheckBox("Журналировать обмен");
    _timeout        = new QSpinBox();
    _dynamicTimeout = new QCheckBox("Динамическая подстройка");
    _readOnly       = new QCheckBox("Только прослушивать");
    _readOnly->setToolTip("Работа канала в режиме «Только чтение»");

    auto topLayout = new QGridLayout();
    topLayout->addWidget(new QLabel("Название:"), 0, 0);
    topLayout->addWidget(_channelName,  0, 1, 1, 2);
    topLayout->addWidget(new QLabel("Тип:"), 1, 0);
    topLayout->addWidget(_channelType,  1, 1, 1, 2);
    topLayout->addWidget(new QLabel("Протокол:"), 2, 0);
    topLayout->addWidget(_protocol,     2, 1, 1, 2);
    topLayout->addWidget(_logExchange,  3, 0, 1, 3);
    topLayout->addWidget(_readOnly,     4, 0, 1, 3);
    topLayout->addWidget(new QLabel("Таймаут обмена (мс):"), 5, 0);
    topLayout->addWidget(_timeout, 5, 1);
    topLayout->addWidget(_dynamicTimeout, 5, 2);
    topLayout->setColumnStretch(2, 1);

    topLayout->setContentsMargins(0, 0, 0, 0);

    _widgets["Udp"] = new UdpConnectionWidget(mccmsg::NetTransport::Udp);
    _widgets["Tcp"] = new UdpConnectionWidget(mccmsg::NetTransport::Tcp);
    _widgets["Serial"] = new SerialConnectionWidget();

    _channelType->addItems(QStringList() << "Udp" << "Tcp" << "Serial");

    auto channelsLayout = new QVBoxLayout();
    for (auto w : _widgets)
    {
        channelsLayout->addWidget(w.second);
    }
    channelsLayout->setContentsMargins(0, 0, 0, 0);

    auto groupBox = new QGroupBox("Параметры");
    groupBox->setLayout(channelsLayout);

    auto addButton = new QPushButton("Ок");
    auto closeButton = new QPushButton("Отмена");

    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(closeButton);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    auto layout = new QVBoxLayout();
    layout->addLayout(topLayout);
    layout->addWidget(groupBox);
    layout->addLayout(buttonsLayout);

    layout->setContentsMargins(3, 3, 3, 3);

    setLayout(layout);

    _timeout->setMinimum(1);
    _timeout->setMaximum(100000);
    _timeout->setValue(1000);

    connect(_channelType, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AddChannelDialog::setConnectionWidget);
    setConnectionWidget("Udp");

    connect(addButton, &QPushButton::pressed, this, &AddChannelDialog::accept);
    connect(closeButton, &QPushButton::pressed, this, &AddChannelDialog::reject);
    connect(_protocol, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
        [this](int index)
    {
            if (index < 0)
                return;
        auto protocolDescription = _protocol->currentData(Qt::UserRole).value<mccmsg::ProtocolDescription>();
        _timeout->setValue(protocolDescription->timeout().count());
        _logExchange->setChecked(protocolDescription->logging());
        _readOnly->setChecked(false);
    });

    connect(_channelsController.get(), &mccuav::ChannelsController::protocolsChanged, this,
            [this]()
    {
        setProtocols(_channelsController->protocols());
    });
    connect(this, &AddChannelDialog::accepted, this,
            [this]()
    {
        if(_channel.isNone())
        {
            //        if (_channels.contains(QString::fromStdString(_channelDialog->netAddress().serialize())))
            //        {
            //            QMessageBox::warning(this, "Ошибка при добавлении канала связи", "Такой канал связи уже существует!");
            //            return;
            //        }
            //
            //        if (!_protocols.contains(_channelDialog->netAddress().protocol()))
            //        {
            //            QMessageBox::warning(this, "Ошибка при добавлении канала связи", "Неизвестный протокол обмена!");
            //            return;
            //        }
            _channelsController->requestChannelRegister(netAddress(), this);
        }
        else
        {
            _channelsController->requestChannelUpdate(netAddress(_channel.unwrap()), this);
        }
    });

    connect(_readOnly, &QCheckBox::pressed, this,
            [this]()
    {
        if(_channel.isNone())
            return;

        if(_readOnly->checkState() == Qt::Unchecked)
            return;

        if(QMessageBox::question(this, "Режим «Только чтение»", "Действительно выключить режим «Только чтение» для данного канала?") == QMessageBox::Yes)
        {
            _readOnly->blockSignals(true);
            _readOnly->setChecked(false);
            _readOnly->blockSignals(false);
        }
    });
}

AddChannelDialog::~AddChannelDialog()
{
}

void AddChannelDialog::setConnectionWidget(const QString& name)
{
    for (const auto& kv : _widgets)
    {
        kv.second->setVisible(false);
    }

    auto it = _widgets.find(name);
    if (it == _widgets.end())
    {
        Q_ASSERT(false);
        return;
    }

    _channelType->setCurrentIndex(_channelType->findText(name));

    it->second->setVisible(true);
}

void AddChannelDialog::show(const mccmsg::Channel& channel)
{
    if(channel.isNil())
    {
        if(_channel.isSome())
            _channel.clear();

        setDefaultName(QString("Канал %1").arg(_channelsController->channelsCount() + 1));
        _readOnly->setChecked(false);
    }
    else
    {
        _channel = channel;

        auto chInfo = _channelsController->channelInformation(channel);
        if (chInfo.isNone() || chInfo->channelDescription().isNone())
            return;

        auto& description = chInfo->channelDescription().unwrap();
        setChannelDescription(description);
    }

    _protocol->setEnabled(!_channel.isSome());
    _channelType->setEnabled(!_channel.isSome());

    if (_channel.isSome())
    {
        setWindowTitle("Редактировать канал обмена");
    }
    else
    {
        _channelName->setText(QString());
        _channelName->setPlaceholderText(_defaultName);
        setWindowTitle("Добавить канал обмена");
    }
    mccui::Dialog::show();
}

mccmsg::ChannelDescription AddChannelDialog::netAddress(const mccmsg::Channel& channel) const
{
    auto name = _channelType->currentText();
    auto it = _widgets.find(name);
    if (it == _widgets.end())
    {
        Q_ASSERT(false);
        return mccmsg::ChannelDescription();
    }

    QString chanName = _channelName->text().simplified();
    if(chanName.isEmpty())
        chanName = _channelName->placeholderText();

    return it->second->fillNetDescription(channel, protocol(), chanName, _logExchange->isChecked(), _timeout->value(), _dynamicTimeout->isChecked(), _readOnly->isChecked(), bmcl::None);
}

mccmsg::Protocol AddChannelDialog::protocol() const
{
    return _protocol->currentData().value<mccmsg::ProtocolDescription>()->name();
}

void AddChannelDialog::setProtocols(const mccmsg::ProtocolDescriptions& protocols)
{
    _protocol->clear();
    for (const auto& it : protocols)
    {
        _protocol->addItem(QString::fromStdString(it->info()), QVariant::fromValue(it));
    }
}

void AddChannelDialog::setChannelDescription(const mccmsg::ChannelDescription& description)
{
    QString name = QString::fromStdString(description->info());
    setChannelName(name);

    for (int i = 0; i < _protocol->count(); ++i)
    {
        auto desc = _protocol->itemData(i).value<mccmsg::ProtocolDescription>();
        if (desc->name() == description->protocol())
        {
            _protocol->setCurrentIndex(i);
            break;
        }
    }
    _logExchange->setChecked(description->log());
    _readOnly->setChecked(description->isReadOnly());
    _timeout->setValue(description->timeout().count());
    _dynamicTimeout->setChecked(description->isDynamicTimeout());

    switch (description->params()->transport())
    {
    case mccmsg::NetTransport::Tcp:
        setConnectionWidget("Tcp");
        _widgets["Tcp"]->setSettings(description);
        return;
    case mccmsg::NetTransport::Udp:
        setConnectionWidget("Udp");
        _widgets["Udp"]->setSettings(description);
        return;
    case mccmsg::NetTransport::Serial:
        setConnectionWidget("Serial");
        _widgets["Serial"]->setSettings(description);
        return;
    default:
        break;
    }
}

void AddChannelDialog::setDefaultName(const QString& name)
{
    _defaultName = name;
}

void AddChannelDialog::setChannelName(const QString& name)
{
    _channelName->setText(name);
    _channelName->setPlaceholderText(name);
}
}
