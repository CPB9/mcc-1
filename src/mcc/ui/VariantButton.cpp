#include "mcc/ui/VariantButton.h"

#include <QPushButton>
#include <QVariant>

namespace mccui {

VariantButton::VariantButton(const QString& text, const QVariant& userData, QWidget* parent)
    : QPushButton(text, parent)
{
    _userData = userData;

    connect(this, &QPushButton::pressed, this, [this]() {emit pressedWithData(_userData); });
}

VariantButton::~VariantButton()
{
}

void VariantButton::setData(const QVariant& data)
{
    _userData = data;
}
}
