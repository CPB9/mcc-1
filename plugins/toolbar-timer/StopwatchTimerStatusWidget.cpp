#include "StopwatchTimerStatusWidget.h"
#include "StopwatchTimerTool.h"

#include "mcc/ui/WidgetUtils.h"

#include <QApplication>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

constexpr const char* bgColorSS = "background-color: transparent;";

StopwatchTimerStatusLine::StopwatchTimerStatusLine(const QString& text, QWidget* parent)
    : QWidget (parent)
    , _icon(new QLabel)
    , _text(new QLabel(text))
    , _type(StopwatchTimerTypes::Other)
    , _indicatorWrapper("background-color: transparent;"
                        "color: #%1")
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addStretch(1);
    mainLayout->addWidget(_icon);
    mainLayout->addWidget(_text);
    mainLayout->addStretch(1);

    QFont newFont = font();
    newFont.setWeight(QFont::DemiBold);
    _text->setFont(newFont);

    _icon->setStyleSheet(bgColorSS);
    _text->setStyleSheet(bgColorSS);
    setStyleSheet(bgColorSS);
}

StopwatchTimerStatusLine::StopwatchTimerStatusLine(QWidget* parent)
    : StopwatchTimerStatusLine(QString(), parent)
{}

StopwatchTimerStatusLine::~StopwatchTimerStatusLine() {}

const QPixmap* StopwatchTimerStatusLine::iconPixmap() const
{
    return _icon->pixmap();
}

const QString StopwatchTimerStatusLine::iconSymbol() const
{
    return _icon->text();
}

QString StopwatchTimerStatusLine::text() const
{
    return _text->text();
}

void StopwatchTimerStatusLine::setIcon(const QPixmap& pixmap)
{
    _icon->setPixmap(pixmap);
}

void StopwatchTimerStatusLine::setIcon(const QString& symbol)
{
    _icon->setText(symbol);
}

void StopwatchTimerStatusLine::setText(const QString& text)
{
    if(text == _text->text())
        return;

    _text->setText(text);
    setVisible(!text.isEmpty());
}

void StopwatchTimerStatusLine::clearText()
{
    _text->clear();
    hide();
}

void StopwatchTimerStatusLine::setType(const StopwatchTimerTypes& type)
{
    if(_type == type)
        return;

    _type = type;

    if(_type == StopwatchTimerTypes::Direct)
    {
        _icon->setStyleSheet(_indicatorWrapper.arg("08d008"));
        _icon->setText("▲");
    }
    else if(_type == StopwatchTimerTypes::Reverse)
    {
        _icon->setStyleSheet(_indicatorWrapper.arg("0060e0"));
        _icon->setText("▼");
    }
    else
    {
        _icon->setStyleSheet(_indicatorWrapper.arg("000000"));
        _icon->setText(QString());
    }
}

constexpr int margin = 5;

StopwatchTimerStatusWidget::StopwatchTimerStatusWidget(mccui::Settings* settings, QWidget* parent)
    : QFrame(parent)
    , _tool(new StopwatchTimerTool(settings))
    , _systemTray(new QSystemTrayIcon(QIcon(":/app_icon.ico"), this))
    , _clock(new StopwatchTimerStatusLine)
    , _clockTimer(new QTimer(this))
{
    _tool->hide();

    setObjectName("StopwatchTimerStatusWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(margin, 0, margin, 0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch(1);
    mainLayout->addWidget(_clock);

    _tool->setInformator(this);
    _lines.reserve(_tool->timersAmount());
    for (size_t i = 0; i < _tool->timersAmount(); ++i)
    {
        _lines.push_back(new StopwatchTimerStatusLine);
        mainLayout->addWidget(_lines.back());
        _lines.back()->hide(); // without text
    }

    mainLayout->addStretch(1);

    setToolTip("Отобразить таймер-секундомер");

    _systemTray->setToolTip("МПУ: Запущен таймер-секундомер");
    connect(_systemTray, &QSystemTrayIcon::activated, this,
            [this]()
    {
        QWidget* mw = mccui::findMainWindow();
        if(mw != nullptr)
        {
            // Maximaze window
            mw->setWindowState((mw->windowState() &~ Qt::WindowMinimized) | Qt::WindowActive);
            mw->raise();
            mw->activateWindow();
        }

        _tool->requestShowing();
    });

    _clockTimer->setInterval(500);
    _clockTimer->start();

    connect(_clockTimer, &QTimer::timeout, this,
            [this]()
    {
        if(isVisible() && _clock->isVisible())
            updateClock();
    });

    QFontMetrics fontTest(qApp->font());
    QChar clockSymbol(0x023F0);  // ⏰ U+023F0
    if(fontTest.inFont(clockSymbol))
        _clock->setIcon(clockSymbol);

    setMinimumWidth(fontTest.boundingRect(QString(clockSymbol) + QString("XX:XX:XX.X")).width() + margin * 2);

#if defined(_MSC_VER)
    // HACK: QSystemTrayIcon doesn't hides by default on Windows 10
    _systemTray->setVisible(true);
    _systemTray->setVisible(false);
#endif

    updateText();
}

StopwatchTimerStatusWidget::~StopwatchTimerStatusWidget()
{
    delete _tool;
}

void StopwatchTimerStatusWidget::clearValue(size_t index)
{
    if(index >= _lines.size())
        return;

    _lines[index]->clearText();
    updateText();
}

void StopwatchTimerStatusWidget::setValue(size_t index, const QString &value)
{
    if(index >= _lines.size())
        return;

    _lines[index]->setText(value);
    updateText();
}

void StopwatchTimerStatusWidget::setValue(size_t index, const QString &value, StopwatchTimerTypes type)
{
    setValue(index, value);
    setIndicator(index, type);
}

void StopwatchTimerStatusWidget::setIndicator(size_t index, StopwatchTimerTypes type)
{
    if(index >= _lines.size())
        return;

    _lines[index]->setType(type);
    updateText();
}

void StopwatchTimerStatusWidget::inform(const QString &text)
{
    qApp->beep();

    _systemTray->show();
    _systemTray->showMessage("МПУ", text);

    _tool->requestShowing();
}

void StopwatchTimerStatusWidget::cancelInform()
{
    _systemTray->hide();
}

bool StopwatchTimerStatusWidget::hasInformation() const
{
    return _systemTray->isVisible();
}

void StopwatchTimerStatusWidget::mousePressEvent(QMouseEvent *event)
{
    _tool->requestShowing();
    QFrame::mousePressEvent(event);
}

void StopwatchTimerStatusWidget::showEvent(QShowEvent* event)
{
    QFrame::showEvent(event);
    updateClock();
}

void StopwatchTimerStatusWidget::updateText()
{
    bool trayVisible(false);

    for(auto l : _lines)
        if(l->isVisible())
        {
            trayVisible = true;
            break;
        }

    // Hack for popup-dialog hidding.
    bool isToolVisible = false;
    isToolVisible = _tool->isVisible();

    if(_systemTray->isVisible() != trayVisible)
        _systemTray->setVisible(trayVisible);

    if(isToolVisible)
        _tool->requestShowing();
}

void StopwatchTimerStatusWidget::updateClock()
{
    _clock->setText(QTime::currentTime().toString("HH:mm:ss"));
}
