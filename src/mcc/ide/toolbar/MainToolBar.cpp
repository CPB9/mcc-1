#include "mcc/ide/toolbar/MainToolBar.h"

#include "mcc/ui/ClickableLabel.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QImage>
#include <QMenu>
#include <QMouseEvent>
#include <QPalette>
#include <QPainter>

namespace mccide {

MainToolBar::MainToolBar(QWidget* parent)
    : QWidget (parent)
    , _mainLayout(new QHBoxLayout(this))
    , _mainMenu(new QMenu(this))
    , _menuLabel(new mccui::ClickableLabel(QPixmap::fromImage(QImage(":/toolbar/main_menu_passive.png").scaled(blockMinimumSize())),
                                           QPixmap::fromImage(QImage(":/toolbar/main_menu_active.png").scaled(blockMinimumSize()))))
{
    setMinimumHeight(blockMinimumSize().height());

    setStyleSheet(QString(
        "QWidget\n"
        "{\n"
        "	color: #909090;\n" // fafafa
        "	background-color: #404040;\n"
        "}\n\n"
//    ));
       "QWidget:hover\n"
       "{\n"
       "	color: #ffffff;\n"
       "	background-color: #202020;\n"
       "}\n\n"));

    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->setSpacing(0);
    setLayout(_mainLayout);

    _menuLabel->setMinimumSize(blockMinimumSize());
    _menuLabel->setMaximumSize(_menuLabel->minimumSize());
    _mainLayout->addWidget(_menuLabel);

    connect(_menuLabel, &mccui::ClickableLabel::clicked, this,
            [this]()
    {
        _mainMenu->exec(_menuLabel->mapToGlobal(QPoint(0, _menuLabel->height())));
    });
}

MainToolBar::~MainToolBar()
{}

bool MainToolBar::eventFilter(QObject* watched, QEvent* event)
{
    std::map<QObject*, QFrame*>::iterator line = _lines.find(watched);
    if(line != _lines.end())
    {
        if(event->type() == QEvent::Show)
            line->second->show();
        else if(event->type() == QEvent::Hide)
            line->second->hide();
    }

    return QWidget::eventFilter(watched, event);
}

void MainToolBar::addUserWidget(QWidget* widget, bool left)
{
    int h = blockMinimumSize().height();
    widget->setMinimumHeight(h);
    int w = blockMinimumSize().width();
    if(widget->minimumWidth() < w)
        widget->setMinimumWidth(w);

    widget->setMaximumHeight(h);

    widget->installEventFilter(this);

    QFrame* line = addLine();
    if(!left)
        _mainLayout->addWidget(line);
    _mainLayout->addWidget(widget);
    if(left)
        _mainLayout->addWidget(line);
    _lines[widget] = line;

    if(widget->isHidden())
        line->hide();
}

void MainToolBar::addStretch(int stretch)
{
    _mainLayout->addStretch(stretch);
}

void MainToolBar::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.fillRect(rect(), QBrush(mainBackgroundColor()));

    QWidget::paintEvent(event);
}

QFrame*MainToolBar::addLine()
{
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}
}
