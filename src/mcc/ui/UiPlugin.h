#pragma once

#include "mcc/Config.h"
#include "mcc/plugin/Plugin.h"

#include <Qt>

class QWidget;

namespace mccui {

class MCC_UI_DECLSPEC UiPlugin : public mccplugin::Plugin {
public:
    UiPlugin(const char* id);
    ~UiPlugin() override;

    virtual void setMccMainWindow(QWidget* parent);
    virtual Qt::Alignment alignment() const;
};
}
