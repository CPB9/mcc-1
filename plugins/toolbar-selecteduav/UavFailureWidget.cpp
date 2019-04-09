#include "UavFailureWidget.h"

#include <QLabel>
#include <QHBoxLayout>

constexpr int iconSize = 32;
constexpr int margin = 3;

UavFailureWidget::UavFailureWidget(QWidget* separator, QWidget *parent)
    : AbstractUavWidget(separator, parent)
    , _image(new QLabel())
    , _text()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, margin, 0);
    layout->setSpacing(3);

    layout->addWidget(_image);

    _image->setFixedSize(iconSize, iconSize);
    _image->setPixmap(QPixmap::fromImage(QImage(":/toolbar-selecteddevice/resources/attention_icon.png").
                                         scaled(QSize(iconSize, iconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    _image->setStyleSheet("background-color: transparent;");

    setMinimumWidth(iconSize + margin);
}

UavFailureWidget::~UavFailureWidget()
{}

bool UavFailureWidget::mayToShow() const
{
    return !_text.isEmpty();
}

void UavFailureWidget::setText(const QString& text)
{
    if(text == _text)
        return;

    _text = text;
    _image->setToolTip(_text);
}
