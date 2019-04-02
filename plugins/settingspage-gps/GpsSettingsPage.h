#pragma once

#include "mcc/Config.h"
#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

namespace Ui { class GpsSettingsPage; }

class GpsSettingsPage : public mccui::SettingsPage
{
    Q_OBJECT
public:
    GpsSettingsPage(mccui::Settings* settings, QWidget* parent = nullptr);
    ~GpsSettingsPage() override;

    void load() override;
    void apply() override;
    void saveOld() override;
    void restoreOld() override;

    QString pageTitle() const override;
    QString pagePath() const override;
    QIcon pageIcon() const override;

private slots:
    void updatePorts();

private:
    void applySettings();

    Ui::GpsSettingsPage*        _ui;
    mccui::Rc<mccui::Settings>   _coreSettings;
    mccui::Rc<mccui::SettingsWriter>   _portWriter;
    mccui::Rc<mccui::SettingsWriter>   _speedWriter;

    struct OldValues
    {
        int portIndex;
        int serialSpeedIndex;
    };
    OldValues                   _old;
};
