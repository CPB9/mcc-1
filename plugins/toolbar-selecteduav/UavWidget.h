#pragma once

#include <cstddef>
#include "mcc/msg/Fwd.h"
#include "mcc/msg/SubHolder.h"
#include "mcc/msg/exts/ITmExtension.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"
#include <bmcl/Option.h>
#include <QWidget>

class QHBoxLayout;
class QFrame;
class QLabel;

namespace mccui{
class ClickableLabel;
}

class UavFailureWidget;
class UavModeWidget;
class UavNameWidget;
class UavStatisticsWidget;

class UavWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UavWidget(mccuav::Uav* uav,
                       mccuav::UavController* uavController,
                       mccuav::GlobalActions* actions,
                       QWidget* parent = nullptr);
    ~UavWidget() override;

    mccuav::Uav* currentUav() const {return _currentUav;}

    bool isSeparatedMode() const { return _separatedMode; }
    void setSeparatedMode(bool isSeparatedMode);

    void setStatisticsVisible(bool visible);
    bool isStatisticsVisible() const {return _isStatisticsVisible;}

public slots:
    void updateCurrentUav(mccuav::Uav* uav);
    void updateUavActivation();

    void updatePixmap();
    void updateName();
    void updateCommands();
    void updateModes();
    void updateFailures();
    void updateStatistics();

public:
    bool event(QEvent *event) override;

signals:
    void vehicleMenuClicked(const mccmsg::Device& device);

private slots:
    void tmStorageUpdated();
    void tmStorageClear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void checkSelection();
    void updateMinimumSize();

private:
    const QSize                             _iconsSize;

    mccui::Rc<mccuav::UavController>        _uavController;
    mccui::Rc<mccuav::GlobalActions>        _actions;
    mccuav::Uav*                            _currentUav;
    bmcl::Option<mccmsg::SubHolder>         _handlerState;

    UavNameWidget*                          _nameWidget;
    UavModeWidget*                          _modeWidget;
    UavFailureWidget*                       _failureWidget;
    UavStatisticsWidget*                    _statisticsWidget;
    mccui::ClickableLabel*                  _menu;
    mccui::OnOffSliderCheckBox*             _slider;
    QFrame*                                 _line;

    bool                                    _separatedMode;
    bool                                    _hovered;
    bool                                    _isStatisticsVisible;

    Q_DISABLE_COPY(UavWidget)
};
