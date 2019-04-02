#include "GlobalSettingsPage.h"

#include "ui_GlobalSettingsPage.h"

#include "mcc/ui/SettingsPagePlugin.h"
#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/CoordinateEditor.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/CoordinateSystem.h"
#include "mcc/res/Resource.h"

#include "mcc/plugin/PluginCache.h"

#include <QFileDialog>
#include <QClipboard>

#include <bmcl/OptionUtils.h>

GlobalSettingsPage::GlobalSettingsPage(mccui::CoordinateSystemController* csController, mccui::Settings* settings, QWidget* parent)
    : SettingsPage(parent)
    , _lcsCenterLatitude(nullptr)
    , _lcsCenterLongitude(nullptr)
    , _lcsCenterAltitude(nullptr)
    , _lcsDirectionLatitude(nullptr)
    , _lcsDirectionLongitude(nullptr)
    , _lcsDirectionAngle(nullptr)
    , _csController(csController)
    , _settings(settings)
{
    _ui = new Ui::GlobalSettingsPage;
    _ui->setupUi(this);

    _invertedPfdWriter = settings->acquireUniqueWriter("pfd/inverted", false).unwrap();
    _showOnMapPfdWriter = settings->acquireUniqueWriter("pfd/showOnMap", false).unwrap();
/*
    _ui->lcsSettings->setEnabled(false);
    const int leftColumn = 2;
    _lcsCenterLatitude = new mccui::CoordinateEditor(_ui->lcsSettings);
    _lcsCenterLatitude->setMinMax(-85.0, 85.0);
    _ui->lcsLayout->addWidget(_lcsCenterLatitude, 1, leftColumn);

    _lcsCenterLongitude = new mccui::CoordinateEditor(_ui->lcsSettings);
    _lcsCenterLongitude->setMinMax(-180.0, 180.0);
    _ui->lcsLayout->addWidget(_lcsCenterLongitude, 2, leftColumn);

    _lcsCenterAltitude = new mccui::CoordinateEditor(_ui->lcsSettings);
    _lcsCenterAltitude->setCoordinateFormat(mccui::CoordinateFormat("м"));
    _lcsCenterAltitude->setMinMax(-6378100.0, 1000000.0);
    _ui->lcsLayout->addWidget(_lcsCenterAltitude, 3, leftColumn);

    const int rightColumn = leftColumn + 5 + 2;
    _lcsDirectionLatitude = new mccui::CoordinateEditor(_ui->lcsSettings);
    _lcsDirectionLatitude->setMinMax(-85.0, 85.0);
    _ui->lcsLayout->addWidget(_lcsDirectionLatitude, 1, rightColumn);

    _lcsDirectionLongitude = new mccui::CoordinateEditor(_ui->lcsSettings);
    _lcsDirectionLongitude->setMinMax(-180.0, 180.0);
    _ui->lcsLayout->addWidget(_lcsDirectionLongitude, 2, rightColumn);

    _lcsDirectionAngle = new mccui::CoordinateEditor(_ui->lcsSettings);
    _lcsDirectionAngle->setMinMax(0.0, 360.0);
    _ui->lcsLayout->addWidget(_lcsDirectionAngle, 3, rightColumn);

    QIcon copyIcon = mccres::loadIcon(mccres::ResourceKind::CopyIcon);
    QIcon pasteIcon = mccres::loadIcon(mccres::ResourceKind::PasteIcon);

    _ui->lcsCenterCopy->setToolTip("Копировать");
    _ui->lcsCenterCopy->setIcon(copyIcon);
    _ui->lcsCenterPaste->setToolTip("Вставить");
    _ui->lcsCenterPaste->setIcon(pasteIcon);

    _ui->lcsDirectionCopy->setToolTip("Копировать");
    _ui->lcsDirectionCopy->setIcon(copyIcon);
    _ui->lcsDirectionPaste->setToolTip("Вставить");
    _ui->lcsDirectionPaste->setIcon(pasteIcon);
    //TODO: complete copy/paste
    connect(_ui->lcsCenterCopy, &QToolButton::pressed, this,
            [this]()
    {
        copyLcs(LcsForm::CenterPoint);
    });
    connect(_ui->lcsDirectionCopy, &QToolButton::pressed, this,
            [this]()
    {
        copyLcs(LcsForm::DirectionPoint);
    });
    connect(_ui->lcsCenterPaste, &QToolButton::pressed, this,
            [this]()
    {
        pasteLcs(LcsForm::CenterPoint);
    });
    connect(_ui->lcsDirectionPaste, &QToolButton::pressed, this,
            [this]()
    {
        pasteLcs(LcsForm::DirectionPoint);
    });

    _ui->lcsMethod->addItem("Центр и угол (оси Y от севера по часовой)", CenterAndAngle);
    _ui->lcsMethod->addItem("Центр и точка направления (оси Y)", CenterAndDirection);

    connect(_ui->lcsMethod, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]()
    {
        checkLcsMethod();
    });
    checkLcsMethod();
*/


    for (std::size_t i = 0; i < _csController->systems().size(); i++) {
        const auto& desc = _csController->systems()[i];
        _ui->coordSystem->addItem(desc.fullName(), static_cast<int>(i));
    }

    _ui->coordFormat->addItem("Градусы", static_cast<int>(mccui::AngularFormat::Degrees));
    _ui->coordFormat->addItem("Градусы, минуты", static_cast<int>(mccui::AngularFormat::DegreesMinutes));
    _ui->coordFormat->addItem("Градусы, минуты, секунды", static_cast<int>(mccui::AngularFormat::DegreesMinutesSeconds));

    connect(_ui->coordSystem, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]()
    {
        updatecsController();
        //_ui->lcsSettings->setEnabled(false);
        //_ui->lcsSettings->setEnabled(static_cast<mccui::CoordinateSystem>(_ui->coordSystem->currentData().toInt()) ==
        //                             mccui::CoordinateSystem::LCS);
    });

    connect(_ui->coordFormat, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]()
    {
        updatecsController();

//        updateLcsFormat(static_cast<mccui::AngularFormat>(_ui->coordFormat->currentData().toInt()));
    });

    connect(_ui->invertedPfdIndication, &QCheckBox::stateChanged,
          [this]()
    {
        _invertedPfdWriter->write(_ui->invertedPfdIndication->isChecked());
    });

    connect(_ui->showOnMapPfd, &QCheckBox::stateChanged,
            [this]()
    {
        _showOnMapPfdWriter->write(_ui->showOnMapPfd->isChecked());
    });
