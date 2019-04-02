#pragma once

#include "mcc/ui/Dialog.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"

#include "Types.h"

#include <QLabel>
#include <QTime>

class QAction;
class QBasicTimer;
class QIcon;
class QPropertyAnimation;
class QTimeEdit;
class QTimer;
class QToolButton;

class StopwatchTimerStatusWidget;
class StopwatchTimerLabel;

class TimerInputDialog : public mccui::Dialog
{
    Q_OBJECT

public:
    explicit TimerInputDialog(QWidget *parent = nullptr);
    QTime time() const;

private:
    QTimeEdit* _editor;

private:
    Q_DISABLE_COPY(TimerInputDialog)
};

class StopwatchTimerBaseWidget : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor animationBrush READ animationBrush WRITE setAnimationBrush)

public:
    explicit StopwatchTimerBaseWidget(mccui::Settings* settings,
                                      StopwatchTimerTypes type = StopwatchTimerTypes::Direct,
                                      StopwatchTimerStatusWidget *informator = nullptr,
                                      size_t index = 0,
                                      QWidget *parent = nullptr);
    ~StopwatchTimerBaseWidget() override;

    void setType(StopwatchTimerTypes type);

    QColor animationBrush() const;
    void setAnimationBrush(const QColor& color);

    void setInformator(StopwatchTimerStatusWidget* informator);

    static const QTime& zeroTime();
    static const QTime& minimumTime();
    static const QTime& maximumTime();

public slots:
    void reset();
    void startStopTimer();
    void timerTimeout();
    void animationFinished();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void startTimer();
    void stopTimer();
    void setTime(const QTime &time);
    void drawTime();
    void activateNotification();
    void deactivateNotification();
    void readSettings();
    void showParent();
    static const QIcon& playIcon();
    static const QIcon& pauseIcon();
    static const QString timerSettingsKey() {return "utils/lastTimerStart";}
    static const QString stopwatchSettingsKey() {return "utils/lastStopwatchSet";}

private:
    StopwatchTimerStatusWidget* _informator;
    size_t                      _index;
    StopwatchTimerTypes         _type;
    QTimer*                     _timer;
    QTime                       _time;
    bool                        _hasNotification;
    QString                     _notificationValue;

    TimerInputDialog*           _inputDialog;
    StopwatchTimerLabel*        _text;
    QToolButton*                _start;
    QToolButton*                _reset;
    QAction*                    _directAction;
    QAction*                    _reverseAction;

    QPropertyAnimation*         _bgAnimation;
    QPalette                    _initialPalette;

    QString        _textStyle;

    mccui::Rc<mccui::Settings>   _settings;
    mccui::Rc<mccui::SettingsWriter>   _directWriter;
    mccui::Rc<mccui::SettingsWriter>   _reverseWriter;
    const QString               _directKey;
    const QString               _reverseKey;

private:
    Q_DISABLE_COPY(StopwatchTimerBaseWidget)
};

class StopwatchTimerLabel : public QLabel
{
    Q_OBJECT

public:
    explicit StopwatchTimerLabel(StopwatchTimerBaseWidget *parent);
    ~StopwatchTimerLabel() override;

    void setReady(bool ready = true) {_isReady = ready;}
    bool isReady() const {return _isReady;}

protected:
    void timerEvent(QTimerEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    StopwatchTimerBaseWidget*   _parent;
    QBasicTimer*                _doubleClickTimer;
    bool                        _isReady;

    Q_DISABLE_COPY(StopwatchTimerLabel)
};
