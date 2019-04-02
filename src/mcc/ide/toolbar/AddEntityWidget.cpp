#include "mcc/ide/toolbar/AddEntityWidget.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/ui/ClickableLabel.h"
#include "mcc/res/Resource.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>

namespace mccide {

AddEntityWidget::AddEntityWidget(const QImage& main, const QImage& hovered, QWidget* parent)
    : QFrame (parent)
    , _iconLabel(new mccui::ClickableLabel(QPixmap::fromImage(main.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
                                           QPixmap::fromImage(hovered.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
                                           this))
    , _textLabel(new QLabel("Добавить новый", this))
{
    setObjectName("AddEntityWidget");
    setStyleSheet("QFrame#AddEntityWidget{"
                  "  background-color: transparent;"
                  "  color: darkgray;"
                  "  border-width: 1px;"
                  "  border-style: dot-dot-dash;"
                  "  border-radius: 2px;"
                  "  border-color: #707070;"
                  "  margin: 3px;"
                  "}"
                  "QFrame#AddVehicleWidget:hover{"
                  "  background-color: #dc000000;"
                  "}");

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(5);

    _iconLabel->setMinimumSize(24, 24);

    _textLabel->setAlignment(Qt::AlignCenter);

    l->addStretch();
    l->addWidget(_iconLabel);
    l->addWidget(_textLabel);
    l->addStretch();

    _iconLabel->setStyleSheet("background-color: transparent;");
    _textLabel->setStyleSheet("background-color: transparent;");

    setMinimumHeight(mccide::MainToolBar::blockMinimumSize().height());
    resize(width(), mccide::MainToolBar::blockMinimumSize().height());

    _iconLabel->installEventFilter(this);
    _iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    _textLabel->installEventFilter(this);

    setMouseTracking(true);
}

AddEntityWidget::AddEntityWidget(QWidget* parent)
    : AddEntityWidget(mccres::loadImage(mccres::ResourceKind::AddButtonPassiveIcon), mccres::loadImage(mccres::ResourceKind::AddButtonIcon), parent)
{}

AddEntityWidget::~AddEntityWidget()
{}

bool AddEntityWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == _iconLabel)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            event->accept();
            emit clicked(reinterpret_cast<QMouseEvent*>(event));
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AddEntityWidget::setIcon(const QImage& main, const QImage& hovered)
{
    _iconLabel->setMainPixmap(QPixmap::fromImage(main.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    if(!hovered.isNull())
        _iconLabel->setHoveredPixmap(QPixmap::fromImage(hovered.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void AddEntityWidget::mousePressEvent(QMouseEvent* event)
{
    event->accept();
    emit clicked(event);
    return;
}

void AddEntityWidget::mouseMoveEvent(QMouseEvent* event)
{
    _iconLabel->forceHover(true);

    QFrame::mouseMoveEvent(event);
}

void AddEntityWidget::leaveEvent(QEvent* event)
{
     _iconLabel->forceHover(false);

    QFrame::leaveEvent(event);
}
}
