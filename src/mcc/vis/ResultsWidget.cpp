#include "mcc/vis/ResultsWidget.h"
#include "mcc/vis/Region.h"
#include "mcc/vis/RegionViewer.h"
#include "mcc/vis/ProfileViewer.h"
#include "mcc/vis/ProfileDataViewer.h"
#include "mcc/vis/ReportGen.h"
#include "mcc/vis/ReportConfig.h"

#include <QTabWidget>
#include <QTableView>
#include <QHBoxLayout>
#include <QAbstractListModel>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QProgressDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QCheckBox>

namespace mccvis {

class DirectionListModel : public QAbstractListModel {
public:
    explicit DirectionListModel(const Region* region)
        : _region(region)
    {
    }

    void setRegion(const Region* region)
    {
        beginResetModel();
        _region.reset(region);
        endResetModel();
    }

    int rowCount(const QModelIndex& parent) const override
    {
        (void)parent;
        return _region->profiles().size();
    }

    double adjustAngle(double angle)
    {
        if (angle < 0) {
            return 360 + angle;
        }
        return 0;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (role == Qt::DisplayRole) {
            if (index.row() >= _region->profiles().size()) {
                return QVariant();
            }
            return _region->profiles()[index.row()]->direction();
        }
        return QVariant();
    }

private:
    Rc<const Region> _region;
};

class ResolutionEdit : public QWidget {
public:
    explicit ResolutionEdit(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        _widthBox = new QSpinBox;
        _heightBox = new QSpinBox;

        auto layout = new QHBoxLayout;
        layout->addStretch();
        layout->addWidget(_widthBox);
        layout->addWidget(new QLabel("x"));
        layout->addWidget(_heightBox);
        setLayout(layout);
    }

    void setMinResolution(int width, int height)
    {
        _widthBox->setMinimum(width);
        _heightBox->setMinimum(height);
    }

    void setMaxResolution(int width, int height)
    {
        _widthBox->setMaximum(width);
        _heightBox->setMaximum(height);
    }

    void setResolution(int width, int height)
    {
        _widthBox->setValue(width);
        _heightBox->setValue(height);
    }

    int resolutionWidth() const
    {
        return _widthBox->value();
    }

