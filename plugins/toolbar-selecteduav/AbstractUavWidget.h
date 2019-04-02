#pragma once

#include <QWidget>

class AbstractUavWidget : public QWidget
{
public:
    explicit AbstractUavWidget(QWidget* separator = nullptr, QWidget *parent = nullptr);
    ~AbstractUavWidget() override;

    void setSeparator(QWidget* separator);
    QWidget* separator() {return _separator;}

    virtual bool mayToShow() const = 0;

private:
    QWidget*    _separator;

    Q_DISABLE_COPY(AbstractUavWidget)
};
