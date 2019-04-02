#include "mcc/hm/OmhmReader.h"

#include <bmcl/Endian.h>
#include <bmcl/Buffer.h>

#ifndef BMCL_LITTLE_ENDIAN
#error "Only works on little endian systems"
#endif

#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QProgressBar>
#include <QStyle>
#include <QDesktopWidget>

#include <iostream>

#include <gdal_priv.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include <ogr_spatialref.h>

#include <stddef.h>
#include <stdint.h>

void reportErrorAndExit(const QString& error)
{
    QMessageBox::critical(nullptr, "Ошибка", error, QMessageBox::Close);
}

int progressCallback(double value, const char* msg, void* arg)
{
    QProgressBar* progressBar = (QProgressBar*)arg;
    progressBar->setValue(100 * value);
    qApp->processEvents();
    return TRUE;
}

void startApp(int argc, char** argv)
{
    QString srcPath = QFileDialog::getOpenFileName(nullptr, "Открыть карту высот", QString(), "Все файлы (*.*)");
    if (srcPath.isNull()) {
        return;
    }

    GDALAllRegister();
    GDALDataset* dataset = (GDALDataset*)GDALOpen(srcPath.toUtf8().data(), GA_ReadOnly);
    if (!dataset) {
        reportErrorAndExit(QString("Не удалось открыть карту: ") + CPLGetLastErrorMsg());
        return;
    }

    QString destPath = QFileDialog::getSaveFileName(nullptr, "Выбрать файл для сохранения", QString(), "Карта высот (*.omhm)");
    if (destPath.isNull()) {
        return;
    }

    if (dataset->GetRasterCount() == 0) {
        reportErrorAndExit("Пустой файл");
        return;
    }

    // x = t0 + p*t1 + l*t2
    // y = t3 + p*t4 + l*t5
    double geoTransform[6];
    if (dataset->GetGeoTransform(geoTransform) != CE_None) {
        qWarning() << "Ошибка чтения геотрансформации";
    }

    GDALRasterBand* band = dataset->GetRasterBand(1);

    std::string unitStr = band->GetUnitType();
    double unitScale = 1;

    if (unitStr == "") {
        unitScale = 1;
    } else if (unitStr == "m") {
        unitScale = 1;
    } else if (unitStr == "dm") {
        unitScale = 0.1;
    } else if (unitStr == "cm") {
        unitScale = 0.01;
    } else if (unitStr == "mm") {
        unitScale = 0.001;
    } else {
        reportErrorAndExit(QString("Неправильная единица измерения высот: ") + QString::fromStdString(unitStr));
        return;
    }

    mcchm::OmhmDataType dtype;
    switch (band->GetRasterDataType()) {
    case GDT_Byte:
        dtype = mcchm::OmhmDataType::UInt8;
        break;
    case GDT_UInt16:
        dtype = mcchm::OmhmDataType::UInt16;
        break;
    case GDT_Int16:
        dtype = mcchm::OmhmDataType::Int16;
        break;
    case GDT_UInt32:
        dtype = mcchm::OmhmDataType::UInt32;
        break;
    case GDT_Int32:
        dtype = mcchm::OmhmDataType::Int32;
        break;
    case GDT_Float32:
        dtype = mcchm::OmhmDataType::Float32;
        break;
    case GDT_Float64:
        dtype = mcchm::OmhmDataType::Float64;
        break;
    default:
        reportErrorAndExit("Неправильный формат чисел задающих высоты");
        return;
    }

    const char* projectionRef = dataset->GetProjectionRef();
    if (!projectionRef) {
        reportErrorAndExit("В карте отсутствует информация о проекции");
        return;
    }

#if GDAL_VERSION_MAJOR < 2
# error "gdal version >=2 required"
#endif

    OGRSpatialReference spatialRef;
#if GDAL_VERSION_MINOR < 3
    // gdal < 2.3 HACK
    std::string projectionRefString(projectionRef);
    char* refData = (char*)projectionRefString.data();
    if (spatialRef.importFromWkt(&refData) != OGRERR_NONE) {
#else
    if (spatialRef.importFromWkt(projectionRef) != OGRERR_NONE) {
#endif
        reportErrorAndExit("Ошибка чтения информации о проекции");
        return;
    }

    struct Proj4Desc {
        Proj4Desc()
            : desc(nullptr)
        {
        }

        ~Proj4Desc()
        {
            if (desc) {
                CPLFree(desc);
            }
        }

        char* desc;
    } proj4Desc;

    spatialRef.exportToProj4(&proj4Desc.desc);
    std::size_t proj4DescSize = std::strlen(proj4Desc.desc);

    bmcl::Buffer header;
    header.reserve(6 * 4 + 8 * 10 + proj4DescSize + 4);
    header.writeUint32Le(mcchm::OmhmReader::magicHeader); //magic
    header.writeUint32Le(1); //version
    header.writeUint32Le(0); //reserved
    header.writeUint32Le(uint32_t(dtype)); //dtype
    header.writeUint32Le(band->GetXSize()); //image width
    header.writeUint32Le(band->GetYSize()); //image height
    header.writeFloat64Le(geoTransform[0]); // origin x
    header.writeFloat64Le(geoTransform[1]); // pixel size x
    header.writeFloat64Le(geoTransform[2]); //
    header.writeFloat64Le(geoTransform[3]); // origin y
    header.writeFloat64Le(geoTransform[4]); // pixel size y
    header.writeFloat64Le(geoTransform[5]); //
    header.writeFloat64Le(band->GetNoDataValue()); //no data value
    header.writeFloat64Le(band->GetScale() * unitScale); //height scale
    header.writeFloat64Le(band->GetOffset()); //height offset
    header.writeUint64Le(proj4DescSize); //desc size
    header.write(proj4Desc.desc, proj4DescSize);
    header.writeUint32Le(mcchm::OmhmReader::crc32(header.data(), header.size()));

    int nXSize = band->GetXSize();
    int nYSize = band->GetYSize();
    size_t fsize = header.size() + size_t(nXSize) * size_t(nYSize) * GDALGetDataTypeSizeBytes(band->GetRasterDataType());

    double gbSize = double(fsize) / (1024 * 1024 * 1024);
    QString sizeQuestion = QString("Файл займет %1гб. Продолжить?").arg(gbSize, 'f');
    auto btn = QMessageBox::question(nullptr, "Размер файла", sizeQuestion, QMessageBox::Save | QMessageBox::Cancel);
    if (btn == QMessageBox::Cancel) {
        return;
    }

    QFile file;
    file.setFileName(destPath);
    if (!file.open(QFile::ReadWrite)) {
        reportErrorAndExit("Не удалось открыть файл для записи: " + file.errorString());
        return;
    }

    if (!file.resize(fsize)) {
        reportErrorAndExit("Не удалось увеличить файл до нужного размера: " + file.errorString());
        return;
    }

    uint8_t* dest = file.map(0, fsize);
    if (!dest) {
        reportErrorAndExit("Не удалось отобразить файл в память: " + file.errorString());
        return;
    }

    std::memcpy(dest, header.data(), header.size());
    dest += header.size();

    GDALRasterIOExtraArg extraArg;
    INIT_RASTERIO_EXTRA_ARG(extraArg);
    QProgressBar* progressBar = new QProgressBar;
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->show();
    progressBar->resize(640, progressBar->height());
    progressBar->setWindowTitle("Сохранение файла...");

    progressBar->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            progressBar->size(),
            qApp->desktop()->availableGeometry()
        )
    );

    qApp->processEvents();
    extraArg.pfnProgress = progressCallback;
    extraArg.pProgressData = progressBar;

    auto rv = band->RasterIO(GF_Read, 0, 0, nXSize, nYSize,
                             dest, nXSize, nYSize,
                             band->GetRasterDataType(), 0, 0, &extraArg);

    delete progressBar;
    if (rv != CE_None) {
        reportErrorAndExit(QString("Ошибка при чтении файла: ") + CPLGetLastErrorMsg());
        return;
    }

    QMessageBox::information(nullptr, "Сохранение файла...", "Файл сохранен", QMessageBox::Ok);
}

int main(int argc, char** argv)
{
    int fakeArgc = 1;
    QApplication app(fakeArgc, argv);
    QTimer::singleShot(0, [=](){ startApp(argc, argv); });
    return app.exec();
}
