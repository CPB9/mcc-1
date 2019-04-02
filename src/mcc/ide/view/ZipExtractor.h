#pragma once

#include "mcc/Config.h"

#include <QString>

namespace mccide {

class MCC_IDE_DECLSPEC ZipExtractor
{
public:
    ZipExtractor(const QString& filename);
    ~ZipExtractor();

    bool extractTo(const QString& targetFolder, const QString& newName = QString());

    static QString uiFolderPath();
private:
    QString _filename;
};

}

