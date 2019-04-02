#pragma once

#include "mcc/ui/QProgressIndicator.h"

namespace mccui{

class MCC_UI_DECLSPEC TextualProgressIndicator : public QProgressIndicator
{
    Q_OBJECT
public:
    explicit TextualProgressIndicator(QWidget *parent = nullptr);
    ~TextualProgressIndicator() override;

    QString text() const {return _text;}
    void setText(const QString& text);

    QColor textColor() const { return _textColor; }
    void setTextColor(const QColor& textColor);

    void setColor(const QColor & color) override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void adjustTextSize();

    QString _text;
    QColor  _textColor;

    Q_DISABLE_COPY(TextualProgressIndicator)
};
}
