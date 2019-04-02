#pragma once

#include "mcc/Config.h"
#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

namespace Ui { class GlobalSettingsPage; }

class GlobalSettingsPage : public mccui::SettingsPage
{
    Q_OBJECT
public:
    GlobalSettingsPage(mccui::CoordinateSystemController* csController, mccui::Settings* settings, QWidget* parent = nullptr);
    ~GlobalSettingsPage() override;

    void load() override;
    void apply() override;
    void saveOld() override;
    void restoreOld() override;

    QString pageTitle() const override;
    QString pagePath() const override;
    QIcon pageIcon() const override;

private:
    void updateLcsFormat(mccui::AngularFormat format);
    void updatecsController();
    void updateLcs();

    enum LcsMethod
    {
        CenterAndAngle = 0,
        CenterAndDirection
    };

    void checkLcsMethod();

    enum class LcsForm
    {
        CenterPoint = 0,
        DirectionPoint,
        AngleValue
    };

    void copyLcs(LcsForm form);
    void pasteLcs(LcsForm form);

    Ui::GlobalSettingsPage* _ui;

    mccui::CoordinateEditor* _lcsCenterLatitude;
    mccui::CoordinateEditor* _lcsCenterLongitude;
    mccui::CoordinateEditor* _lcsCenterAltitude;

    mccui::CoordinateEditor* _lcsDirectionLatitude;
    mccui::CoordinateEditor* _lcsDirectionLongitude;

    mccui::CoordinateEditor* _lcsDirectionAngle;
    mccui::Rc<mccui::CoordinateSystemController> _csController;
    mccui::Rc<mccui::Settings> _settings;
    mccui::Rc<mccui::SettingsWriter> _invertedPfdWriter;
    mccui::Rc<mccui::SettingsWriter> _showOnMapPfdWriter;

    struct OldValues
    {
        int     coordSystemIndex;
        int     coordFormatIndex;
        int     lcsMethodIndex;

        double  lcsCenterLatitude;
        double  lcsCenterLongitude;
        double  lcsCenterAltitude;

        double  lcsDirectionLatitude;
        double  lcsDirectionLongitude;
        double  lcsDirectionAngle;

        bool    invertedPfdIndication;
    };
    OldValues               _old;
};
