#include "UavNameWidget.h"

#include "mcc/ui/TextualProgressIndicator.h"
#include "mcc/ui/TextUtils.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontMetrics>

constexpr int horizontalMargin = 3;
constexpr int maxTextSize = 10;
constexpr int iconSize = 32;

UavNameWidget::UavNameWidget(QWidget* parent)
    : QWidget (parent)
    , _layout(new QHBoxLayout(this))
    , _icon(new QLabel())
    , _nameLabel(new QLabel("Unknown"))
{
    _layout->setContentsMargins(0, 0, horizontalMargin, 0);
    _layout->setSpacing(horizontalMargin);

    _layout->addWidget(_icon);
    _layout->addWidget(_nameLabel);

    _processWidget = new mccui::TextualProgressIndicator(this);
    _processWidget->setColor(QColor(Qt::white));
    _processWidget->setFixedSize(iconSize, iconSize);
    _processWidget->setAnimationDelay(75);
    _processWidget->startAnimation();
    _processWidget->hide();

    _icon->setAlignment(Qt::AlignCenter);
    _icon->setStyleSheet("background-color: transparent;");
    _icon->setFixedSize(iconSize, iconSize);
    _nameLabel->setStyleSheet("background-color: transparent;");

    QString testLine(maxTextSize, 'x');
    QFontMetrics fm(font());
    int minW = fm.boundingRect(testLine).width();
    _nameLabel->setFixedWidth(minW);

//    _nameLabel->setMinimumWidth(iconSize + minW + horizontalMargin * 2);
}

UavNameWidget::~UavNameWidget()
{}

void UavNameWidget::setPixmap(const QPixmap& pixmap)
{
    if(_icon->pixmap() != nullptr)
    {
        if(_icon->pixmap()->cacheKey() == pixmap.cacheKey())
            return;
    }

    _icon->setPixmap(pixmap);
}

void UavNameWidget::setName(const QString& name)
{
    if(_nameText == name)
        return;

    _nameText = name;
    QString shortName;
    QFontMetrics fm(font());
    for(int i = maxTextSize; i > 1; --i)
    {
        shortName = mccui::shortTextLine(name, i);
        if(fm.boundingRect(shortName).width() <= _nameLabel->width())
            break;
    }

    _nameLabel->setText(shortName);
    if(_nameText.size() > shortName.size())
        _nameLabel->setToolTip(_nameText);
    else
        _nameLabel->setToolTip(QString());
}

QString UavNameWidget::name() const
{
    return _nameText;
}

void UavNameWidget::activateProcess(bool activate)
{
    _processWidget->setVisible(activate);
}

bool UavNameWidget::isProcessActivated() const
{
    return _processWidget->isVisible();
}

void UavNameWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    _processWidget->move(_icon->x() + 1, _icon->y() + 1);
}

