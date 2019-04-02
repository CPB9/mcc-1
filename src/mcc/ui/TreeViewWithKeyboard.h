#pragma once

#include "mcc/Config.h"

#include <QTreeView>

namespace mccui {

class MCC_UI_DECLSPEC TreeViewWithKeyboard : public QTreeView
{
public:
    TreeViewWithKeyboard(QWidget* parent = 0);

    void keyPressEvent(QKeyEvent* event) override;
};
}