    int resolutionHeight() const
    {
        return _heightBox->value();
    }

private:
    QSpinBox* _widthBox;
    QSpinBox* _heightBox;
};

class ReportConfigWidget : public QDialog {
public:
    explicit ReportConfigWidget(QWidget* parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Настройки отчета");

        _excelBox = new QGroupBox("Отчет excel");
        _excelBox->setCheckable(true);
        _excelBox->setChecked(_config.genExcelReport);

        _profilesPerExcelBox = new QComboBox;
        _profilesPerExcelBox->addItems({"0", "1", "2", "3"});
        _profilesPerExcelBox->setCurrentIndex(3);

        auto excelLayout = new QGridLayout;
        excelLayout->addWidget(new QLabel("Профилей на страницу"), 0, 0);
        excelLayout->addWidget(_profilesPerExcelBox, 0, 1);
        _excelBox->setLayout(excelLayout);

        _profileImagesBox = new QGroupBox("Изображения профилей");
        _profileImagesBox->setCheckable(true);
        _profileImagesBox->setChecked(_config.genProfileImages);

        _profilesPerImageBox = new QComboBox;
        _profilesPerImageBox->addItems({"1", "2", "3"});
        _profilesPerImageBox->setCurrentIndex(2);

        _profileResolutionEdit = new ResolutionEdit;
        _profileResolutionEdit->setMinResolution(640, 480);
        _profileResolutionEdit->setMaxResolution(15360, 8640);
        _profileResolutionEdit->setResolution(_config.profileImageWidth, _config.profileImageHeight);

        _profileDrawBackground = new QCheckBox;
        _profileDrawGround = new QCheckBox;
        _profileDrawViewRegion = new QCheckBox;

        auto profLayout = new QGridLayout;
        profLayout->addWidget(new QLabel("Профилей на изображение"), 0, 0);
        profLayout->addWidget(_profilesPerImageBox, 0, 1);
        profLayout->addWidget(new QLabel("Разрешение"), 1, 0);
        profLayout->addWidget(_profileResolutionEdit, 1, 1);
        profLayout->addWidget(new QLabel("Рисовать фон"), 2, 0);
        profLayout->addWidget(_profileDrawBackground, 2, 1);
        profLayout->addWidget(new QLabel("Рисовать землю"), 3, 0);
        profLayout->addWidget(_profileDrawGround, 3, 1);
        profLayout->addWidget(new QLabel("Рисовать видимую зону"), 4, 0);
        profLayout->addWidget(_profileDrawViewRegion, 4, 1);
        _profileImagesBox->setLayout(profLayout);

        //zones
        _zoneImagesBox = new QGroupBox("Изображения зон");
        _zoneImagesBox->setCheckable(true);
        _zoneImagesBox->setChecked(_config.genZoneImages);

        _zoneResolutionEdit = new ResolutionEdit;
        _zoneResolutionEdit->setMinResolution(640, 480);
        _zoneResolutionEdit->setMaxResolution(15360, 8640);
        _zoneResolutionEdit->setResolution(_config.zoneImageWidth, _config.zoneImageHeight);

        _zoneDrawBackground = new QCheckBox;

        auto zoneLayout = new QGridLayout;
        zoneLayout->addWidget(new QLabel("Разрешение"), 0, 0);
        zoneLayout->addWidget(_zoneResolutionEdit, 0, 1);
        zoneLayout->addWidget(new QLabel("Рисовать фон"), 1, 0);
        zoneLayout->addWidget(_zoneDrawBackground, 1, 1);
        _zoneImagesBox->setLayout(zoneLayout);

        //angles
        _anglesImagesBox = new QGroupBox("Изображения углов закрытия");
        _anglesImagesBox->setCheckable(true);
        _anglesImagesBox->setChecked(_config.genAnglesImages);

        _anglesResolutionEdit = new ResolutionEdit;
        _anglesResolutionEdit->setMinResolution(640, 480);
        _anglesResolutionEdit->setMaxResolution(15360, 8640);
        _anglesResolutionEdit->setResolution(_config.anglesImageWidth, _config.anglesImageHeight);

        _anglesDrawBackground = new QCheckBox;

        auto anglesLayout = new QGridLayout;
        anglesLayout->addWidget(new QLabel("Разрешение"), 0, 0);
        anglesLayout->addWidget(_anglesResolutionEdit, 0, 1);
        anglesLayout->addWidget(new QLabel("Рисовать фон"), 1, 0);
        anglesLayout->addWidget(_anglesDrawBackground, 1, 1);
        _anglesImagesBox->setLayout(anglesLayout);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        auto mainLayout = new QVBoxLayout;
        mainLayout->addWidget(_excelBox);
        mainLayout->addWidget(_profileImagesBox);
        mainLayout->addWidget(_zoneImagesBox);
        mainLayout->addWidget(_anglesImagesBox);
        mainLayout->addStretch();
        mainLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &ReportConfigWidget::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &ReportConfigWidget::reject);

        setLayout(mainLayout);
    }

    bool execAndGetConfig(ReportConfig* dest,
                          const ProfileViewer::RenderConfig& profRenderCfg,
                          const RegionViewer::RenderConfig& zoneRenderCfg,
                          const RegionViewer::RenderConfig& angleRenderCfg)
    {
        _profileDrawBackground->setChecked(profRenderCfg.drawBackground);
        _profileDrawGround->setChecked(profRenderCfg.drawGround);
        _profileDrawViewRegion->setChecked(profRenderCfg.drawViewArea);
        _zoneDrawBackground->setChecked(zoneRenderCfg.drawBackground);
        _anglesDrawBackground->setChecked(angleRenderCfg.drawBackground);
        int rv = exec();
        if (rv == QDialog::Accepted) {
            dest->profileImageWidth = _profileResolutionEdit->resolutionWidth();
            dest->profileImageHeight = _profileResolutionEdit->resolutionHeight();
            dest->zoneImageWidth = _zoneResolutionEdit->resolutionWidth();
            dest->zoneImageHeight = _zoneResolutionEdit->resolutionHeight();
            dest->anglesImageWidth = _anglesResolutionEdit->resolutionWidth();
            dest->anglesImageHeight = _anglesResolutionEdit->resolutionHeight();
            dest->profileRenderCfg.drawBackground = _profileDrawBackground->isChecked();
            dest->profileRenderCfg.drawGround = _profileDrawGround->isChecked();
            dest->profileRenderCfg.drawViewArea = _profileDrawViewRegion->isChecked();
            dest->zoneRenderCfg.drawBackground = _zoneDrawBackground->isChecked();
            dest->anglesRenderCfg.drawBackground = _anglesDrawBackground->isChecked();
            dest->genExcelReport = _excelBox->isChecked();
            dest->genProfileImages = _profileImagesBox->isChecked();
            dest->genZoneImages = _zoneImagesBox->isChecked();
            dest->genAnglesImages = _anglesImagesBox->isChecked();
            dest->profilesPerExcelPage = _profilesPerExcelBox->currentIndex();
            dest->profilesPerImage = _profilesPerImageBox->currentIndex() + 1;
            return true;
        }
        return false;
    }

private:
    QComboBox* _profilesPerExcelBox;
    QComboBox* _profilesPerImageBox;
    ResolutionEdit* _profileResolutionEdit;
    QCheckBox* _profileDrawBackground;
    QCheckBox* _profileDrawGround;
    QCheckBox* _profileDrawViewRegion;
    ResolutionEdit* _zoneResolutionEdit;
    QCheckBox* _zoneDrawBackground;
    ResolutionEdit* _anglesResolutionEdit;
    QCheckBox* _anglesDrawBackground;
    QGroupBox* _excelBox;
    QGroupBox* _profileImagesBox;
    QGroupBox* _zoneImagesBox;
    QGroupBox* _anglesImagesBox;

