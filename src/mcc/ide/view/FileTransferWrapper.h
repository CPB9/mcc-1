#pragma once

#include <QObject>

#include <map>
#include <string>

#include "mcc/Config.h"
#include "mcc/msg/obj/Declarations.h"

class QString;

namespace mcc {
namespace ui {
namespace exchange { class Service; }
class UavController;
namespace ide {

struct FileTransfer
{
    enum class Direction //TODO: вынести
    {
        Up,
        Down
    };

    FileTransfer() {}
    FileTransfer(const std::string& file_path, std::size_t fileSizeBytes, std::size_t uploadedBytes, Direction direction)
        : _direction(direction), _file_path(file_path), _fileSizeBytes(fileSizeBytes), _uploadedBytes(uploadedBytes) {}
    inline double progress() const
    {
        if (_fileSizeBytes == 0)
            return 0;
        return 100.0 * (double)_uploadedBytes / (double)_fileSizeBytes;
    }
    Direction   _direction;
    std::string _file_path;
    std::size_t _fileSizeBytes;
    std::size_t _uploadedBytes;
};

class MCC_IDE_DECLSPEC FileTransferWrapper : public QObject
{
    Q_OBJECT

public:
    FileTransferWrapper(QObject* parent = 0);

public slots:
    void onDeviceFileLoad(const QString& service, const QString& device, const QString& localPath, const QString& remotePath, mcc::ui::ide::FileTransfer::Direction direction);
    void onDeviceFileLoadCancel(const QString& service, const QString& device, const QString& localPath, const QString& remotePath, const QString& reason);

private slots:
    void cmdState(const mcc::messages::cmd::StatePtr& state);

signals:
    void deviceFileLoaded(const QString& device, const QString& filePath);
    void deviceFileLoadFailed(const QString& device, const QString& filePath, const QString& reason);
    void fileUploadProgressChanged(const QString& device, const QString& path, int progress);

private:
    std::string generateKey(const QString& device, const QString& local, const QString& remote) const;

private:
    std::map<std::string, mcc::messages::cmd::ParamListPtr> _fileCmds;
};
}
}
}

