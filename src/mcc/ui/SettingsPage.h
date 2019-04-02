#pragma once

#include <QWidget>

#include "mcc/Config.h"

class QIcon;

namespace mccui {

class MCC_UI_DECLSPEC SettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);
    ~SettingsPage() override;

    virtual void load() = 0;
    virtual void apply() = 0;
    virtual void saveOld() = 0;
    virtual void restoreOld() = 0;

    virtual QString pagePath() const = 0;
    virtual QString pageTitle() const = 0;

    virtual QIcon pageIcon() const;
};
}
