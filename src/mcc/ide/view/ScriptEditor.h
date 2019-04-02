#pragma once

#include <QQuickWidget>

namespace mccide {

class QmlController;

class ScriptEditor : public QQuickWidget
{
public:
    ScriptEditor(QWidget* parent = 0);

private:
    QmlController* _context;
};

}