/*
    connect(_lcsCenterLatitude,     &mccui::CoordinateEditor::valueChanged, this, [this]() {updateLcs();});
    connect(_lcsCenterLongitude,    &mccui::CoordinateEditor::valueChanged, this, [this]() {updateLcs();});
    connect(_lcsCenterAltitude,     &mccui::CoordinateEditor::valueChanged, this, [this]() {updateLcs();});

    connect(_lcsDirectionLatitude,  &mccui::CoordinateEditor::valueChanged, this, [this]() {updateLcs();});
    connect(_lcsDirectionLongitude, &mccui::CoordinateEditor::valueChanged, this, [this]() {updateLcs();});
    connect(_lcsDirectionAngle,     &mccui::CoordinateEditor::valueChanged, this, [this]() {updateLcs();});

    updateLcsFormat(_csController->angularFormat());
*/
    setWindowTitle("Общие");
}

GlobalSettingsPage::~GlobalSettingsPage()
{
    delete _ui;
}

QString GlobalSettingsPage::pageTitle() const
{
    return "Общие";
}

QString GlobalSettingsPage::pagePath() const
{
    return "Общие";
}

QIcon GlobalSettingsPage::pageIcon() const
{
    return mccres::loadIcon(mccres::ResourceKind::SettingsIcon);
}

