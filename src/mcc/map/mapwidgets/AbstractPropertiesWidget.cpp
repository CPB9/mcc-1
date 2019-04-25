#include "mcc/map/mapwidgets/AbstractPropertiesWidget.h"
#include "mcc/map/mapwidgets/Types.h"

#include "mcc/map/MapWidget.h"
#include "mcc/ui/ClickableLabel.h"
#include "mcc/ui/ColorDialogOptions.h"

#include <QContextMenuEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

AbstractPropertiesWidget::AbstractPropertiesWidget(QWidget* parent)
    : mccmap::UserWidget(parent)
    , _isEditMode(false)
    , _isPinnedMode(false)
    , _mainLayout(new QVBoxLayout(this))
    , _toolsLayout(new QHBoxLayout())
    , _optionsLayout(new QGridLayout())
    , _infoLayout(new QGridLayout())
    , _hLine(new QFrame())
    , _nameLabel(new mccui::ClickableLabel(this))
    , _labels()
    , _closeButton(new mccui::ClickableLabel(QPixmap(":/resources/icons/close_passive.png"),
                                             QPixmap(":/resources/icons/close_active.png"),
                                             this))
    , _pinButton(new mccui::ClickableLabel(QPixmap(":/resources/icons/clip_passive.png"),
                                           QPixmap(":/resources/icons/clip_active.png"),
                                           this))
    , _alignment(Qt::AlignRight)
    , _lastListView(0, 0, 0, 0)
    , _lastItem()
    , _cornerRadius(8)
{
    setObjectName("AbstractPropertiesWidget");

    setMinimumWidth(210);

    // For Unity-theme (toolTips)
    if(mccui::colorDialogOptions() != QColorDialog::DontUseNativeDialog)
        setStyleSheet("color: white;");
    else
        setStyleSheet("QLabel{color:white;}");

//    setStyleSheet("background-color: rgba(0, 0, 0, 153);");

    QFont regularFont;
    regularFont.setFamily("Roboto Condensed");
    regularFont.setStyleName("Regular");
    regularFont.setPointSizeF(maxFontSize);
    setFont(regularFont);

    // Controls
    _mainLayout->addLayout(_toolsLayout);
    _toolsLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    _toolsLayout->addWidget(_closeButton);
    _closeButton->setToolTip("Закончить редактирование");
    connect(_closeButton, &mccui::ClickableLabel::clicked, this, &AbstractPropertiesWidget::editingCanceled);
    _closeButton->hide();

    _toolsLayout->addWidget(_pinButton);
    _pinButton->setToolTip("Прикрепить данный виджет свойств");
    connect(_pinButton, &mccui::ClickableLabel::clicked, this,
            [this]()
    {
        setPinnedMode(!isPinnedMode());
    });
    _pinButton->hide();

    _toolsLayout->addStretch(1);

    _optionsLayout->setVerticalSpacing(1);
    _mainLayout->addLayout(_optionsLayout);

    _hLine->setFrameShape(QFrame::HLine);
    _hLine->setFrameShadow(QFrame::Sunken);
    _mainLayout->addWidget(_hLine);

    _nameLabel->setAlignment(Qt::AlignCenter);
    _nameLabel->setFont(regularFont);
    _nameLabel->setText("Properties");
    _nameLabel->setToolTip("Редактирование названия аппарата");
    _mainLayout->addWidget(_nameLabel);

    _infoLayout->setVerticalSpacing(1);
    _mainLayout->addLayout(_infoLayout);

    _mainLayout->addStretch(1);

    hide();
}

void AbstractPropertiesWidget::setCornerRadius(double radius)
{
    if(_cornerRadius == radius)
        return;

    _cornerRadius = radius;
    update();
}

void AbstractPropertiesWidget::setAlignment(Qt::Alignment alignment)
{
    if(_alignment != alignment)
    {
        _alignment = alignment;
        changePosition();
    }
}

void AbstractPropertiesWidget::setEditMode(bool editMode)
{
    if(editMode == _isEditMode)
        return;

    _isEditMode = editMode;

    _closeButton->setVisible(isEditMode());
    _pinButton->setVisible(isEditMode() || isPinnedMode());

    changePosition();

    updateEditMode();

    if(!isEditMode() && !isPinnedMode())
        cancelHovering();

    update();
}

