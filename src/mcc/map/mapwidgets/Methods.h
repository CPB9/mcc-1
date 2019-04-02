#pragma once

#include "mcc/Config.h"
#include "mcc/map/mapwidgets/Types.h"

#include <QString>
#include <QColor>

class QPixmap;

QString MCC_MAP_DECLSPEC coloredTitle(const QString& channelName, bool activated);
QString MCC_MAP_DECLSPEC coloredLabelText(const QString& name, uint color);

const QColor MCC_MAP_DECLSPEC &backgroundColorCommon(bool hovered = false);
const QColor MCC_MAP_DECLSPEC &backgroundColorSelected(bool hovered = false);
const QColor MCC_MAP_DECLSPEC &backgroundColorEditing(bool hovered = false);

QPixmap MCC_MAP_DECLSPEC scrollAmountPixmap(const QPixmap& original, int value);