void GlobalSettingsPage::load()
{
    using mccui::Settings;

    std::size_t currentCoordSystem = _csController->currentSystemIndex();
    mccui::AngularFormat currentCoordFormat = _csController->angularFormat();

    int coordSystemIdx = _ui->coordSystem->findData(static_cast<int>(currentCoordSystem));
    int coordFormatIdx = _ui->coordFormat->findData(static_cast<int>(currentCoordFormat));

    _ui->coordSystem->setCurrentIndex(coordSystemIdx);
    _ui->coordFormat->setCurrentIndex(coordFormatIdx);

//     mccgeo::LocalSystem localSystem = _csController->localSystem();
//     mccgeo::Position centerPos = localSystem.centerPoint();
//     _lcsCenterLatitude->setValue(centerPos.latitude());
//
//     _lcsCenterLongitude->setValue(centerPos.longitude());
//
//     _lcsCenterAltitude->setValue(centerPos.altitude());
//
//     mccgeo::Position dirPos = localSystem.directionPoint();
//     _lcsDirectionLatitude->setValue(dirPos.latitude());
//     _lcsDirectionLongitude->setValue(dirPos.longitude());
//
//     _lcsDirectionAngle->setValue(localSystem.angle());

    _ui->invertedPfdIndication->setChecked(_invertedPfdWriter->read().toBool());
    _ui->showOnMapPfd->setChecked(_showOnMapPfdWriter->read().toBool());
}

void GlobalSettingsPage::apply()
{
    updatecsController();
    updateLcs();
    _invertedPfdWriter->write(_ui->invertedPfdIndication->isChecked());
}

void GlobalSettingsPage::saveOld()
{
    _old.coordSystemIndex   = _ui->coordSystem->currentIndex();
    _old.coordFormatIndex   = _ui->coordFormat->currentIndex();
/*
    _old.lcsMethodIndex     = _ui->lcsMethod->currentIndex();

    _old.lcsCenterLatitude  = _lcsCenterLatitude->value();
    _old.lcsCenterLongitude = _lcsCenterLongitude->value();
    _old.lcsCenterAltitude  = _lcsCenterAltitude->value();

    _old.lcsDirectionLatitude   = _lcsDirectionLatitude->value();
    _old.lcsDirectionLongitude  = _lcsDirectionLongitude->value();
    _old.lcsDirectionAngle      = _lcsDirectionAngle->value();
*/

    _old.invertedPfdIndication  = _ui->invertedPfdIndication->isChecked();
}

void GlobalSettingsPage::restoreOld()
{
    _ui->coordSystem->setCurrentIndex(_old.coordSystemIndex);
    _ui->coordFormat->setCurrentIndex(_old.coordFormatIndex);
/*
    _lcsCenterLatitude->setValue(_old.lcsCenterLatitude);
    _lcsCenterLongitude->setValue(_old.lcsCenterLongitude);
    _lcsCenterAltitude->setValue(_old.lcsCenterAltitude);

    _lcsDirectionLatitude->setValue(_old.lcsDirectionLatitude);
    _lcsDirectionLongitude->setValue(_old.lcsDirectionLongitude);
    _lcsDirectionAngle->setValue(_old.lcsDirectionAngle);

    updateLcs();

    _ui->lcsMethod->setCurrentIndex(_old.lcsMethodIndex);
*/
    _ui->invertedPfdIndication->setChecked(_old.invertedPfdIndication);
}

void GlobalSettingsPage::updateLcsFormat(mccui::AngularFormat format)
{
/*
    mccui::CoordinateFormat fmt(format);
    _lcsCenterLatitude->setCoordinateFormat(fmt);
    _lcsCenterLongitude->setCoordinateFormat(fmt);
    _lcsDirectionLatitude->setCoordinateFormat(fmt);
    _lcsDirectionLongitude->setCoordinateFormat(fmt);
    _lcsDirectionAngle->setCoordinateFormat(fmt);
*/
}

void GlobalSettingsPage::updatecsController()
{
    std::size_t coordSystem = static_cast<size_t>(_ui->coordSystem->currentData().toInt());
    mccui::AngularFormat coordFormat = static_cast<mccui::AngularFormat>(_ui->coordFormat->currentData().toInt());

    //TODO: merge
    _csController->setAngularFormat(coordFormat);
    _csController->selectSystem(coordSystem);
}

