#include "AbstractUavWidget.h"

AbstractUavWidget::AbstractUavWidget(QWidget* separator, QWidget *parent)
    : QWidget(parent)
    , _separator(separator)
{}

AbstractUavWidget::~AbstractUavWidget()
{}

void AbstractUavWidget::setSeparator(QWidget* separator)
{
    if(separator == _separator)
        return;

    _separator = separator;

    if(_separator != nullptr)
        _separator->setVisible(isVisible());
}
