#include "mcc/ide/view/UdpConnectionWidget.h"
#include "ui_UdpConnectionWidget.h"

#include "mcc/ide/view/AbstractConnectionWidget.h"
#include "ui_UdpConnectionWidget.h"
#include <bmcl/MakeRc.h>
#include <QIntValidator>

namespace mccide {

UdpConnectionWidget::UdpConnectionWidget(mccmsg::NetTransport transport, QWidget* parent)
    : AbstractConnectionWidget(parent)
    , _transport(transport)
{
    _ui = new Ui::UdpConnectionWidget;
    _ui->setupUi(this);

    //_ui->hostText->setValidator(new QIntValidator(0, 65535));
    _sendValidator = new QIntValidator(1, 65535);
    _ui->sendPort->setValidator(_sendValidator);
    _listenValidator = new QIntValidator(1, 65535);
    _ui->listenPort->setValidator(_listenValidator);

    if (_transport == mccmsg::NetTransport::Tcp)
    {
        _ui->listenPort->setVisible(false);
        _ui->label_3->setVisible(false);
        _ui->autoListenPort->setVisible(false);
    }

    connect(_ui->autoListenPort, &QCheckBox::toggled, this,
        [this](bool state)
        {
            _ui->listenPort->setEnabled(!state);
            if (state && _ui->autoSendPort->isChecked())
                _ui->autoSendPort->setChecked(false);
        }
    );

    connect(_ui->autoSendPort, &QCheckBox::toggled, this,
        [this](bool state)
        {
            _ui->sendPort->setEnabled(!state);
            if (state && _ui->autoListenPort->isChecked())
                _ui->autoListenPort->setChecked(false);
        }
    );
}

UdpConnectionWidget::~UdpConnectionWidget()
{
    delete _sendValidator;
    delete _listenValidator;
    delete _ui;
}

mccmsg::NetUdpPtr UdpConnectionWidget::getUdpParams() const
{
    BMCL_ASSERT(!_ui->autoListenPort->isChecked() || !_ui->autoSendPort->isChecked());
    if (_ui->autoListenPort->isChecked())
        return bmcl::makeRc<const mccmsg::NetUdpParams>(_ui->host->text().toStdString(), _ui->sendPort->text().toUInt(), bmcl::None);
    else if (_ui->autoSendPort->isChecked())
        return bmcl::makeRc<const mccmsg::NetUdpParams>(_ui->host->text().toStdString(), bmcl::None, _ui->listenPort->text().toUInt());
    else
        return bmcl::makeRc<const mccmsg::NetUdpParams>(_ui->host->text().toStdString(), _ui->sendPort->text().toUInt(), _ui->listenPort->text().toUInt());
}

mccmsg::ChannelDescription UdpConnectionWidget::fillNetDescription(const mccmsg::Channel& channel, const mccmsg::Protocol& protocol, const QString& info, bool log, int timeout, bool isDynamicTimeout, bool isReadOnly, const bmcl::Option<std::chrono::seconds>& reconnect) const
{
    if (_transport == mccmsg::NetTransport::Tcp)
    {
        auto p = bmcl::makeRc<const mccmsg::NetTcpParams>(_ui->host->text().toStdString(), _ui->sendPort->text().toUInt());
        return bmcl::makeRc<const mccmsg::ChannelDescriptionObj>(channel, protocol, info.toStdString(), p, log, std::chrono::milliseconds(timeout), isDynamicTimeout, isReadOnly, reconnect, bmcl::None);
    }

    return bmcl::makeRc<const mccmsg::ChannelDescriptionObj>(channel, protocol, info.toStdString(), getUdpParams(), log, std::chrono::milliseconds(timeout), isDynamicTimeout, isReadOnly, reconnect, bmcl::None);
}

void UdpConnectionWidget::setSettings(const mccmsg::ChannelDescription& dscr)
{
    if (dscr->params()->transport() == mccmsg::NetTransport::Tcp )
    {
        auto p = bmcl::dynamic_pointer_cast<const mccmsg::NetTcpParams>(dscr->params());
        assert(!p.isNull());
        _ui->host->setText(QString::fromStdString(p->host()));
        _ui->sendPort->setText(QString::number(p->remotePort()));
        return;
    }

    if (dscr->params()->transport() == mccmsg::NetTransport::Udp)
    {
        auto p = bmcl::dynamic_pointer_cast<const mccmsg::NetUdpParams>(dscr->params());

        _ui->host->setText(QString::fromStdString(p->host()));
        _ui->sendPort->setText(QString::number(p->remotePort().unwrap()));
        if (p->localPort().isSome())
        {
            _ui->listenPort->setText(QString::number(p->localPort().unwrap()));
            _ui->label_3->setEnabled(true);
            _ui->listenPort->setEnabled(true);
            _ui->autoListenPort->setEnabled(true);
        }
        else
        {
            _ui->label_3->setEnabled(false);
            _ui->listenPort->setEnabled(false);
            _ui->autoListenPort->setEnabled(false);
        }
        return;
    }

    Q_ASSERT(false);
}
}
