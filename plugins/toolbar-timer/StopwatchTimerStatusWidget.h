#pragma once

#include "Types.h"

#include <QFrame>

#include <vector>

namespace mccui { class Settings; };
class QLabel;
class QSystemTrayIcon;
class StopwatchTimerTool;

class StopwatchTimerStatusLine : public QWidget
{
public:
    explicit StopwatchTimerStatusLine(const QString& text, QWidget* parent = nullptr);
    explicit StopwatchTimerStatusLine(QWidget* parent = nullptr);
    ~StopwatchTimerStatusLine() override;

    const QPixmap* iconPixmap() const;
    const QString iconSymbol() const;
    QString text() const;

    void setIcon(const QPixmap& pixmap);
    void setIcon(const QString& symbol);
    void setText(const QString& text);
    void clearText();

    StopwatchTimerTypes type() const { return _type; }
    void setType(const StopwatchTimerTypes& type);

private:
    QLabel*             _icon;
    QLabel*             _text;
    StopwatchTimerTypes _type;
    const QString       _indicatorWrapper;

    Q_DISABLE_COPY(StopwatchTimerStatusLine)
};

class StopwatchTimerStatusWidget : public QFrame
{
    Q_OBJECT

public:
    explicit StopwatchTimerStatusWidget(mccui::Settings* settings, QWidget* parent = nullptr);
    ~StopwatchTimerStatusWidget() override;
    void clearValue(size_t index);
    void setValue(size_t index, const QString& value);
    void setValue(size_t index, const QString& value, StopwatchTimerTypes type);
    void setIndicator(size_t index, StopwatchTimerTypes type);
    void inform(const QString& text);
    void cancelInform();
    bool hasInformation() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateText();
    void updateClock();

private:
    StopwatchTimerTool*                     _tool;
    QSystemTrayIcon*                        _systemTray;

    StopwatchTimerStatusLine*               _clock;
    std::vector<StopwatchTimerStatusLine*>  _lines;
    QTimer*                                 _clockTimer;

private:
    Q_DISABLE_COPY(StopwatchTimerStatusWidget)
};