void AbstractPropertiesWidget::setPinnedMode(bool pinnedMode)
{
    if(pinnedMode == _isPinnedMode)
        return;

    _isPinnedMode = pinnedMode;
    if(isPinnedMode())
        _pinButton->setMainPixmap(QPixmap(":/resources/icons/clip_additional.png"));
    else
        _pinButton->setMainPixmap(QPixmap(":/resources/icons/clip_passive.png"));

    _pinButton->setVisible(isEditMode() || isPinnedMode());

    changePosition();

    updatePinnedMode();

    if(isEditMode())
    {
        emit editingCanceled();
    }
    if(!isPinnedMode())
    {
        cancelHovering();
    }

    update();
}

void AbstractPropertiesWidget::acceptHovering(const QRect& itemRect, const QRect& listRect)
{
    _lastListView = listRect;
    _lastItem = itemRect;

    if(isEditMode())
    {
        emit editingCanceled();
    }

    if(parentWidget() == nullptr)
    {
        return;
    }

    // Widget position
    changePosition();

    // Info data
    setConnections();
    fillInfo();
}

void AbstractPropertiesWidget::cancelHovering()
{
    unsetConnections();
    emit hoveringCanceled();

    hide();
}

void AbstractPropertiesWidget::paintEvent(QPaintEvent* event)
{
    QColor bc(0, 0, 0, 153);
    if(isEditMode())
        bc = QColor(0, 0, 0, 220);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QColor(0, 0, 0, 0));
    painter.setBrush(bc);
    painter.drawRoundedRect(rect(), cornerRadius(), cornerRadius());

    mccmap::UserWidget::paintEvent(event);
}

void AbstractPropertiesWidget::mousePressEvent(QMouseEvent* event)
{
    event->accept();
    return;
}

void AbstractPropertiesWidget::keyPressEvent(QKeyEvent* event)
{
    if(isEditMode())
    {
        if(event->key() == Qt::Key_Escape)
            emit editingCanceled();
    }

    event->accept();
}

void AbstractPropertiesWidget::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
    return;
}

void AbstractPropertiesWidget::addOptionWidget(QWidget* widget, const QString& labelText)
{
    addWidgetToGrid(optionsLayout(), widget, labelText);
}

void AbstractPropertiesWidget::addOptionWidget(QWidget* widget, QLabel* labelWidget)
{
    addWidgetToGrid(optionsLayout(), widget, labelWidget);
}

void AbstractPropertiesWidget::addOptionWidget(QWidget* widget, QWidget* labelWidget)
{
    addWidgetToGrid(optionsLayout(), widget, labelWidget);
}

void AbstractPropertiesWidget::addInfoWidget(QWidget* widget, const QString& labelText, Qt::Orientation orientation)
{
    addWidgetToGrid(infoLayout(), widget, labelText, orientation);
}

void AbstractPropertiesWidget::addOptionLine(QFrame* line)
{
    addLineToGrid(optionsLayout(), line);
}

void AbstractPropertiesWidget::addInfoLine(QFrame* line)
{
    addLineToGrid(infoLayout(), line);
}

void AbstractPropertiesWidget::replaceOptionWidget(QWidget* oldWidget, QWidget* newWidget)
{
    replaceWidgetInGrid(optionsLayout(), oldWidget, newWidget);
}

void AbstractPropertiesWidget::replaceInfoWidget(QWidget* oldWidget, QWidget* newWidget)
{
    replaceWidgetInGrid(infoLayout(), oldWidget, newWidget);
}

void AbstractPropertiesWidget::setFieldValue(QLabel* label, const QString& text, const QColor& color)
{
    bool hasValue = !text.isEmpty();
    if (hasValue)
    {
        setClickableLabelText(label, text);
        setWidgetColor(label, color);
    }

    setWidgetVisible(label, hasValue);
}

void AbstractPropertiesWidget::setFieldValue(QLabel* label, const QString& text, const QString& labelText, const QColor& color)
{
    bool hasValue = !text.isEmpty();
    if (hasValue)
    {
        setClickableLabelText(label, text);
        setWidgetColor(label, color);
    }
    label->setVisible(hasValue);

    QLabel* l = widgetLabel(label);
    if(l != nullptr)
    {
        if(hasValue && l->text() != labelText)
        {
            l->setText(labelText);
        }

        l->setVisible(hasValue);
    }

    //adjustSize();
}

void AbstractPropertiesWidget::setWidgetVisible(QWidget* widget, bool visible)
{
    if(widget == nullptr/* || widget->isVisible() == visible*/)
        return;

    widget->setVisible(visible);
    QWidget* l = widgetLabelContainer(widget);
    if(l != nullptr)
        l->setVisible(visible);

    adjustSize();
}

void AbstractPropertiesWidget::setWidgetColor(QWidget* widget, const QColor& color)
{
    if(widget == nullptr)
        return;

    if(color.isValid())
        widget->setStyleSheet(QString("QLabel{color: #%1;}").arg(color.rgb(), 6, 16, QLatin1Char('0')));
}

