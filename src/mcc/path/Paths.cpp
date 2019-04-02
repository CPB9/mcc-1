#include <QCoreApplication>
#include <QStandardPaths>

#include "mcc/path/Paths.h"

namespace mccpath {

std::string getBinPath()
{
    return qGetBinPath().toStdString();
}

std::string getConfigPath()
{
    return qGetConfigPath().toStdString();
}

std::string getDataPath()
{
    return qGetDataPath().toStdString();
}

std::string getLogsPath()
{
    return qGetLogsPath().toStdString();
}

std::string getUiPath()
{
    return qGetUiPath().toStdString();
}

MCC_PATH_DECLSPEC std::string getSoundsPath()
{
    return qGetSoundsPath().toStdString();
}

QString qGetBinPath()
{
    return QCoreApplication::applicationDirPath();
}

QString qGetConfigPath()
{
#ifdef QT_DEBUG
    return qGetBinPath();
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
#endif
}

QString qGetDataPath()
{
#ifdef QT_DEBUG
    return qGetBinPath();
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}

QString qGetLogsPath()
{
    return qGetDataPath() + "/logs";
}

QString qGetUiPath()
{
    return qGetDataPath() + "/ui";
}

MCC_PATH_DECLSPEC QString qGetSoundsPath()
{
    return qGetBinPath() + "/sounds";
}

}
