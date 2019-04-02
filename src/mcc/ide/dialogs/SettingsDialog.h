#pragma once

#include "mcc/ui/Dialog.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Rc.h"

#include <bmcl/OptionPtr.h>

#include <vector>

class QDialogButtonBox;
class QTreeView;
class QVBoxLayout;
class QLabel;

namespace mccide {

class SettingsTreeItem;
class SettingsTreeModel;

class MCC_IDE_DECLSPEC SettingsDialog : public mccui::Dialog
{
public:
    explicit SettingsDialog(mccui::Settings* settings, QWidget* parent = nullptr);
    ~SettingsDialog() override;

    bool addPage(mccui::SettingsPage* page);

    void apply();
    void load();
    void saveOld();
    void restoreOld();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void restoreDefaults();

private:
    void foreachPage(void (mccui::SettingsPage::*functor)());

    QVBoxLayout*                        _containerLayout;
    QTreeView*                          _settingsTreeView;
    QLabel*                             _pageLabel;
    SettingsTreeModel*                  _settingsModel;
    QDialogButtonBox*                   _buttonBox;
    QWidget*                            _pageContainer;
    bmcl::OptionPtr<QWidget>            _currentSettingsWidget;
    std::vector<mccui::SettingsPage*>   _pages;
    mccuav::Rc<mccui::Settings>         _settings;
};
}
