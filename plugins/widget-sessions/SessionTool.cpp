#include "SessionTool.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/ui/SliderCheckBox.h"
#include "mcc/msg/obj/TmSession.h"
#include "mcc/path/Paths.h"
#include <fmt/format.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QListView>

#include "VideoRecordWidget.h"
#include "DeviceListView.h"

SessionTool::SessionTool(const mccui::Rc<mccui::Settings>& settings,
                         const mccui::Rc<mccuav::ExchangeService>& exchangeService,
                         const mccui::Rc<mccuav::UavController>& uavController,
                         const mccui::Rc<mccuav::ChannelsController>& channelsController)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setStyleSheet(QString(
        "QWidget\n"
        "{\n"
        "	color: #909090;\n" // fafafa
        "	background-color: #%1;\n"
        "}\n\n"
    ).arg(mccide::MainToolBar::mainBackgroundColor().rgb(), 6, 16, QLatin1Char('0')));

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(3, 3, 3, 3);
    layout->setSpacing(3);
    setLayout(layout);
    _videoWidget = new VideoRecordWidget(uavController, settings, this);

    layout->addWidget(_videoWidget);
    addSeparator();
    _devicesLogLabel = new QLabel("Журналирование устройств");
    layout->addWidget(_devicesLogLabel);
    _devicesList = new DeviceListView(uavController);
    layout->addWidget(_devicesList);
}

SessionTool::~SessionTool()
{

}

void SessionTool::setSessionDescription(const  bmcl::Option<mccmsg::TmSessionDescription>& session)
{
    _session = session;
    if(session.isSome())
    {
        std::string path = fmt::format("{}/{}/{}.mp4", mccpath::getLogsPath(), (*session)->folder(), mccmsg::TmSessionDescriptionObj::genVideoFile());
        _videoWidget->setFileName(QString::fromStdString(path));
        _videoWidget->startRecord();
    }
    else
    {
        _videoWidget->stopRecord();
    }

    _videoWidget->setEnabled(session.isNone());
    _devicesList->setEnabled(session.isNone());
}

void SessionTool::addSeparator()
{
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout()->addWidget(line);
}

void SessionTool::showEvent(QShowEvent *event)
{
    bool hasDevices = !_devicesList->empty();
    _devicesList->setVisible(hasDevices);
    _devicesLogLabel->setVisible(hasDevices);
    QWidget::showEvent(event);
}

