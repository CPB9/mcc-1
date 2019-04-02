#include "mcc/ui/WidgetUtils.h"

#include <QApplication>
#include <QWidget>
#include <QFileInfo>
#include <QProcess>
#include <QDir>

#include <bmcl/Option.h>

namespace mccui {

QWidget* findMainWindow()
{
     // hack for QTBUG-49508
    QWidget* mainWindowWidget = QApplication::activeWindow();
    if(mainWindowWidget == nullptr)
    {
        for(auto w : QApplication::topLevelWidgets())
        {
            if(w->objectName() == "mccide::MainWindow")
            {
                mainWindowWidget = w;
                break;
            }
        }
    }

    return mainWindowWidget;
}

void showInGraphicalShell(const QString &pathIn)
{
    const QFileInfo fileInfo(pathIn);
#ifdef Q_OS_WIN
    QString explorer = "explorer";
    QStringList param;
    if (!fileInfo.isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
    QProcess::startDetached(explorer, param);
#endif
}
}
