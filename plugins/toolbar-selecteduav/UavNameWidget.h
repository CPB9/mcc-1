#pragma once

#include <QWidget>

class QLabel;
class QHBoxLayout;

namespace mccui {
class TextualProgressIndicator;
}

class UavNameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UavNameWidget(QWidget* parent = nullptr);
    ~UavNameWidget() override;

    void setPixmap(const QPixmap& pixmap);
    void setName(const QString& name);
    QString name() const;

    void activateProcess(bool activate = true);
    bool isProcessActivated() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QHBoxLayout*                        _layout;
    QLabel*                             _icon;
    QLabel*                             _nameLabel;
    QString                             _nameText;
    mccui::TextualProgressIndicator*    _processWidget;

    Q_DISABLE_COPY(UavNameWidget)
};
