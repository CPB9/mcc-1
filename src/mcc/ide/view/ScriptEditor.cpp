#include "mcc/ide/view/ScriptEditor.h"
#include "mcc/ide/view/QmlController.h"

#include <QQmlEngine>
#include <QQmlContext>

namespace mccide {

ScriptEditor::ScriptEditor(QWidget* parent /*= 0*/)
{
    setObjectName("ScriptEditor");
    setWindowTitle("Сценарий");

    _context = new QmlController();
    this->engine()->rootContext()->setContextProperty("context", _context);

    this->setSource(QUrl("qrc:///qml/ScriptEditor.qml"));
}
}

