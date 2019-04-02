#pragma once

#include <QtGlobal>
#include <Qt>

constexpr const int   maxNameSize = 12;
constexpr const qreal maxFontSize = 9.0;
constexpr const int   buttonsSize = 26;

enum ListRoles
{
    NumberRole = Qt::UserRole + 1,
    ActiveRole,
    EditModeRole,
    NextSpecialRole
};

