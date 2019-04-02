#pragma once

#include "AbstractUavWidget.h"

class QLabel;

class UavModeWidget : public AbstractUavWidget
{
    Q_OBJECT
public:
    explicit UavModeWidget(QWidget* separator = nullptr, QWidget *parent = nullptr);
    ~UavModeWidget() override;

    void setMode(const QString& mode);
    void setSubmode(const QString& submode);

    bool mayToShow() const override;

private:
    QLabel*     _modeLabel;
    QLabel*     _submodeLabel;

    QString     _modeText;
    QString     _submodeText;

    Q_DISABLE_COPY(UavModeWidget)
};
