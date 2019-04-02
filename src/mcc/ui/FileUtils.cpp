#include "mcc/ui/FileUtils.h"

#include <QDir>
#include <QCoreApplication>

namespace mccui {

MCC_UI_DECLSPEC bmcl::Result<bool, QString> copyRecursively(const QString& srcFilePath, const QString& tgtFilePath)
{
    QFileInfo srcFileInfo = QFileInfo(srcFilePath);
    QFileInfo tgtFileInfo = QFileInfo(tgtFilePath);
    if (srcFileInfo.isDir())
    {
        if (!tgtFileInfo.exists())
        {
            QDir targetDir(tgtFilePath);
            targetDir.cdUp();
            if (!targetDir.mkdir(tgtFileInfo.fileName()))
            {
                return QCoreApplication::translate("mccui::FileUtils", "Failed to create directory \"%1\".")
                    .arg(QDir::toNativeSeparators(tgtFilePath));
            }
        }
        QDir sourceDir(srcFileInfo.absoluteFilePath());
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for(const QString &fileName: fileNames)
        {
            QFileInfo newSrcFile(sourceDir, fileName);
            QFileInfo newTgtFile(tgtFilePath, fileName);
            auto result = copyRecursively(newSrcFile.absoluteFilePath(), newTgtFile.absoluteFilePath());
            if(result.isErr())
                return result.takeErr();
        }
    }
    else
    {
        if (!QFile::copy(srcFileInfo.absoluteFilePath(), tgtFileInfo.absoluteFilePath()))
        {
            return QCoreApplication::translate("Utils::FileUtils", "Could not copy file \"%1\" to \"%2\".")
                    .arg(QDir::toNativeSeparators(srcFilePath), QDir::toNativeSeparators(tgtFilePath));
        }
    }
    return true;
}

}