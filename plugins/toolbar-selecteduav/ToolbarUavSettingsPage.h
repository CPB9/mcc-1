#pragma once

#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

class QCheckBox;
class QComboBox;

class ToolbarUavSettingsPage : public mccui::SettingsPage
{
    Q_OBJECT
public:
    explicit ToolbarUavSettingsPage(mccui::Settings* settings,
                                    QWidget *parent = nullptr);
    ~ToolbarUavSettingsPage() override;

    mccui::Settings* settings() const;

    void load() override;
    void apply() override;
    void saveOld() override;
    void restoreOld() override;

    QString pageTitle() const override;
    QString pagePath() const override;
    QIcon pageIcon() const override;

    bool showToolbarUavStatisticsState() const;
    bool showListUavStatisticsState() const;

    void setShowToolbarUavStatisticsState(bool show);
    void setShowListUavStatisticsState(bool show);

signals:
    void showToolbarUavStatisticsChanged(bool show);
    void showListUavStatisticsChanged(bool show);

private:
    mccui::Rc<mccui::Settings>          _settings;
    mccui::Rc<mccui::SettingsWriter>    _showToolbarUavStatisticsWriter;
    mccui::Rc<mccui::SettingsWriter>    _showListUavStatisticsWriter;

    QCheckBox*                          _showToolbarUavStatisticsBox;
    QCheckBox*                          _showListUavStatisticsBox;

    static constexpr bool showToolbarUavStatisticsDefault()   {return true;}
    static constexpr bool showListUavStatisticsDefault()   {return true;}

    struct OldValues
    {
        bool    showToolbarUavStatistics;
        bool    showListUavStatistics;
    };
    OldValues       _old;

    Q_DISABLE_COPY(ToolbarUavSettingsPage)
};
