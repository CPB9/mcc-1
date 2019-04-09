#include "RemoveUavDialog.h"

#include "mcc/res/Resource.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/TextUtils.h"

#include <QAbstractButton>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

//constexpr const char* questionTemplate = "Удалить аппарат <b>«%1»</b>?";
constexpr const char* questionEmpty = "Удалить выбранный аппарат?";

RemoveUavDialog::RemoveUavDialog(mccuav::UavController* uavController,
                                 mccuav::ChannelsController* channelsController,
                                 QWidget* parent)
    : mccui::Dialog (parent)
    , _uavController(uavController)
    , _channelsController(channelsController)
    , _deviceId(mccmsg::Device())
    , _question(new QLabel(questionEmpty, this))
    , _channelsBox(new QCheckBox("Удалить соответствующий канал обмена", this))
    , _allowRemoveChannel(false)
{
    setWindowTitle("Удаление аппарата");


    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    QHBoxLayout* horLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel(this);
    horLayout->addStretch();
    iconLabel->setPixmap(QPixmap::fromImage(mccres::loadImage(mccres::ResourceKind::DeleteButtonIcon).scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    horLayout->addWidget(iconLabel);
    horLayout->addWidget(_question);
    horLayout->addStretch();
    mainLayout->addLayout(horLayout);
    mainLayout->addWidget(_channelsBox);

    _channelsBox->setChecked(true);

    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::StandardButton::Ok);
    buttons->buttons().back()->setIcon(mccres::loadIcon(mccres::ResourceKind::OkButtonIcon));
    buttons->addButton(QDialogButtonBox::StandardButton::Cancel);
    buttons->buttons().back()->setIcon(mccres::loadIcon(mccres::ResourceKind::CancelButtonIcon));
    mainLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &RemoveUavDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &RemoveUavDialog::accept);
}

RemoveUavDialog::~RemoveUavDialog() {}

void RemoveUavDialog::setUav(const mccmsg::Device& deviceId)
{
    if(_deviceId == deviceId)
        return;

    _deviceId = deviceId;

    auto uav = this->uav();
    if(uav == nullptr)
    {
        _question->setText(questionEmpty);
        return;
    }

    _question->setText(QString("Удалить аппарат <b>«%1»</b>?").arg(mccui::shortTextLine(uav->getName(), 48)));
    updateUavChannels();
}

const mccmsg::Device&RemoveUavDialog::device() const
{
    return _deviceId;
}

bool RemoveUavDialog::isAllowToRemoveChannel() const
{
    return _allowRemoveChannel;
}

void RemoveUavDialog::showEvent(QShowEvent* event)
{
    mccui::Dialog::showEvent(event);
    updateUavChannels();
    adjustSize();
}

void RemoveUavDialog::accept()
{
    mccui::Dialog::accept();

    auto uav = this->uav();
    if(uav == nullptr)
        return;

    if(_allowRemoveChannel && _channelsBox->isChecked())
        _uavController->unregisterUavAndChannel(uav);
    else
        _uavController->unregisterUav(uav);
}

mccuav::Uav*RemoveUavDialog::uav() const
{
    auto uav = _uavController->uav(_deviceId);
    if(uav.isNone())
        return nullptr;
    return uav.unwrap();
}

void RemoveUavDialog::updateUavChannels()
{
    auto uav = this->uav();
    if(uav == nullptr)
        return;

    _allowRemoveChannel = false;
    if (uav->channels().size() == 1)
    {
        auto channel = uav->channels().front();
        size_t devicesInChannelCount = 0;
        for (const auto& d : _uavController->uavsList())
        {
            const auto& deviceChannels = d->channels();
            if (std::find(deviceChannels.begin(), deviceChannels.end(), channel) != deviceChannels.end())
                devicesInChannelCount++;
        }
        _allowRemoveChannel = devicesInChannelCount == 1;
    }

    _channelsBox->setVisible(_allowRemoveChannel);
}
