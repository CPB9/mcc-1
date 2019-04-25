#include "UavUi.h"
#include "mcc/ui/FileUtils.h"
#include "mcc/path/Paths.h"
#include "kml/base/zip_file.h"

#include <bmcl/Sha3.h>
#include <bmcl/FixedArrayView.h>
#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>
#include <bmcl/String.h>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>

namespace mccuav {

UavUi::UavUi(const QTemporaryDir& tempDir, const QString& name)
    : _dir(tempDir), _name(name)
{
}

UavUi::~UavUi()
{

}

bmcl::Result<bool, UiExtractError> UavUi::extractAndValidate(const QString& name, const bmcl::SharedBytes& data)
{
    _name = name;
    using kmlbase::ZipFile;
    using kmlbase::StringVector;
    ZipFile* f = ZipFile::OpenFromString(std::string((const char*)data.data(), data.size()));
    if (!f)
    {
        BMCL_CRITICAL() << "Ошибка при открытии Zip-архива!! ";
        return UiExtractError::BadZipArchive;
    }

    std::vector<std::string> files;
    if (!f->GetToc(&files))
    {
        BMCL_CRITICAL() << "FAILED: f->GetToc(&subFiles) ";
        return UiExtractError::BrokenZipArchive;
    }

    std::sort(files.begin(), files.end());

    QString archivePath = _dir.path() + "/" + _name;
    QDir dir;
    if (dir.exists(archivePath))
        dir.remove(archivePath);

    std::vector<QString> originFiles;
    QFileInfo mainFile;
    for (const auto& fileName : files)
    {
        std::string data;
        if (!f->GetEntry(fileName, &data))
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
        if (fileName == "main.qml")
        {
            mainFile = fileInfo;
        }
        file.write(data.data(), data.size());
        file.close();

        originFiles.emplace_back(QString::fromStdString(fileName));
    }

    if (!mainFile.exists())
    {
        BMCL_CRITICAL() << "Не найден main.qml!!";
        return UiExtractError::MainFileNotFound;
    }
    _originUi = bmcl::makeRc<UiFileInfo>();
    _originUi->set(mainFile.absoluteFilePath(), originFiles);
    if(QFile::exists(localCopyDir()))
    {
        setupLocalCopy();
    }
    return true;
}

bool UavUi::hasLocalChanges() const
{
    return (_localUi.isNone() || _localUi->hash() != _originUi->hash());
}

bool UavUi::localCopyExists() const
{
    return _localUi.isSome();
}

bool UavUi::isOnboard() const
{
    return _currentType == UavUi::Type::Onboard;
}

bmcl::Result<bool, QString> UavUi::createLocalCopy()
{
    auto res = mccui::copyRecursively(_originUi->rootDir(), localCopyDir());
    if(res.isOk())
        setupLocalCopy();
    return res;
}

void UavUi::setupLocalCopy()
{
    _localUi = bmcl::makeRc<UiFileInfo>();
    _localUi->set(localCopyDir() + "/main.qml", _originUi->files());
    _localUi->initWatcher();
    connect(_localUi.unwrap().get(), &UiFileInfo::hashChanged, this, &UavUi::localHashChanged);
}

QString UavUi::originPath() const
{
    return _originUi->mainPath();
}

QString UavUi::localPath() const
{
    return _localUi->mainPath();
}

UavUi::Type UavUi::currentType() const
{
    return _currentType;
}

void UavUi::setType(UavUi::Type type)
{
    _currentType = type;
}

void UavUi::switchToLocalCopy()
{
    assert(_localUi.isSome());
    _currentType = UavUi::Type::LocalCopy;
    emit typeChanged();
}

void UavUi::switchToOnboard()
{
    _currentType = UavUi::Type::Onboard;
    emit typeChanged();
}

QString UavUi::localCopyDir() const
{
    return mccpath::qGetUiPath() + "/" + _name;
}

QString UavUi::name() const
{
    return _name;
}

void UiFileInfo::set(const QString& mainFilePath, const std::vector<QString>& files)
{
    _fileInfo.setFile(mainFilePath);
    _files = files;
    updateHash();
}

void UiFileInfo::initWatcher()
{
    QString rootDir = _fileInfo.absolutePath();
    QStringList watchFiles;
    for (const auto& f : files())
    {
        watchFiles.append(rootDir + "/" + f);
    }
    _watcher.addPaths(watchFiles);
    disconnect(&_watcher);
    connect(&_watcher, &QFileSystemWatcher::fileChanged, this, &UiFileInfo::updateHash);
}

const std::vector<QString>& UiFileInfo::files() const
{
    return _files;
}

UiFileInfo::UiFileInfo()
{

}

UiFileInfo::~UiFileInfo()
{

}

QString UiFileInfo::mainPath() const
{
    return _fileInfo.absoluteFilePath();
}

QString UiFileInfo::rootDir() const
{
    return _fileInfo.absolutePath();
}

UiFileInfo::Hash UiFileInfo::hash() const
{
    return _hash;
}

void UiFileInfo::updateHash()
{
    HashType hasher;
    for (const auto& fileName : _files)
    {
        QFileInfo fileInfo(_fileInfo.absoluteDir(), fileName);
        QFile f(fileInfo.absoluteFilePath());
        if (!f.open(QIODevice::OpenModeFlag::ReadOnly))
        {
            BMCL_CRITICAL() << "Failed to update hash!";
            return;
        }
        QByteArray data = f.readAll();
        hasher.update(data.data(), data.size());
    }
    auto hash = hasher.finalize();
    _hash = bmcl::bytesToHexStringLower(hash.data(), hash.size());
    emit hashChanged();
}

}