QLabel* AbstractPropertiesWidget::widgetLabel(QWidget* widget) const
{
    QWidget* cont = widgetLabelContainer(widget);
    return qobject_cast<QLabel*>(cont);
}

QWidget* AbstractPropertiesWidget::widgetLabelContainer(QWidget* widget) const
{
    if(widget == nullptr)
        return nullptr;

    auto i = _labels.find(widget);
    if (i == _labels.end())
        return nullptr;

    return i->second;
}

void AbstractPropertiesWidget::setClickableLabelText(QLabel* label, const QString& text)
{
    mccui::ClickableLabel *cl = qobject_cast<mccui::ClickableLabel*>(label);
    if(cl != nullptr)
        cl->setHoverableText(text);
    else
        label->setText(text);
}

void AbstractPropertiesWidget::changePosition()
{
    QPoint newPos = mapToParent(mapFromGlobal(_lastListView.topLeft()));
    if(!isPinnedMode() && !_lastItem.isNull())
    {
        newPos = mapToParent(mapFromGlobal(_lastItem.topLeft()));

        if((newPos.y() + height()) > (parentWidget()->height() - mccmap::MapWidget::subWidgetMargin()) &&
           _lastListView.height() < parentWidget()->height() &&
           height() < _lastListView.height())
        {
            QRect listAtMap = _lastListView;
            listAtMap.moveTopLeft(mapToParent(mapFromGlobal(listAtMap.topLeft())));
            newPos.setY(listAtMap.y() + listAtMap.height() - height() - 2);
        }
    }

    if(alignment() == Qt::AlignRight)
        move(newPos.x() + _lastListView.width() + 2 - 20, newPos.y()); // NOTE: hack for group tree widget. Refactor it.
    else
        move(newPos.x() - (width() + 2), newPos.y());
}

void AbstractPropertiesWidget::addWidgetToGrid(QGridLayout* layout, QWidget* widget, const QString& labelText, Qt::Orientation orientation)
{
    if(widget == nullptr || layout == nullptr)
        return;

    addWidgetToGrid(layout, widget, new QLabel(labelText), orientation);
}

void AbstractPropertiesWidget::addWidgetToGrid(QGridLayout* layout, QWidget* widget, QWidget* labelWidget, Qt::Orientation orientation)
{
    if(widget == nullptr || layout == nullptr || labelWidget == nullptr)
        return;

    widget->setFont(font());

    int row = layout->rowCount();
    if (orientation == Qt::Horizontal)
    {
        layout->addWidget(labelWidget, row, 0, Qt::AlignLeft);
        layout->addWidget(widget, row, 1, Qt::AlignRight);
    }
    else if(orientation == Qt::Vertical)
    {
        layout->addWidget(labelWidget, row, 0, 1, 2, Qt::AlignCenter);
        layout->addWidget(widget, row + 1, 0, 1, 2, Qt::AlignLeft);
    }
    labelWidget->setStyleSheet("QLabel{color: lightgray;}");
    labelWidget->setFont(font());

    widget->hide();
    labelWidget->hide();

    auto i = _labels.find(widget);
    if (i != _labels.end())
    {
        delete i->second;
        i->second = labelWidget;
    }
    else
        _labels.emplace(widget, labelWidget);
}

void AbstractPropertiesWidget::addLineToGrid(QGridLayout* layout, QFrame* line)
{
    if(line == nullptr || layout == nullptr)
        return;

    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("QFrame[frameShape=\"4\"]" // QFrame::HLine == 0x0004
                        "{color: darkgray;}");
    layout->addWidget(line, layout->rowCount(), 0, 1, 2);
}

void AbstractPropertiesWidget::replaceWidgetInGrid(QGridLayout* layout, QWidget* oldWidget, QWidget* newWidget)
{
    if(layout == nullptr || oldWidget == nullptr || newWidget == nullptr)
        return;

    newWidget->setFont(font());

    layout->replaceWidget(oldWidget, newWidget);
    adjustSize();

    auto i = _labels.find(oldWidget);
    if (i != _labels.end())
    {
        QWidget* label(i->second);
        _labels.erase(i);

        _labels.emplace(newWidget, label);
    }

}

void AbstractPropertiesWidget::removeWidget(QWidget* widget)
{
    if(widget == nullptr)
        return;

    auto i = _labels.find(widget);
    if(i != _labels.end())
    {
        delete i->second;
        _labels.erase(i);
    }

    delete widget;
}
