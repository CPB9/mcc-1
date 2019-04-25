#pragma once

#include "mcc/plugin/PluginCache.h"

#include "mcc/ui/Settings.h"
#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/SettingsPagePlugin.h"

class QSpinBox;
class QComboBox;
class QLabel;
class QLineEdit;

class RecorderSettingsPage : public mccui::SettingsPage
{
public:
    RecorderSettingsPage(mccui::Settings* settings, QWidget* parent = nullptr);
    void load() override;
    void apply() override;
    void saveOld() override;
    void restoreOld() override;
    QString pagePath() const override;
    QString pageTitle() const override;
    QIcon pageIcon() const override;

private:
    void updatePreview();
private:
    mccui::Rc<mccui::Settings>   _coreSettings;
    QSpinBox* _fps;
    QComboBox* _codec;
    QComboBox* _captureMode;
    QLineEdit* _ffmpegPath;
    QLabel* _preview;

    mccui::Rc<mccui::SettingsWriter> _fpsWriter;
    mccui::Rc<mccui::SettingsWriter> _ffmpegPathWriter;
    mccui::Rc<mccui::SettingsWriter> _codecWriter;
};

class RecorderSettingsPagePlugin : public mccui::SettingsPagePlugin
{
public:
    bool init(mccplugin::PluginCache* cache) override;
};
