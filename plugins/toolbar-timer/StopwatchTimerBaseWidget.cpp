#include "StopwatchTimerBaseWidget.h"
#include "StopwatchTimerStatusWidget.h"

#include "mcc/ui/Settings.h"

#include <QApplication>
#include <QBasicTimer>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTime>
#include <QTimeEdit>
#include <QTimer>
#include <QToolButton>

TimerInputDialog::TimerInputDialog(QWidget* parent)
    : mccui::Dialog(parent)
    , _editor(new QTimeEdit)
{
    setModal(true);
    setWindowTitle("Время отсчёта");

    QGridLayout *mainLayout = new QGridLayout(this);

    _editor->setAlignment(Qt::AlignCenter);
    _editor->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _editor->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    _editor->setMinimumTime(StopwatchTimerBaseWidget::minimumTime());
    _editor->setMaximumTime(StopwatchTimerBaseWidget::maximumTime());
    _editor->setCurrentSection(QDateTimeEdit::SecondSection);
    _editor->setDisplayFormat(QString::fromUtf8("H:mm:ss"));
    mainLayout->addWidget(_editor, 0, 1);

    QPushButton* okButton = new QPushButton("Ок");
    QPushButton* cancelButton = new QPushButton("Отмена");
    QPushButton* clearButton = new QPushButton("Очистить");
    connect(okButton, &QPushButton::clicked, this, [this]()
    {
        accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, [this]()
    {
        reject();
    });
    connect(clearButton, &QPushButton::clicked, this, [this]()
    {
        _editor->setTime(StopwatchTimerBaseWidget::minimumTime());
    });

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(okButton);
    buttons->addWidget(cancelButton);
    buttons->addWidget(clearButton);
    mainLayout->addLayout(buttons, 1, 0, 1, 3);
}

QTime TimerInputDialog::time() const
{
    return _editor->time();
}

// dirty hack for buttons sizing (QPushButton + QToolButton)

StopwatchTimerBaseWidget::~StopwatchTimerBaseWidget()
{
    delete _bgAnimation;
}

StopwatchTimerBaseWidget::StopwatchTimerBaseWidget(mccui::Settings* settings,
                                                   StopwatchTimerTypes type,
                                                   StopwatchTimerStatusWidget* informator,
                                                   size_t index,
                                                   QWidget* parent)
    : QFrame(parent)
    , _informator(informator)
    , _index(index)
    , _type(type)
    , _timer(new QTimer(this))
    , _time(type == StopwatchTimerTypes::Reverse ? minimumTime() : zeroTime())
    , _hasNotification(false)
    , _notificationValue()
    , _inputDialog(new TimerInputDialog(this))
    , _text(new StopwatchTimerLabel(this))
    , _start(new QToolButton)
    , _reset(new QToolButton)
    , _bgAnimation(new QPropertyAnimation(this, "animationBrush"))
    , _initialPalette(palette())
    , _settings(settings)
    , _directKey(timerSettingsKey() + QString::number(_index))
    , _reverseKey(stopwatchSettingsKey() + QString::number(_index))
{
    _directWriter = settings->acquireUniqueWriter(_directKey).unwrap();
    _reverseWriter = settings->acquireUniqueWriter(_reverseKey).unwrap();

    _textStyle = QString("margin-left:0.5em; margin-right:0.5em; color: %1");
    setObjectName("StopwatchTimerBaseWidget");

    _timer->setInterval(100);
    connect(_timer, &QTimer::timeout, this, &StopwatchTimerBaseWidget::timerTimeout);


    QHBoxLayout *layout = new QHBoxLayout(this);

    QFont font = _text->font();
    font.setWeight(QFont::Medium);
    font.setPointSize(font.pointSize() + 4);
    _text->setFont(font);
    _text->setAlignment(Qt::AlignCenter);

    _start->setStyleSheet("text-align: middle;");
    connect(_start, &QToolButton::clicked, this, &StopwatchTimerBaseWidget::startStopTimer);

    QMenu *resetMenu = new QMenu(this);
    _directAction  = resetMenu->addAction(QIcon(":/player/reset.png"), "Сбросить");
    connect(_directAction, &QAction::triggered, this, &StopwatchTimerBaseWidget::reset);
    _reverseAction = resetMenu->addAction(QIcon(":/player/record.png"), "Задать");
    connect(_reverseAction, &QAction::triggered, this, &StopwatchTimerBaseWidget::reset);
    _reset->setMenu(resetMenu);
    _reset->setDefaultAction(_directAction);

    layout->addWidget(_start);
    layout->addWidget(_reset);
    layout->addWidget(_text);

    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);

    QColor backgroundColor = palette().color(QPalette::Base); // It's better to use QPalette::Window. But there'is not correct background color.

    _bgAnimation->setDuration(3000);
    _bgAnimation->setKeyValueAt(0, backgroundColor);
    _bgAnimation->setKeyValueAt(0.2, QColor(0x4682b4));
    _bgAnimation->setKeyValueAt(0.4, backgroundColor);
    _bgAnimation->setKeyValueAt(0.6, QColor(0x4682b4));
    _bgAnimation->setKeyValueAt(0.8, backgroundColor);
    _bgAnimation->setEndValue(backgroundColor);

    connect(_bgAnimation, &QAbstractAnimation::finished, this, &StopwatchTimerBaseWidget::animationFinished);

    readSettings();

    connect(_inputDialog, &TimerInputDialog::accepted, this,
            [this]()
    {
        deactivateNotification(); // if dialog was opened before

        setType(StopwatchTimerTypes::Reverse);

        stopTimer();
        setTime(_inputDialog->time());

        _notificationValue = _time.toString("h:mm:ss");

        _start->setFocus();

        showParent();
    });

    connect(_inputDialog, &TimerInputDialog::rejected, this, &StopwatchTimerBaseWidget::showParent);
}

