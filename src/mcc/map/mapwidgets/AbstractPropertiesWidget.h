#pragma once

#include "mcc/Config.h"
#include "mcc/map/UserWidget.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"

#include <map>

class QFrame;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QVBoxLayout;

class MCC_MAP_DECLSPEC AbstractPropertiesWidget : public mccmap::UserWidget
{
    Q_OBJECT

public:
    explicit AbstractPropertiesWidget(QWidget *parent = nullptr);

    bool isEditMode() const {return _isEditMode;}
    bool isPinnedMode() const {return _isPinnedMode;}

    double cornerRadius() const {return _cornerRadius;}
    void setCornerRadius(double radius);

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const {return _alignment;}

signals:
    void editingCanceled();
    void hoveringCanceled();

public slots:
    virtual void acceptHovering(const QRect& itemRect, const QRect& listRect);
    virtual void cancelHovering();
    virtual void setEditMode(bool editMode = true);
    virtual void setPinnedMode(bool pinnedMode = true);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    virtual void updateEditMode() {}
    virtual void setConnections() {}
    virtual void unsetConnections() {}
    virtual void updatePinnedMode() {}
    virtual void fillInfo() = 0;

    QHBoxLayout* toolsLayout() const {return _toolsLayout;}
    QGridLayout* optionsLayout() const {return _optionsLayout;}
    QGridLayout* infoLayout() const {return _infoLayout;}

    void addOptionWidget(QWidget* widget, const QString& labelText);
    void addOptionWidget(QWidget* widget, QLabel* labelWidget);
    void addOptionWidget(QWidget* widget, QWidget* labelWidget);
    void addInfoWidget(QWidget* widget, const QString& labelText, Qt::Orientation orientation = Qt::Horizontal);
    void addOptionLine(QFrame* line);
    void addInfoLine(QFrame* line);

    // Doesn't remove oldWidget
    void replaceOptionWidget(QWidget* oldWidget, QWidget* newWidget);
    void replaceInfoWidget(QWidget* oldWidget, QWidget* newWidget);

    void removeWidget(QWidget* widget);

    void setFieldValue(QLabel* label, const QString& text, const QColor& color = QColor());
    void setFieldValue(QLabel* label, const QString& text, const QString& labelText, const QColor& color = QColor());

    void setWidgetVisible(QWidget* widget, bool visible);
    void setWidgetColor(QWidget* widget, const QColor& color);
    QLabel* widgetLabel(QWidget* widget) const;
    QWidget* widgetLabelContainer(QWidget* widget) const;

    mccui::ClickableLabel* nameLabel() const {return _nameLabel;}
    const QRect& lastListView() const {return _lastListView;}
    const QRect& lastItem() const {return _lastItem;}

private:
    void setClickableLabelText(QLabel* label, const QString& text);
    void changePosition();

    void addWidgetToGrid(QGridLayout* layout, QWidget* widget, const QString& labelText, Qt::Orientation orientation = Qt::Horizontal);
    void addWidgetToGrid(QGridLayout* layout, QWidget* widget, QWidget* labelWidget, Qt::Orientation orientation = Qt::Horizontal);
    void addLineToGrid(QGridLayout* layout, QFrame* line);
    void replaceWidgetInGrid(QGridLayout* layout, QWidget* oldWidget, QWidget* newWidget);

    bool                            _isEditMode;
    bool                            _isPinnedMode;
    QVBoxLayout*                    _mainLayout;
    QHBoxLayout*                    _toolsLayout;
    QGridLayout*                    _optionsLayout;
    QGridLayout*                    _infoLayout;
    QFrame*                         _hLine;
    mccui::ClickableLabel*          _nameLabel;

    std::map<QWidget*, QWidget*>    _labels;

    mccui::ClickableLabel*          _closeButton;
    mccui::ClickableLabel*          _pinButton;

    Qt::Alignment                   _alignment;
    QRect                           _lastListView;
    QRect                           _lastItem;

    double                          _cornerRadius;

    Q_DISABLE_COPY(AbstractPropertiesWidget)
};
