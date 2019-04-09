#include "GpsSettingsPage.h"

#include "ui_GpsSettingsPage.h"

#include "mcc/ide/view/SerialConnectionWidget.h"
#include "mcc/plugin/PluginCache.h"
#include "mcc/ui/SettingsPagePlugin.h"

#include "mcc/ui/Settings.h"

#include <QPushButton>
#include <QIcon>

GpsSettingsPage::GpsSettingsPage(mccui::Settings* settings, QWidget* parent)
    : SettingsPage(parent)
    , _coreSettings(settings)
{
    _ui = new Ui::GpsSettingsPage;
    _ui->setupUi(this);

    _portWriter = settings->acquireUniqueWriter("gps/settings/port").unwrap();
    _speedWriter = settings->acquireUniqueWriter("gps/settings/speed").unwrap();

    setWindowTitle("GPS");

    updatePorts();

    connect(_ui->updatePortsButton, &QPushButton::clicked, this, &GpsSettingsPage::updatePorts);

    connect(_ui->portComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [this]()
    {
        applySettings();
    });
    connect(_ui->serialSpeedComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [this]()
    {
        applySettings();
    });
}

GpsSettingsPage::~GpsSettingsPage()
{
    delete _ui;
}

QString GpsSettingsPage::pageTitle() const
{
    return "GPS";
}

QString GpsSettingsPage::pagePath() const
{
    return "GPS";
}

QIcon GpsSettingsPage::pageIcon() const
{
    return QIcon(":/settingspage-gps/icon.png");
}

void GpsSettingsPage::load()
{
    QString port = _portWriter->read().toString();

    int idx = _ui->portComboBox->findText(port);

    if (idx != -1)
        _ui->portComboBox->setCurrentIndex(idx);

    int speed = _speedWriter->read().toInt();
    idx = _ui->serialSpeedComboBox->findText(QString::number(speed));

    if (idx != -1)
        _ui->serialSpeedComboBox->setCurrentIndex(idx);
}

void GpsSettingsPage::apply()
{
    applySettings();
}

void GpsSettingsPage::saveOld()
{
    _old.portIndex = _ui->portComboBox->currentIndex();
    _old.serialSpeedIndex = _ui->serialSpeedComboBox->currentIndex();
}

void GpsSettingsPage::restoreOld()
{
    _ui->portComboBox->setCurrentIndex(_old.portIndex);
    _ui->serialSpeedComboBox->setCurrentIndex(_old.serialSpeedIndex);
}

void GpsSettingsPage::updatePorts()
{
    auto ports = mccide::SerialConnectionWidget::getSerialPortInfo();

    std::sort(ports.begin(), ports.end(), [](const mccide::SerialPortInfo& p1, const mccide::SerialPortInfo& p2){ return p1.portName < p2.portName; });

    _ui->portComboBox->clear();
    for (auto port : ports)
    {
        _ui->portComboBox->addItem(port.portName);
    }
}

void GpsSettingsPage::applySettings()
{
    QString port = _ui->portComboBox->currentText();
    int speed = _ui->serialSpeedComboBox->currentText().toInt();

    _portWriter->write(port);
    _speedWriter->write(speed);
}

class GpsPagePlugin : public mccui::SettingsPagePlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        if (settingsData.isNone()) {
            return false;
        }
        setSettingsPage(new GpsSettingsPage(settingsData->settings()));
        return true;
    }
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<GpsPagePlugin>());
}

MCC_INIT_PLUGIN(create);
