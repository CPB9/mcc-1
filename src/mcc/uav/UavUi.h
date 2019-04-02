#pragma once

#include "mcc/qml/DeviceUiWidget.h"
#include "mcc/ui/QObjectRefCountable.h"

#include <bmcl/Sha3.h>
#include <bmcl/SharedBytes.h>
#include <vector>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTemporaryDir>

namespace mccuav {

class MCC_UAV_DECLSPEC UiFileInfo : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
public:
    using HashType = bmcl::Sha3<512>;
    using Hash = std::string;

    UiFileInfo();
    ~UiFileInfo();
    void set(const QString& mainFilePath, const std::vector<QString>& files);
    void initWatcher();
    const std::vector<QString>& files() const;
    QString mainPath() const;
    QString rootDir() const;
    Hash hash() const;
signals:
    void hashChanged();
private:
    void updateHash();
    QFileInfo _fileInfo;
    std::vector<QString> _files;
    QFileSystemWatcher _watcher;
    Hash _hash;
};

class MCC_UAV_DECLSPEC UavUi : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
public:
    enum class Type {
        Onboard,
        LocalCopy,
        Custom
    };

    UavUi(const QTemporaryDir& dir, const QString& name);
    ~UavUi();

    bool extractAndValidate(const QString& name, const bmcl::SharedBytes& data);
    bool hasLocalChanges() const;
    bool localCopyExists() const;
    bool isOnboard() const;
    void createLocalCopy();
    void setupLocalCopy();
    QString originPath() const;
    QString localPath() const;
    Type currentType() const;
    void setType(UavUi::Type type);
    void switchToLocalCopy();
    void switchToOnboard();
    QString localCopyDir() const;
signals:
    void typeChanged();
    void localHashChanged();
private:
    const QTemporaryDir&        _dir;
    QString                     _name;
    bmcl::Rc<UiFileInfo>        _originUi;
    bmcl::OptionRc<UiFileInfo>  _localUi;
    Type                        _currentType;
};

}

