#include "VideoRecordWidget.h"

#include <QGridLayout>
#include <QLabel>

#include "mcc/uav/UavController.h"
#include "mcc/ui/SliderCheckBox.h"
#include "mcc/ui/TextUtils.h"

#include "ScreenRecorder.h"

VideoRecordWidget::VideoRecordWidget(const mccui::Rc<mccuav::UavController>& uavController, const mccui::Rc<mccui::Settings>& settings, QWidget* parent)
    : QWidget(parent)
{
    _recordEnabledWriter = settings->acquireUniqueWriter("videorecorder/enabled", false).unwrap();

    _slider = new mccui::OnOffSliderCheckBox();
    _recorder = new ScreenRecorder(settings.get(), this);
    _recordTime = new QLabel(this);
    _recordRect = new QLabel(this);
    _availableSize = new QLabel(this);

    connect(_recorder, &ScreenRecorder::recordTimeChanged, this, &VideoRecordWidget::updateData);
    connect(_recorder, &ScreenRecorder::storageInfoChanged, this, &VideoRecordWidget::updateData);
    //connect(_recorder, &ScreenRecorder::rectChanged, this, &VideoRecordWidget::updateData);
    connect(_recorder, &ScreenRecorder::started, this, &VideoRecordWidget::updateData);
    connect(_recorder, &ScreenRecorder::stopped, this, &VideoRecordWidget::updateData);
    connect(_recorder, &ScreenRecorder::storageInfoChanged, this, &VideoRecordWidget::updateData);
    connect(_recorder, &ScreenRecorder::error, this,
            [uavController](const QString& text)
            {
                uavController->onLog(bmcl::LogLevel::Critical, text.toStdString());
            }
    );

    connect(_recorder, &ScreenRecorder::message, this,
            [uavController](const QString& text)
            {
                uavController->onLog(bmcl::LogLevel::Warning, text.toStdString());
            }
    );
    auto layout = new QGridLayout();
    setLayout(layout);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->setSpacing(3);
    layout->addWidget(_slider, 0, 0);
    layout->addWidget(new QLabel("Запись видео"), 0, 1);
    layout->addWidget(_recordTime, 0, 2);
    layout->addWidget(_recordRect, 1, 1, 1, 2);
    layout->addWidget(_availableSize, 2, 1, 1, 2);

    connect(_slider, &mccui::OnOffSliderCheckBox::sliderStateChanged, this,
            [this](const bmcl::Uuid& uuid, bool checked)
            {
                _slider->setChecked(checked);
                _recordEnabledWriter->write(checked);
            }
    );

    _slider->setChecked(_recordEnabledWriter->read().toBool());
    updateData();
}

void VideoRecordWidget::setFileName(const QString& fileName)
{
    _recorder->setFileName(fileName);
}

void VideoRecordWidget::startRecord()
{
    if(_slider->isChecked())
        _recorder->startRecord();
}

void VideoRecordWidget::stopRecord()
{
    _recorder->stopRecord();
}

void VideoRecordWidget::updateData()
{
//     bool isRecording = _recorder->state() == ScreenRecorder::State::Started;
//     _slider->setChecked(isRecording);
//    _slider->setEnabled(!isRecording);
    _recordTime->setText(_recorder->recordTime().toString("HH:mm:ss"));
    _recordRect->setText(_recorder->currentSettings());
    _availableSize->setText(QString("Свободно: %1").arg(mccui::bytesToString(_recorder->bytesAvailable())));
}

