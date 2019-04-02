#pragma once

#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

class QCheckBox;
class QComboBox;

class CoordinateSettingsPage : public mccui::SettingsPage
{
    Q_OBJECT
public:
    explicit CoordinateSettingsPage(mccui::Settings* settings,
                               QWidget *parent = nullptr);
    ~CoordinateSettingsPage() override;

    mccui::Settings* settings() const;

    void load() override;
    void apply() override;
    void saveOld() override;
    void restoreOld() override;

    QString pageTitle() const override;
    QString pagePath() const override;
    QIcon pageIcon() const override;

    bool showConverterOnTopState() const;

    void setShowConverterOnTopState(bool show);

signals:
    void showConverterOnTopChanged(bool show);

public slots:

private:
    mccui::Rc<mccui::Settings>          _settings;
    mccui::Rc<mccui::SettingsWriter>    _showConverterOnTopWriter;

    QCheckBox*                          _showConverterOnTopBox;

    static constexpr bool showConverterOnTopDefault()   {return true;}

    struct OldValues
    {
        bool    showConverterOnTop;
    };
    OldValues       _old;

    Q_DISABLE_COPY(CoordinateSettingsPage)
};