void StopwatchTimerBaseWidget::setType(StopwatchTimerTypes type)
{
    if(type != _type)
    {
        // old
        switch (_type) {
        case StopwatchTimerTypes::Direct:
            _directWriter->clear();
            break;
        case StopwatchTimerTypes::Reverse:
            _reverseWriter->clear();
            break;
        default:break;
        }

        _type = type;
    }

    switch (_type) {
    case StopwatchTimerTypes::Direct:
        _reset->setDefaultAction(_directAction);
        break;
    case StopwatchTimerTypes::Reverse:
        _reset->setDefaultAction(_reverseAction);
        break;
    default:break;
    }
}

QColor StopwatchTimerBaseWidget::animationBrush() const
{
    return palette().background().color();
}

void StopwatchTimerBaseWidget::setAnimationBrush(const QColor &color)
{
    auto p = palette();
    p.setColor(QPalette::Window, color);
    setPalette(p);
    update();
}

void StopwatchTimerBaseWidget::setInformator(StopwatchTimerStatusWidget* informator)
{
    if(_informator == informator)
        return;

    _informator = informator;
}

const QTime &StopwatchTimerBaseWidget::zeroTime()
{
    static QTime time = QTime(0, 0, 0);
    return time;
}

const QTime &StopwatchTimerBaseWidget::minimumTime()
{
    static QTime time = QTime(0, 0, 1);
    return time;
}

const QTime &StopwatchTimerBaseWidget::maximumTime()
{
    static QTime time = QTime(9, 59, 59);
    return time;
}

void StopwatchTimerBaseWidget::reset()
{
    deactivateNotification();

    QAction *act = qobject_cast<QAction *>(sender());
    if(!act)
    {
        act = _reset->defaultAction();
    }
    if(act)
    {
        if(act == _directAction)
        {
            setType(StopwatchTimerTypes::Direct);

            stopTimer();
            setTime(zeroTime());
            _start->setFocus();
        }
        else if(act == _reverseAction)
        {
            _inputDialog->show();
        }
    }
}

void StopwatchTimerBaseWidget::startStopTimer()
{
    deactivateNotification();

    if(_timer->isActive())
    {
        stopTimer();
    }
    else
    {
        startTimer();
    }
}

void StopwatchTimerBaseWidget::timerTimeout()
{
    if(_type == StopwatchTimerTypes::Direct)
    {
        if(_time < maximumTime())
        {
            _timer->start();

            setTime(_time.addMSecs(_timer->interval()));
        }
        else
        {
            stopTimer();
            setTime(maximumTime());
            _start->setEnabled(false);
            _text->setReady(false);
        }
    }
    else if(_type == StopwatchTimerTypes::Reverse)
    {
        QTime zeroTime(0, 0, 0, 0);

        if(zeroTime.msecsTo(_time) <= _timer->interval()) // Time is over
        {
            stopTimer();
            setTime(zeroTime);
            _start->setEnabled(false);
            _text->setReady(false);

            activateNotification();
        }
        else
        {
            _timer->start();

            setTime(_time.addMSecs(-_timer->interval()));
        }
    }
}

void StopwatchTimerBaseWidget::animationFinished()
{
    if(_hasNotification)
    {
        _bgAnimation->start();
    }
}

void StopwatchTimerBaseWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(hasMouseTracking())
    {
        deactivateNotification();
    }

    QFrame::mouseMoveEvent(event);
}

void StopwatchTimerBaseWidget::startTimer()
{
    _timer->start();

    _text->setStyleSheet(_textStyle.arg(_type == StopwatchTimerTypes::Direct ? "#08d008" : "#0094e0")); // TODO: use style polish

    _start->setEnabled(true);
    _start->setIcon(pauseIcon());
    _start->setToolTip("Приостановить таймер");
    _text->setReady(true);

    if(_type == StopwatchTimerTypes::Direct)
    {
        if(!_settings->contains(_directKey))
        {
            _directWriter->write(QDateTime::currentDateTime().toMSecsSinceEpoch());
        }
    }
    else if(_type == StopwatchTimerTypes::Reverse)
    {
        if(!_settings->contains(_reverseKey))
        {
            _reverseWriter->write(QDateTime::currentDateTime().toMSecsSinceEpoch() +
                                  static_cast<qint64>(_time.msecsSinceStartOfDay()));
        }
    }

    drawTime();
}