void GlobalSettingsPage::updateLcs()
{
//     mccgeo::Position lcsCenter(_lcsCenterLatitude->value(),
//                                _lcsCenterLongitude->value(),
//                                _lcsCenterAltitude->value());
//     if(_ui->lcsMethod->currentIndex() == CenterAndAngle)
//     {
//         mccgeo::LocalSystem localSystem(lcsCenter, _lcsDirectionAngle->value());
//         _csController->setLocalSystem(localSystem);
//
//         // update other form
//         mccgeo::Position dirPos = localSystem.directionPoint();
//         _lcsDirectionLatitude->setValue(dirPos.latitude());
//         _lcsDirectionLongitude->setValue(dirPos.longitude());
//     }
//     else
//     {
//         mccgeo::Position dirPos(_lcsDirectionLatitude->value(), _lcsDirectionLongitude->value(), 0.0);
//         mccgeo::LocalSystem localSystem(lcsCenter, dirPos);
//         _csController->setLocalSystem(localSystem);
//
//         // update other form
//         _lcsDirectionAngle->setValue(localSystem.angle());
//     }
}

void GlobalSettingsPage::checkLcsMethod()
{
/*
    if(_ui->lcsMethod->currentIndex() == CenterAndAngle)
    {
        _ui->lcsDirectionLatitudeLabel->setEnabled(false);
        _ui->lcsDirectionLongitudeLabel->setEnabled(false);
        _lcsDirectionLatitude->setEnabled(false);
        _lcsDirectionLongitude->setEnabled(false);

        _ui->lcsDirectionCopy->setEnabled(false);
        _ui->lcsDirectionPaste->setEnabled(false);

        _ui->lcsDirectionAngleLabel->setEnabled(true);
        _lcsDirectionAngle->setEnabled(true);
    }
    else
    {
        _ui->lcsDirectionAngleLabel->setEnabled(false);
        _lcsDirectionAngle->setEnabled(false);

        _ui->lcsDirectionLatitudeLabel->setEnabled(true);
        _ui->lcsDirectionLongitudeLabel->setEnabled(true);
        _lcsDirectionLatitude->setEnabled(true);
        _lcsDirectionLongitude->setEnabled(true);

        _ui->lcsDirectionCopy->setEnabled(true);
        _ui->lcsDirectionPaste->setEnabled(true);
    }
*/
}

void GlobalSettingsPage::copyLcs(GlobalSettingsPage::LcsForm form)
{
//     mccgeo::LatLon latLon;
//
//     if(form == LcsForm::CenterPoint)
//     {
//         latLon = mccgeo::LatLon(_lcsCenterLatitude->value(), _lcsCenterLongitude->value());
//     }
//     else if(form == LcsForm::DirectionPoint)
//     {
//         latLon = mccgeo::LatLon(_lcsDirectionLatitude->value(), _lcsDirectionLongitude->value());
//     }
//
//     // Only in WGS84
//     QApplication::clipboard()->setMimeData(_csController->makeMimeData(latLon,
//                                                                   mccui::CoordinateSystem::WGS84,
//                                                                   _csController->format()));
}

void GlobalSettingsPage::pasteLcs(GlobalSettingsPage::LcsForm form)
{
//     bmcl::Option<mccgeo::LatLon> latLonOption = _csController->getLatLonFromClipboard();
//
//     if (latLonOption.isNone())
//         return;
//
//     mccgeo::LatLon wgsLatLon = latLonOption.unwrap();
//
//     if(form == LcsForm::CenterPoint)
//     {
//         _lcsCenterLatitude->setValue(wgsLatLon.latitude());
//         _lcsCenterLongitude->setValue(wgsLatLon.longitude());
//     }
//     else if(form == LcsForm::DirectionPoint)
//     {
//         _lcsDirectionLatitude->setValue(wgsLatLon.latitude());
//         _lcsDirectionLongitude->setValue(wgsLatLon.longitude());
//     }
}

class GlobalPagePlugin : public mccui::SettingsPagePlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto csControllerData = cache->findPluginData<mccui::CoordinateSystemControllerPluginData>();
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        if (bmcl::anyNone(csControllerData, settingsData)) {
            return false;
        }
        setSettingsPage(new GlobalSettingsPage(csControllerData->csController(), settingsData->settings()));
        return true;
    }

    int64_t priority() const override
    {
        return std::numeric_limits<int64_t>::min();
    }
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<GlobalPagePlugin>());
}

MCC_INIT_PLUGIN(create);