    ReportConfig _config;
};

ResultsWidget::ResultsWidget(const Region* region)
    : _region(region)
    , _gen(0)
{
    _directionsModel = new DirectionListModel(region);

    _directionsView = new QTableView;
    _directionsView->horizontalHeader()->setVisible(false);
    _directionsView->verticalHeader()->setVisible(false);
    _directionsView->setSelectionBehavior(QTableView::SelectRows);
    _directionsView->setSelectionMode(QTableView::SingleSelection);
    _directionsView->setAlternatingRowColors(true);
    _directionsView->setModel(_directionsModel);

    _profileViewer = new ProfileViewer;
    _regionViewer = new RegionViewer;
    _angleViewer = new RegionViewer;
    _angleViewer->setMode(RegionViewer::AnglesMode);
    _dataViewer = new ProfileDataViewer;

    _tabWidget = new QTabWidget;
    _tabWidget->addTab(_profileViewer, "Профиль");
    _tabWidget->addTab(_regionViewer, "Зоны");
    _tabWidget->addTab(_angleViewer, "Углы закрытия");
    _tabWidget->addTab(_dataViewer, "Расчет");

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(_directionsView);
    mainLayout->addSpacing(0);
    mainLayout->addWidget(_tabWidget);
    //mainLayout->setStretchFactor(_tabWidget, 2);

    setLayout(mainLayout);

    QMenuBar* menu = new QMenuBar;
    QMenu* fileMenu = menu->addMenu("Файл");
    QAction* saveProfAction = fileMenu->addAction("Сохранить профиль");
    QAction* saveZoneAction = fileMenu->addAction("Сохранить зоны");
    QAction* saveAnglesAction = fileMenu->addAction("Сохранить углы закрытия");
    fileMenu->addSeparator();
    QAction* printProfAction = fileMenu->addAction("Распечатать профиль");
    QAction* printZoneAction = fileMenu->addAction("Распечатать зоны");
    QAction* printAnglesAction = fileMenu->addAction("Распечатать углы закрытия");
    fileMenu->addSeparator();
    QAction* saveReportAction = fileMenu->addAction("Сгенерировать отчет");
    fileMenu->addSeparator();
    QAction* closeAction = fileMenu->addAction("Выход");

    connect(saveProfAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить профиль"), "profile.png", tr("png (*.png);;jpeg (*.jpg *.jpeg)"));
        if (fileName.isNull()) {
            return;
        }
        QImage img(1920, 1080, QImage::Format_ARGB32_Premultiplied);
        _profileViewer->renderPlot(&img, _profileViewer->renderConfig());
        img.save(fileName);
    });
    connect(saveZoneAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить зоны"), "zone.png", tr("png (*.png);;jpeg (*.jpg *.jpeg)"));
        if (fileName.isNull()) {
            return;
        }
        QImage img(1080, 1080, QImage::Format_ARGB32_Premultiplied);
        _regionViewer->renderPlot(&img, _regionViewer->renderConfig());
        img.save(fileName);
    });

    connect(saveAnglesAction, &QAction::triggered, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить углы закрытия"), "angles.png", tr("png (*.png);;jpeg (*.jpg *.jpeg)"));
        if (fileName.isNull()) {
            return;
        }
        QImage img(1080, 1080, QImage::Format_ARGB32_Premultiplied);
        _angleViewer->renderPlot(&img, _angleViewer->renderConfig());
        img.save(fileName);
    });

    connect(printProfAction, &QAction::triggered, this, [this]() {
        QPrintPreviewDialog dialog(this);

        connect(&dialog, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter* printer) {
            _profileViewer->renderPlot(printer, _profileViewer->renderConfig());
        });
        dialog.exec();
    });

    connect(printZoneAction, &QAction::triggered, this, [this]() {
        QPrintPreviewDialog dialog(this);

        connect(&dialog, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter* printer) {
            _regionViewer->renderPlot(printer, _regionViewer->renderConfig());
        });
        dialog.exec();
    });

    connect(printAnglesAction, &QAction::triggered, this, [this]() {
        QPrintPreviewDialog dialog(this);

        connect(&dialog, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter* printer) {
            _angleViewer->renderPlot(printer, _angleViewer->renderConfig());
        });
        dialog.exec();
    });

    connect(saveReportAction, &QAction::triggered, this, [this]() {
        ReportConfigWidget configWidget(this);
        ReportConfig conf;
        if (!configWidget.execAndGetConfig(&conf,
                                           _profileViewer->renderConfig(),
                                           _regionViewer->renderConfig(),
                                           _angleViewer->renderConfig())) {
            return;
        }

        _gen = new ReportGen;
        QString dir = QFileDialog::getExistingDirectory(this, tr("Сохранить отчет"));
        _dialog = new QProgressDialog(this);
        _dialog->hide();
        _dialog->setMinimum(0);
        _dialog->setMaximum(100);
        _dialog->setValue(0);
        _dialog->setAutoClose(false);
        QObject::connect(_gen, &ReportGen::finished, _dialog, &QProgressDialog::accept, Qt::QueuedConnection);
        QObject::connect(_gen, &ReportGen::progressChanged, _dialog, &QProgressDialog::setValue, Qt::QueuedConnection);
        _gen->generateReport(_region.get(), dir, conf);
        _dialog->exec();
        _gen->wait();
        QObject::disconnect(_dialog, 0, 0, 0);
        QObject::disconnect(_gen, 0, 0, 0);
        delete _dialog;
        _gen->deleteLater();
    });
    connect(closeAction, &QAction::triggered, this, &QWidget::close);

    layout()->setMenuBar(menu);

    auto s = _directionsView->selectionModel();
    connect(s, &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& cur, const QModelIndex& prev) {
        (void)prev;
        selectProfile(cur.row());
    });

    connect(_regionViewer, &RegionViewer::profileClicked, this, [this](std::size_t i) {
        _directionsView->selectionModel()->setCurrentIndex(_directionsModel->index(i, 0), QItemSelectionModel::ClearAndSelect);
        _angleViewer->setSelectedProfile(bmcl::Option<std::size_t>(i));
    });
    connect(_angleViewer, &RegionViewer::profileClicked, this, [this](std::size_t i) {
        _directionsView->selectionModel()->setCurrentIndex(_directionsModel->index(i, 0), QItemSelectionModel::ClearAndSelect);
        _regionViewer->setSelectedProfile(bmcl::Option<std::size_t>(i));
    });

    updateView();
}