void StopwatchTimerBaseWidget::stopTimer()
{
    _timer->stop();

    _text->setStyleSheet(_textStyle.arg(_type == StopwatchTimerTypes::Direct ? "#80A080" : "#8080A0"));

    _start->setEnabled(true);
    _start->setIcon(playIcon());
    _start->setToolTip("Запустить таймер");
    _text->setReady(true);

    if(_type == StopwatchTimerTypes::Direct)
    {
        _directWriter->clear();
    }
    else if(_type == StopwatchTimerTypes::Reverse)
    {
        _reverseWriter->clear();
    }

    drawTime();
}

void StopwatchTimerBaseWidget::setTime(const QTime &time)
{
    _time = time;
    drawTime();
}

void StopwatchTimerBaseWidget::drawTime()
{
    QString timeText = _time.toString("h:mm:ss") + "." + QString::number(_time.msec()/_timer->interval());
    _text->setText(timeText);

    if(_informator != nullptr)
    {
        if(_timer->isActive())
        {
            _informator->setValue(_index, timeText, _type);
        }
        else
        {
            _informator->clearValue(_index);
        }
    }
}

void StopwatchTimerBaseWidget::activateNotification()
{
    if(!_hasNotification)
    {
        _hasNotification = true;

        setMouseTracking(true);

//        setMask(QRegion(0,0,width(),height(),QRegion::Ellipse)); // TODO: make rounded background by special QPolygon-mask

        setAutoFillBackground(true);
        _bgAnimation->start();

        if(_informator != nullptr)
            _informator->inform("Вышло время" + QString(!_notificationValue.isEmpty() ? ", заданное на " + _notificationValue : _notificationValue));
    }
}

void StopwatchTimerBaseWidget::deactivateNotification()
{
    if(_hasNotification)
    {
        _hasNotification = false;

        setMouseTracking(false);

        setAutoFillBackground(false);
        _bgAnimation->stop();

        setPalette(_initialPalette);
        update();

        if(_informator != nullptr)
            _informator->cancelInform();
    }
}

void StopwatchTimerBaseWidget::readSettings()
{
    qint64 diffMsec(0);

    if(_settings->contains(_directKey))
    {
        _type = StopwatchTimerTypes::Direct;
        _reverseWriter->clear(); // for another wrong key

        diffMsec = QDateTime::currentDateTime().toMSecsSinceEpoch() -
                   _settings->read(_directKey).toLongLong();
    }
    else if(_settings->contains(_reverseKey))
    {
        _type = StopwatchTimerTypes::Reverse;
        _directWriter->clear(); // for another wrong key

        diffMsec = _settings->read(_reverseKey).toLongLong() -
                   QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    setType(_type); // from settings or from constuctor

    if(diffMsec > 0 && diffMsec < maximumTime().msecsSinceStartOfDay())
    {
        setTime(QTime::fromMSecsSinceStartOfDay(diffMsec));
        startTimer();
    }
    else
    {
        // Wrong or ancient value
        _directWriter->clear();
        _reverseWriter->clear();

        stopTimer();
    }
}

void StopwatchTimerBaseWidget::showParent()
{
    if(parentWidget() != nullptr)
        parentWidget()->show();
}

//TODO: выпиплить статики
const QIcon &StopwatchTimerBaseWidget::playIcon()
{
    static QIcon icon = QIcon(":/player/play.png");
    return icon;
}

const QIcon& StopwatchTimerBaseWidget::pauseIcon()
{
    static QIcon icon = QIcon(":/player/pause.png");
    return icon;
}


StopwatchTimerLabel::StopwatchTimerLabel(StopwatchTimerBaseWidget* parent)
    : QLabel(parent)
    , _parent(parent)
    , _doubleClickTimer(new QBasicTimer)
    , _isReady(true)
{}

StopwatchTimerLabel::~StopwatchTimerLabel()
{
    delete _doubleClickTimer;
}

void StopwatchTimerLabel::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _doubleClickTimer->timerId())
    {
        // Pause for waiting double click is elapsed. So we understand mouse event like a single click
        _doubleClickTimer->stop();

        if(_parent && isReady())
        {
            _parent->startStopTimer();
        }

        event->accept();
    }
    else
    {
        QObject::timerEvent(event);
    }
}

void StopwatchTimerLabel::mousePressEvent(QMouseEvent *event)
{
    if(_parent)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            if(_doubleClickTimer->isActive())
            {
                // Double click
                _doubleClickTimer->stop();

                _parent->reset();
                event->accept();
            }
            else
            {
                _doubleClickTimer->start(QApplication::doubleClickInterval(), this);
            }
        }
        else if(event->buttons() & Qt::RightButton)
        {
            // Simple single click with right mouse button

            if(isReady())
            {
                _parent->startStopTimer();
                event->accept();
            }
        }
    }

    QLabel::mousePressEvent(event);
}
