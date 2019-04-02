#pragma once

#include "AbstractUavWidget.h"

class QLabel;

class UavFailureWidget : public AbstractUavWidget
{
    Q_OBJECT
public:
    explicit UavFailureWidget(QWidget* separator, QWidget *parent = nullptr);
    ~UavFailureWidget() override;

    bool mayToShow() const override;

    void setText(const QString& text);

private:
    QLabel*     _image;
    QString     _text;

    Q_DISABLE_COPY(UavFailureWidget)
};