ResultsWidget::~ResultsWidget()
{
}

void ResultsWidget::selectProfile(std::size_t i)
{
    std::size_t numProfiles = _region->profiles().size();
    if (i >= numProfiles) {
        _profileViewer->setProfile(bmcl::None);
        _dataViewer->setProfile(bmcl::None);
        _regionViewer->setSelectedProfile(bmcl::None);
        _angleViewer->setSelectedProfile(bmcl::None);
        return;
    }
    const Profile* p = _region->profiles()[i].get();
    _profileViewer->setProfile(p);
    _dataViewer->setProfile(p);
    _regionViewer->setSelectedProfile(bmcl::Option<std::size_t>(i));
    _angleViewer->setSelectedProfile(bmcl::Option<std::size_t>(i));
}

void ResultsWidget::setRegion(const Region* region)
{
    _region.reset(region);
    updateView();
}

void ResultsWidget::updateView()
{
    _regionViewer->setRegion(_region.get());
    _angleViewer->setRegion(_region.get());
    _tabWidget->setCurrentIndex(0);
    _directionsModel->setRegion(_region.get());
    _directionsView->selectionModel()->setCurrentIndex(_directionsModel->index(0, 0), QItemSelectionModel::Select);
    _directionsView->setMaximumWidth(100); //HACK
    adjustSize();
}
}
