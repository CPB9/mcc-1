#include "mcc/ide/view/FilesTable.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QVector>
#include <QProgressBar>
#include <QFileDialog>
#include <QMetaObject>
#include <QInputDialog>
#include <QHeaderView>
#include <QComboBox>
#include <QFileInfo>

#include "mcc/uav/UavController.h"
#include "mcc/ide/view/VariantButton.h"

#include "mcc/ide/view/FileTransferWrapper.h"

namespace mccide {

FilesTableWidget::FilesTableWidget(FileTransferWrapper* wrapper, QWidget* parent)
    : QWidget(parent)
{
    using mcc::ui::mccuav::UavController;

    setupUi();

    setObjectName("FilesTable");
    setWindowTitle("Менеджер файлов");

    connect(wrapper, &FileTransferWrapper::deviceFileLoaded,            this, &FilesTableWidget::onDeviceFileUploaded);
    connect(wrapper, &FileTransferWrapper::deviceFileLoadFailed,        this, &FilesTableWidget::onDeviceFileUploadFailed);
    connect(wrapper, &FileTransferWrapper::fileUploadProgressChanged,   this, &FilesTableWidget::fileUploadProgressChanged);

    connect(this, &FilesTableWidget::requestDeviceFileLoad, wrapper, &FileTransferWrapper::onDeviceFileLoad);
    connect(this, &FilesTableWidget::requestDeviceFileLoadCancel, wrapper, &FileTransferWrapper::onDeviceFileLoadCancel);

    auto manager = mccui::Context::instance()->uavController();

    connect(manager, &UavController::deviceAdded, this, &FilesTableWidget::onDeviceAdded);
    connect(manager, &UavController::deviceRemoved, this, &FilesTableWidget::onDeviceRemoved);
    connect(manager, &UavController::deviceFirmwareLoaded, this, &FilesTableWidget::onDeviceFirmwareLoaded);
}

void FilesTableWidget::addFile(const FileDescriptor& desc)
{
    int idx = findFile("", desc.device, desc.path);
    if (idx != -1)
    {
        QTreeWidgetItem* item = _filesTree->topLevelItem(idx);
        item->setText(2, "Загружается");

        _helpers[idx].bar->setTextVisible(true);
        _helpers[idx].bar->setValue(0);
        _helpers[idx].cancelButton->setVisible(true);
        _helpers[idx].removeButton->setVisible(false);
        _helpers[idx].cancelButton->setData(QVariant::fromValue(desc));
        _files[idx] = desc;

        return;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();

    QFileInfo info(desc.path);
    item->setText(0, info.fileName());
    item->setData(0, Qt::UserRole, desc.path);

    auto dev = mccui::Context::instance()->uavController()->device(desc.device);
    if (dev.isSome())
    {
        QString devInfo = QString::fromStdString(dev.unwrap()->deviceDescription()._device_info);
        if (dev.isSome() && !devInfo.isEmpty())
            item->setText(1, devInfo);
        else
            item->setText(1, desc.device);
    }
    item->setText(2, "Загружается");

    _filesTree->addTopLevelItem(item);

    Helper helper;
    helper.bar = new QProgressBar();
    helper.cancelButton = new VariantButton("Отмена",  QVariant::fromValue(desc));
    helper.removeButton = new VariantButton("Удалить", QVariant::fromValue(desc));
    helper.bar->setTextVisible(true);

    helper.removeButton->setVisible(false);

    _filesTree->setItemWidget(item, 3, helper.bar);

    QWidget* actionsWidget = new QWidget();
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(helper.cancelButton);
    buttonsLayout->addWidget(helper.removeButton);
    buttonsLayout->addStretch();
    buttonsLayout->setContentsMargins(1, 1, 1, 1);

    actionsWidget->setLayout(buttonsLayout);

    _filesTree->setItemWidget(item, 4, actionsWidget);

    _filesTree->resizeColumnToContents(0);
    _filesTree->resizeColumnToContents(1);

    _files.append(desc);
    _helpers.append(helper);

    connect(helper.cancelButton, &VariantButton::pressedWithData, this, &FilesTableWidget::cancelPressed);
    connect(helper.removeButton, &VariantButton::pressedWithData, this, &FilesTableWidget::removePressed);
}

void FilesTableWidget::onDeviceAdded(mcc::ui::mccui::FlyingDevice* aircraft)
{
    QString info = QString::fromStdString(aircraft->deviceDescription()._device_info);
    if (info.isEmpty())
        info = aircraft->name();

    _devicesList->addItem(info, aircraft->name());
}

void FilesTableWidget::onDeviceFirmwareLoaded(mcc::ui::mccui::FlyingDevice* aircraft)
{
    int index = _devicesList->findData(aircraft->name());
    if (index == -1)
        return;

    _devicesList->setItemText(index, QString::fromStdString(aircraft->deviceDescription()._device_info));
}

void FilesTableWidget::onDeviceRemoved(mcc::ui::mccui::FlyingDevice* aircraft)
{
    int idx = _devicesList->findData(aircraft->name());
    if (idx != -1)
        _devicesList->removeItem(idx);
}

void FilesTableWidget::cancelPressed(const QVariant& data)
{
    FileDescriptor desc = data.value<FileDescriptor>();
    qDebug() << desc.path;

    emit requestDeviceFileLoadCancel("", desc.device, desc.path, desc.remote, "Отменено пользователем");
}

void FilesTableWidget::removePressed(const QVariant& data)
{
    FileDescriptor desc = data.value<FileDescriptor>();
    qDebug() << desc.path;

    int idx = findFile("", desc.device, desc.path);
    if (idx == -1)
    {
        Q_ASSERT(false);
        qDebug() << "Can't find file in widget: " << desc.path;
        return;
    }

    _helpers.removeAt(idx);
    _files.removeAt(idx);
    QTreeWidgetItem* item = _filesTree->topLevelItem(idx);
    delete item;
}

void FilesTableWidget::uploadFilePressed()
{
    if (_devicesList->count() == 0)
        return;

    QString fileToUpload = QFileDialog::getOpenFileName(this, "Выбор файла для загрузки");
    if (fileToUpload.isEmpty())
        return;

    QString encoderService = _devicesList->currentData().toString();
    QString device = _devicesList->currentData().toString();

    bool ok = false;
    QString remote = QInputDialog::getText(this, "Введите путь к файлу на аппарате", "Введите путь к файлу на аппарате", QLineEdit::Normal, "0", &ok);
    if (ok && !remote.isEmpty())
    {
        addFile(FileDescriptor(device, fileToUpload, remote, FileDirection::Upload));
        emit requestDeviceFileLoad("", device, fileToUpload, remote, mcc::ui::ide::FileTransfer::Direction::Up);
    }
}

void FilesTableWidget::downloadFilePressed()
{
    if (_devicesList->count() == 0)
        return;

    QString fileToUpload = QFileDialog::getSaveFileName(this, "Введите путь для сохранения файла");
    if (fileToUpload.isEmpty())
        return;

    QString encoderService = _devicesList->currentData().toString();
    QString device = _devicesList->currentData().toString();

    bool ok = false;
    QString remote = QInputDialog::getText(this, "Введите путь к файлу на аппарате",
        "Введите путь к файлу на аппарате", QLineEdit::Normal, "0", &ok);
    if (ok && !remote.isEmpty())
    {
        addFile(FileDescriptor(device, fileToUpload, remote, FileDirection::Download));
        emit requestDeviceFileLoad("", device, fileToUpload, remote, mcc::ui::ide::FileTransfer::Direction::Down);
    }
}

void FilesTableWidget::onDeviceFileUploaded(const QString& device, const QString& filePath)
{
    int idx = findFile("" , device, filePath);
    if (idx == -1)
        return;

    _helpers[idx].bar->setValue(100);
    _helpers[idx].cancelButton->setVisible(false);
    _helpers[idx].removeButton->setVisible(true);

    auto widget = _filesTree->topLevelItem(idx);
    if (widget == nullptr)
        return;

    widget->setText(2, "Загружено");
}

void FilesTableWidget::onDeviceFileUploadFailed(const QString& device, const QString& filePath, const QString& reason)
{
    int idx = findFile("", device, filePath);

    if (idx == -1)
    {
        FileDescriptor fDesc;
        fDesc.device = device;
        fDesc.exchange = "";
        fDesc.path = filePath;
        fDesc.progress = 0;

        addFile(fDesc);
        idx = findFile("", device, filePath);
    }

    _helpers[idx].cancelButton->setVisible(false);
    _helpers[idx].removeButton->setVisible(true);

    auto widget = _filesTree->topLevelItem(idx);
    if (widget == nullptr)
        return;

    widget->setText(2, QString("Ошибка: %1").arg(reason));
    _files[idx].error = true;
}

void FilesTableWidget::fileUploadProgressChanged(const QString& device, const QString& path, int progress)
{
    int idx = findFile("", device, path);
    if (idx == -1)
        return;
    _helpers[idx].bar->setValue(progress);
}

void FilesTableWidget::setupUi()
{
    _devicesList = new QComboBox();
    _devicesList->setMinimumWidth(150);

    _filesTree = new QTreeWidget();
    _filesTree->setHeaderLabels(QStringList() << "Локальный путь" << "Аппарат" << "Направление" << "Состояние" << "Действия");
    _filesTree->header()->setStretchLastSection(true);
    _filesTree->setAlternatingRowColors(true);

    QVBoxLayout* layout = new QVBoxLayout();

    QHBoxLayout* sendFileLayout = new QHBoxLayout();
    QPushButton* sendFileButton = new QPushButton("Загрузить файл");
    QPushButton* loadFileButton = new QPushButton("Скачать файл");

    sendFileLayout->addWidget(_devicesList);
    sendFileLayout->addWidget(sendFileButton);
    sendFileLayout->addWidget(loadFileButton);
    sendFileLayout->addStretch();

    connect(sendFileButton, &QPushButton::pressed, this, &FilesTableWidget::uploadFilePressed);
    connect(loadFileButton, &QPushButton::pressed, this, &FilesTableWidget::downloadFilePressed);

    layout->addLayout(sendFileLayout);

    layout->addWidget(_filesTree);
    setLayout(layout);
}

int FilesTableWidget::findFile(const QString& exchange, const QString& device, const QString& file)
{
    Q_UNUSED(exchange);
    for (int i = 0; i < _files.count(); ++i)
    {
        FileDescriptor f = _files[i];

        if (f.device == device && f.path == file)
            return i;
    }

    return -1;
}

void FilesTableWidget::updateFile(const QString& exchange, const QString& device, const std::vector<mcc::ui::ide::FileTransfer>& files)
{
    for (auto& f : files)
    {
        int idx = findFile(exchange, device, QString::fromStdString(f._file_path));

        if (idx != -1 && _files[idx].error)
        {
            _helpers.removeAt(idx);
            _files.removeAt(idx);
            QTreeWidgetItem* item = _filesTree->topLevelItem(idx);
            delete item;
            idx = -1;
        }

        if (idx == -1)
        {
            FileDescriptor fDesc;
            fDesc.device = device;
            fDesc.exchange = "";
            fDesc.path = QString::fromStdString(f._file_path);
            fDesc.progress = f.progress();

            idx = _files.size();
            addFile(fDesc);
        }

        _helpers[idx].bar->setValue(f.progress());
        QString stateText;
        if (f._direction == mcc::ui::ide::FileTransfer::Direction::Up)
        {
            stateText = "Загрузка";
        }
        else
        {
            stateText = "Скачивание";
        }


        _filesTree->topLevelItem(idx)->setText(2, stateText);
    }
}
}
