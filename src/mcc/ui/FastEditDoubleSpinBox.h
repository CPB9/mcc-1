#pragma once

#include "mcc/Config.h"

#include <QDoubleSpinBox>

namespace mccui {

class MCC_UI_DECLSPEC FastEditDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit FastEditDoubleSpinBox(QWidget *parent = nullptr);
    ~FastEditDoubleSpinBox() override;

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool _fieldIsSelected; // hasSelectedText works wrong for lineEdit in this case

    Q_DISABLE_COPY(FastEditDoubleSpinBox)
};
}
