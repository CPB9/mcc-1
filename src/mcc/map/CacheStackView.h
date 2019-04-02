#pragma once

#include "mcc/Config.h"
#include "mcc/map/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/SettingsPage.h"

#include <QWidget>
#include <QByteArray>

namespace mccmap {

class CacheStackModel;

class MCC_MAP_DECLSPEC CacheStackView : public mccui::SettingsPage {
    Q_OBJECT
public:
    explicit CacheStackView(CacheStackModel* model, mccui::Settings* settings, QWidget* parent = 0);
    ~CacheStackView();

    bool event(QEvent*) override;

    void load() override;
    void apply() override;
    void saveOld() override;
    void restoreOld() override;

    QString pagePath() const override;
    QString pageTitle() const override;
    QIcon pageIcon() const override;

signals:
    void clicked();

private:
    CacheStackModel* _model;
    mccui::TableEditWidget* _tableEdit;

    Rc<mccui::Settings> _settings;
    QByteArray _lastState;
};
}
