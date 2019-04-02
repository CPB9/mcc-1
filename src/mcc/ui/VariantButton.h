#pragma once

#include "mcc/Config.h"

#include <QPushButton>
#include <QVariant>

class QString;

namespace mccui {

class MCC_UI_DECLSPEC VariantButton : public QPushButton
{
    Q_OBJECT
public:
    VariantButton(const QString& text, const QVariant& userData, QWidget* parent = 0);
    ~VariantButton();

    void setData(const QVariant& data);

signals:
    void pressedWithData(const QVariant& userData);

private:
    QVariant _userData;
};
}
