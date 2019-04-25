#include "SettingsPage.h"
#include "mcc/ui/Settings.h"
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

#include "ScreenRecorder.h"

RecorderSettingsPage::RecorderSettingsPage(mccui::Settings* settings, QWidget* parent /*= nullptr*/)
    : _coreSettings(settings)
{
    _fpsWriter = settings->acquireUniqueWriter("videorecorder/fps", ScreenRecorder::defaultFps()).unwrap();
    _ffmpegPathWriter = settings->acquireUniqueWriter("videorecorder/ffmpegPath", ScreenRecorder::defaultFfmpegPath()).unwrap();
    _codecWriter = settings->acquireUniqueWriter("videorecorder/codec", ScreenRecorder::defaultCodec()).unwrap();

    auto layout = new QGridLayout();
    setLayout(layout);

    _ffmpegPath = new QLineEdit(this);
    _ffmpegPath->setText(ScreenRecorder::defaultFfmpegPath());

    _fps = new QSpinBox(this);
    _fps->setMinimum(1);
    _fps->setMaximum(60);
    _codec = new QComboBox(this);
    for(const auto& c : ScreenRecorder::availableCodecs())
    {
        _codec->addItem(c);
    }
    _captureMode = new QComboBox(this);
    _captureMode->addItem("desktop");

    _preview = new QLabel(this);
    layout->addWidget(new QLabel("Путь к ffmpeg:", this), 0, 0);
    layout->addWidget(new QLabel("Частота кадров:"), 1, 0);
    layout->addWidget(new QLabel("Кодек:"), 2, 0);
    layout->addWidget(new QLabel("Режим захвата:"), 3, 0);
    layout->addWidget(new QLabel("Командная строка:"), 4, 0);
    layout->addWidget(_ffmpegPath, 0, 1);
    layout->addWidget(_fps, 1, 1);
    layout->addWidget(_codec, 2, 1);
    layout->addWidget(_captureMode, 3, 1);
    layout->addWidget(_preview, 4, 1);
    layout->setRowStretch(5, 1);
    updatePreview();

    connect(_codec, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RecorderSettingsPage::updatePreview);
    connect(_fps, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &RecorderSettingsPage::updatePreview);
}

void RecorderSettingsPage::load()
{
    _ffmpegPath->setText(_ffmpegPathWriter->read().toString());
    _fps->setValue(_fpsWriter->read().toInt());
    int codecIdx = _codec->findText(_codecWriter->read().toString());
    if(codecIdx != -1)
        _codec->setCurrentIndex(codecIdx);
}

void RecorderSettingsPage::apply()
{
    _ffmpegPathWriter->write(_ffmpegPath->text());
    _fpsWriter->write(_fps->value());
    _codecWriter->write(_codec->currentText());
}

void RecorderSettingsPage::saveOld()
{
}

void RecorderSettingsPage::restoreOld()
{
}

QString RecorderSettingsPage::pagePath() const
{
    return "Запись видео";
}

QString RecorderSettingsPage::pageTitle() const
{
    return "Запись видео";
}

QIcon RecorderSettingsPage::pageIcon() const
{
    return QIcon(":/toolbar-sessions/settings_icon.png");
}

void RecorderSettingsPage::updatePreview()
{
    _preview->setText(ScreenRecorder::genCommandLine(_ffmpegPath->text(), _fps->value(), _codec->currentText()) + " %OUTPUTFILE");
}

bool RecorderSettingsPagePlugin::init(mccplugin::PluginCache* cache)
{
    auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
    if(settingsData.isNone())
    {
        return false;
    }
    setSettingsPage(new RecorderSettingsPage(settingsData->settings()));
    return true;
}
