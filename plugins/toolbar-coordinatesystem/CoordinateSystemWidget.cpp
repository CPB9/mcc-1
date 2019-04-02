#include "CoordinateSystemWidget.h"

#include "CoordinateSystemPropertiesDialog.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/ClickableLabel.h"
#include "mcc/ui/CoordinateSystem.h"
#include "mcc/ui/CoordinateSystemController.h"

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>

constexpr const char* bgColorSS = "background-color: transparent;";

CoordinateSystemWidget::CoordinateSystemWidget(mccui::CoordinateSystemController* csController,
                                               mccuav::GlobalActions* actions,
                                               QWidget* parent)
    : QWidget(parent)
    , _csController(csController)
    , _system(new QLabel())
    , _format(new QLabel())
    , _propertiesDialog(new CoordinateSystemPropertiesDialog(csController, actions, this))
    , _hovered(false)
{
    setMinimumWidth(mccide::MainToolBar::blockMinimumSize().width() * 2);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(5, 0, 5, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    mccui::ClickableLabel* iconLabel = new mccui::ClickableLabel(QPixmap(":/toolbar-coordinatesystem/resources/globe_passive.png"),
                                                                 QPixmap(":/toolbar-coordinatesystem/resources/globe_active.png"));
    iconLabel->installEventFilter(this);
    mainLayout->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignCenter);

    mainLayout->addWidget(_system, 0, 1);
    _system->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    _system->installEventFilter(this);

    mainLayout->addWidget(_format, 1, 1);
    _format->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    _format->installEventFilter(this);

    iconLabel->setStyleSheet(bgColorSS);
    _system->setStyleSheet(bgColorSS);
    _format->setStyleSheet(bgColorSS);

    connect(_csController.get(), &mccui::CoordinateSystemController::changed, this, &CoordinateSystemWidget::updateSystemAndFormat);

    updateSystemAndFormat();

    _propertiesDialog->hide();
}

CoordinateSystemWidget::~CoordinateSystemWidget()
{}

bool CoordinateSystemWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        _propertiesDialog->show();
        _propertiesDialog->move(mapToGlobal(QPoint(width() - _propertiesDialog->width(),
                                                   height())));

        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void CoordinateSystemWidget::updateSystemAndFormat()
{
    QString formatText;

    const mccui::CoordinateFormat& fmt = _csController->format();

    if (fmt.isAngular()) {
        switch (fmt.unwrapAngular())
        {
        case mccui::AngularFormat::Degrees:
            formatText = "Г";
            break;
        case mccui::AngularFormat::DegreesMinutes:
            formatText = "ГМ";
            break;
        case mccui::AngularFormat::DegreesMinutesSeconds:
            formatText = "ГМС";
            break;
        }
    } else {
        formatText = QString(fmt.unwrapLinear()).toUpper();
    }

    const QString& systemText = _csController->currentSystem().shortName();

    _system->setText(systemText);
    _format->setText(formatText);

    _propertiesDialog->updateSystemAndFormat();
}

bool CoordinateSystemWidget::event(QEvent* event)
{
    if(event->type() == QEvent::HoverEnter)
    {
        _hovered = true;
        update();
    }
    else if(event->type() == QEvent::HoverLeave)
    {
        _hovered = false;
        update();
    }

    return QWidget::event(event);
}

void CoordinateSystemWidget::paintEvent(QPaintEvent* event)
{
    if(_hovered)
    {
        QPainter painter(this);
        painter.setPen(QColor());
        painter.setBrush(mccide::MainToolBar::hoveredBackgroundColor());
        painter.drawRect(rect());
    }

    QWidget::paintEvent(event);
}
