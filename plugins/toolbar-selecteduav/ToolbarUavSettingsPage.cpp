#include "ToolbarUavSettingsPage.h"
#include "mcc/ui/Settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

constexpr const char* _showToolbarUavStatisticsKey("toolbar-selecteduav/showToolbarUavStatistics");
constexpr const char* _showListUavStatisticsKey("toolbar-selecteduav/showListUavStatistics");

ToolbarUavSettingsPage::ToolbarUavSettingsPage(mccui::Settings* settings, QWidget *parent)
    : mccui::SettingsPage(parent)
    , _settings(settings)

    , _showToolbarUavStatisticsWriter(settings->acquireUniqueWriter(_showToolbarUavStatisticsKey, showToolbarUavStatisticsDefault()).unwrap())
    , _showListUavStatisticsWriter(settings->acquireUniqueWriter(_showListUavStatisticsKey, showListUavStatisticsDefault()).unwrap())

    , _showToolbarUavStatisticsBox(new QCheckBox("Отображать график обмена для аппарата в тулбаре"))
    , _showListUavStatisticsBox(new QCheckBox("Отображать график обмена для аппаратов в выпадающем списке"))
{
    setWindowTitle("Аппараты");

    // Behaviour
    auto infoGroupBox = new QGroupBox("Поведение инструментов");
    auto infoLayout = new QGridLayout;
    infoGroupBox->setLayout(infoLayout);
    infoLayout->addWidget(_showToolbarUavStatisticsBox, 0, 0);
    infoLayout->addWidget(_showListUavStatisticsBox,    1, 0);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoGroupBox);
    mainLayout->addStretch();
    setLayout(mainLayout);

    connect(_showToolbarUavStatisticsBox, &QCheckBox::stateChanged,
          [this](int state)
    {
        bool showing = state;
        _showToolbarUavStatisticsWriter->write(showing);

        emit showToolbarUavStatisticsChanged(showing);
    });
    connect(_showListUavStatisticsBox, &QCheckBox::stateChanged,
          [this](int state)
    {
        bool showing = state;
        _showListUavStatisticsWriter->write(showing);

        emit showListUavStatisticsChanged(showing);
    });
}

QString ToolbarUavSettingsPage::pageTitle() const
{
    return "Аппараты";
}

QString ToolbarUavSettingsPage::pagePath() const
{
    return "Строка состояния/Аппараты";
}

QIcon ToolbarUavSettingsPage::pageIcon() const
{
    return QIcon(":/mapwidget-uav/icon.png");
}

ToolbarUavSettingsPage::~ToolbarUavSettingsPage()
{}

mccui::Settings* ToolbarUavSettingsPage::settings() const
{
    return _settings.get();
}

void ToolbarUavSettingsPage::load()
{
    bool showing = _showToolbarUavStatisticsWriter->read().toBool();
    _showToolbarUavStatisticsBox->setCheckState(showing ? Qt::Checked : Qt::Unchecked);
    emit showToolbarUavStatisticsChanged(showing);

    showing = _showListUavStatisticsWriter->read().toBool();
    _showListUavStatisticsBox->setCheckState(showing ? Qt::Checked : Qt::Unchecked);
    emit showListUavStatisticsChanged(showing);
}

void ToolbarUavSettingsPage::apply()
{
    emit showToolbarUavStatisticsChanged(_showToolbarUavStatisticsBox->isChecked());
    emit showListUavStatisticsChanged(_showListUavStatisticsBox->isChecked());
}

void ToolbarUavSettingsPage::saveOld()
{
    _old.showToolbarUavStatistics = _showToolbarUavStatisticsBox->isChecked();
    _old.showListUavStatistics = _showListUavStatisticsBox->isChecked();
}

void ToolbarUavSettingsPage::restoreOld()
{
    _showToolbarUavStatisticsBox->setCheckState(_old.showToolbarUavStatistics ? Qt::Checked : Qt::Unchecked);
    _showListUavStatisticsBox->setCheckState(_old.showListUavStatistics ? Qt::Checked : Qt::Unchecked);
}

bool ToolbarUavSettingsPage::showToolbarUavStatisticsState() const
{
    return _showToolbarUavStatisticsBox->isChecked();
}

bool ToolbarUavSettingsPage::showListUavStatisticsState() const
{
    return _showListUavStatisticsBox->isChecked();
}

void ToolbarUavSettingsPage::setShowToolbarUavStatisticsState(bool show)
{
    if(showToolbarUavStatisticsState() == show)
        return;

    _showToolbarUavStatisticsBox->setCheckState(show ? Qt::Checked : Qt::Unchecked);
    emit showToolbarUavStatisticsChanged(show);
}

void ToolbarUavSettingsPage::setShowListUavStatisticsState(bool show)
{
    if(showListUavStatisticsState() == show)
        return;

    _showListUavStatisticsBox->setCheckState(show ? Qt::Checked : Qt::Unchecked);
    emit showListUavStatisticsChanged(show);
}
