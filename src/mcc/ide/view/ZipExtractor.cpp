#include "mcc/ide/view/ZipExtractor.h"

#include "kml/base/zip_file.h"

#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

namespace mccide {

ZipExtractor::ZipExtractor(const QString& filename)
    : _filename(filename)
{
}

ZipExtractor::~ZipExtractor()
{
}

bool ZipExtractor::extractTo(const QString& targetFolder, const QString& newName)
{
    kmlbase::ZipFile* f = kmlbase::ZipFile::OpenFromFile(_filename.toStdString().c_str());

    if (f == nullptr)
    {
        qDebug() << "kmlbase::ZipFile::OpenFromFile(" << _filename << ") failed";
        return false;
    }

    kmlbase::StringVector subfiles;

    if (!f->GetToc(&subfiles))
    {
        qDebug() << "f->GetToc(&subfiles) " << _filename << " failed";
        return false;
    }

    QFileInfo archiveName(_filename);

    QString archivePath;
    if (newName.isEmpty())
        archivePath = targetFolder + "/" + archiveName.baseName();
    else
        archivePath = targetFolder + "/" + newName;

    QDir dir;
    if (dir.exists(archivePath))
        dir.remove(archivePath);

    for (auto fileName : subfiles)
    {
        std::string data;
        bool ok = f->GetEntry(fileName, &data);

        if (!ok)
            continue;

        QFileInfo fileInfo(archivePath, QString::fromStdString(fileName));

        if (!dir.mkpath(fileInfo.absolutePath()))
        {
            qDebug() << "Can't create path " << fileInfo.absolutePath();
            continue;
        }

        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Can't open file for writing: " << file.fileName();
            continue;
        }

        file.write(data.data(), data.size());
        file.close();
    }

    return true;
}

QString ZipExtractor::uiFolderPath()
{
    return QDir::tempPath() + "/ui/";
}
}

