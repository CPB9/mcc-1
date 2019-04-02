#include "mcc/ui/FastEditDoubleSpinBox.h"

#include <QLineEdit>
#include <QEvent>

namespace mccui {

FastEditDoubleSpinBox::FastEditDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox (parent)
    , _fieldIsSelected(false)
{
    lineEdit()->installEventFilter(this);
}

void FastEditDoubleSpinBox::focusInEvent(QFocusEvent* event)
{
    QDoubleSpinBox::focusInEvent(event);

    if(!lineEdit()->hasSelectedText())
    {
        lineEdit()->selectAll();
    }
}

void FastEditDoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
    QDoubleSpinBox::focusOutEvent(event);
    lineEdit()->deselect();
    _fieldIsSelected = false;
}

bool FastEditDoubleSpinBox::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == lineEdit())
    {
        if(event->type() == QEvent::FocusIn ||
           event->type() == QEvent::MouseButtonPress)
        {
            if(!_fieldIsSelected)
            {
                lineEdit()->selectAll();
                _fieldIsSelected = true;
                return true;
            }
        }
    }

    return QDoubleSpinBox::eventFilter(obj, event);
}


FastEditDoubleSpinBox::~FastEditDoubleSpinBox()
{
}
}
