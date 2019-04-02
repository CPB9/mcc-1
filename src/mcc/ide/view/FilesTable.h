#pragma once

#include "mcc/Config.h"
#include "mcc/ide/view/FileTransferWrapper.h" //TODO: отвязаться
#include <QWidget>
#include <QVector>
#include <QString>

class QProgressBar;
class QTreeView;
class QComboBox;
class QTreeView;
class QTreeWidget;

namespace mcc {
namespace ui {
namespace exchange { class Service; }
namespace ide {

class VariantButton;

enum class FileDirection
{
    Upload,
    Download
};

struct FileDescriptor
{
    QString exchange;
    QString path;
    QString remote;
    QString device;
    FileDirection direction;
    int           progress;
    bool error;

    FileDescriptor()
    {
        progress = 0;
        error = false;
    }

    FileDescriptor(const QString& device, const QString& path, const QString& remote, FileDirection dir)
    {
        this->device = device;
        this->path = path;
        this->remote = remote;
        this->direction = dir;
        error = false;
    }
};

class FileTransferWrapper;

class MCC_IDE_DECLSPEC FilesTableWidget : public QWidget
{
    Q_OBJECT

    struct Helper
    {
        QProgressBar* bar;
        VariantButton* cancelButton;
        VariantButton* removeButton;
    };

public:
    FilesTableWidget(FileTransferWrapper* wrapper, QWidget* parent = 0);

    void addFile(const FileDescriptor& desc);

signals:
    void requestDeviceFileLoad(const QString& service, const QString& device, const QString& filePath, const QString& remote, mcc::ui::ide::FileTransfer::Direction direction);
    void requestDeviceFileLoadCancel(const QString& service, const QString& device, const QString& filePath, const QString& remote, const QString& reason);

private slots:
    void onDeviceAdded(mcc::ui::mccui::FlyingDevice* aircraft);
    void onDeviceFirmwareLoaded(mcc::ui::mccui::FlyingDevice* aircraft);
    void onDeviceRemoved(mcc::ui::mccui::FlyingDevice* aircraft);
    void cancelPressed(const QVariant& data);
    void removePressed(const QVariant& data);
    void uploadFilePressed();
    void downloadFilePressed();
    void onDeviceFileUploaded(const QString& device, const QString& filePath);
    void onDeviceFileUploadFailed(const QString& device, const QString& filePath, const QString& reason);
    void fileUploadProgressChanged(const QString& device, const QString& path, int progress);

private:
    void setupUi();
    int findFile(const QString& exchange, const QString& device, const QString& file);
    void updateFile(const QString& exchange, const QString& device, const std::vector<mcc::ui::ide::FileTransfer>& files);

    QTreeWidget* _filesTree;
    QComboBox*   _devicesList;

    QVector<FileDescriptor> _files;
    QVector<Helper>         _helpers;
};
}
}
}

Q_DECLARE_METATYPE(mcc::ui::ide::FileDescriptor);
