#include "CoordinateSystemSettingsPage.h"
#include "mcc/ui/Settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

constexpr const char* _showConverterOnTopKey("toolbar-coordinatesystem/showConverterOnTop");

CoordinateSettingsPage::CoordinateSettingsPage(mccui::Settings* settings, QWidget *parent)
    : mccui::SettingsPage(parent)
    , _settings(settings)
    , _showConverterOnTopWriter(settings->acquireUniqueWriter(_showConverterOnTopKey, showConverterOnTopDefault()).unwrap())
    , _showConverterOnTopBox(new QCheckBox("Отображать конвертер координат поверх основного окна"))
{
    setWindowTitle("Утилиты");

    // Behaviour
    auto infoGroupBox = new QGroupBox("Поведение инструментов");
    auto infoLayout = new QGridLayout;
    infoGroupBox->setLayout(infoLayout);
    infoLayout->addWidget(_showConverterOnTopBox,   0, 0);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoGroupBox);
    mainLayout->addStretch();
    setLayout(mainLayout);

    connect(_showConverterOnTopBox, &QCheckBox::stateChanged,
          [this](int state)
    {
        bool showing = state;
        _showConverterOnTopWriter->write(showing);

        emit showConverterOnTopChanged(showing);
    });
}

QString CoordinateSettingsPage::pageTitle() const
{
    return "Утилиты";
}

QString CoordinateSettingsPage::pagePath() const
{
    return "Утилиты";
}

QIcon CoordinateSettingsPage::pageIcon() const
{
    return QIcon(":/toolbar-coordinatesystem/resources/icons/settings.png");
}

CoordinateSettingsPage::~CoordinateSettingsPage()
{}

mccui::Settings* CoordinateSettingsPage::settings() const
{
    return _settings.get();
}

void CoordinateSettingsPage::load()
{
    bool showing = _showConverterOnTopWriter->read().toBool();
    _showConverterOnTopBox->setCheckState(showing ? Qt::Checked : Qt::Unchecked);
    emit showConverterOnTopChanged(showing);
}

void CoordinateSettingsPage::apply()
{
    emit showConverterOnTopChanged(_showConverterOnTopBox->isChecked());
}

void CoordinateSettingsPage::saveOld()
{
    _old.showConverterOnTop = _showConverterOnTopBox->isChecked();
}

void CoordinateSettingsPage::restoreOld()
{
    _showConverterOnTopBox->setCheckState(_old.showConverterOnTop ? Qt::Checked : Qt::Unchecked);
}

bool CoordinateSettingsPage::showConverterOnTopState() const
{
    return _showConverterOnTopBox->isChecked();
}

void CoordinateSettingsPage::setShowConverterOnTopState(bool show)
{
    if(showConverterOnTopState() == show)
        return;

    _showConverterOnTopBox->setCheckState(show ? Qt::Checked : Qt::Unchecked);
    emit showConverterOnTopChanged(show);
}
