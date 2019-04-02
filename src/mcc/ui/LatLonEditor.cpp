#include "mcc/ui/LatLonEditor.h"

#include "mcc/geo/Vector3D.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/Coordinate.h"
#include "mcc/ui/CoordinateEditor.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/CoordinateSystem.h"
#include "mcc/res/Resource.h"

#include <bmcl/Logging.h>

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QLineEdit>

namespace mccui {

LatLonEditor::LatLonEditor(const CoordinateSystemController* csController, QWidget* parent)
    : QWidget(parent)
    , _csController(csController)
    , _systemIndex(0)
{
    QGridLayout* layout = new QGridLayout();
    setLayout(layout);
    _latEditor = new CoordinateEditor();
    _latEditor->setToolTip("Редактировать широту");
    _lonEditor = new CoordinateEditor();
    _lonEditor->setToolTip("Редактировать долготу");
    _altEditor = new QLineEdit();
    _altEditor->setToolTip("Редактировать высоту");
    _latLabel = new QLabel("Ш:");
    _lonLabel = new QLabel("Д:");
    _altLabel = new QLabel("В:");
    _altEditor->setFixedWidth(70);
    _altEditor->setAlignment(Qt::AlignCenter);
    _altEditor->setMaxLength(10);
    _latEditor->setMinMax(-85.0, 85.0);
    _lonEditor->setMinMax(-180.0, 180.0);

    _systemSelector = new QComboBox();
    _systemSelector->setToolTip("Выбор системы координат");
    _formatSelector = new QComboBox();
    _formatSelector->setToolTip("Выбор формата ввода и отображения координат");
    _copyButton = new QToolButton();
    _pasteButton = new QToolButton();
    _copyButton->setToolTip("Копировать");
    _pasteButton->setToolTip("Вставить");

    QIcon copyIcon = mccres::loadIcon(mccres::ResourceKind::CopyIcon);
    QIcon pasteIcon = mccres::loadIcon(mccres::ResourceKind::PasteIcon);

    _copyButton->setIcon(copyIcon);
    _pasteButton->setIcon(pasteIcon);

    for (std::size_t i = 0; i < _csController->systems().size(); i++) {
        const CoordinateSystem& desc = _csController->systems()[i];
        _systemSelector->addItem(desc.fullName(), static_cast<int>(i));
    }

    connect(_csController.get(), &CoordinateSystemController::coordinateSystemAdded, this, [this](std::size_t i) {
        const CoordinateSystem& desc = _csController->systems()[i];
        _systemSelector->addItem(desc.fullName(), static_cast<int>(i));
    });

    connect(_csController.get(), &CoordinateSystemController::coordinateSystemSelected, this, [this](std::size_t i) {
        _systemSelector->setCurrentIndex(i);
    });

    _formatSelector->addItem("Г", static_cast<int>(AngularFormat::Degrees));
    _formatSelector->addItem("ГМ", static_cast<int>(AngularFormat::DegreesMinutes));
    _formatSelector->addItem("ГМС", static_cast<int>(AngularFormat::DegreesMinutesSeconds));

    layout->setRowStretch(0, 0);
    layout->addWidget(_latLabel, 1, 0);
    layout->addWidget(_latEditor, 1, 1);
    layout->addWidget(_lonLabel, 1, 2);
    layout->addWidget(_lonEditor, 1, 3);
    layout->addWidget(_altLabel, 1, 4);
    layout->addWidget(_altEditor, 1, 5);
    layout->addWidget(_formatSelector, 1, 6);
    layout->addWidget(_systemSelector, 1, 7);
    layout->setColumnStretch(8, 1);
    layout->addWidget(_copyButton, 1, 9);
    layout->addWidget(_pasteButton, 1, 10);
    layout->setRowStretch(2, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(_systemSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LatLonEditor::onSystemChanged);
    connect(_formatSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &LatLonEditor::onAngularFormatChanged);

    setLatLon(mccgeo::LatLon());

    connect(_copyButton, &QToolButton::pressed, this, &LatLonEditor::copy);
    connect(_pasteButton, &QToolButton::pressed, this, &LatLonEditor::paste);

    connect(_pasteButton, &QToolButton::pressed, this, [this]() {emit valueChanged(latLon()); });
    connect(_latEditor, &CoordinateEditor::valueChanged, this, [this]() {emit valueChanged(latLon());});
    connect(_lonEditor, &CoordinateEditor::valueChanged, this, [this]() {emit valueChanged(latLon());});

    setAltitudeVisible(false);

    resetFormat();
}

LatLonEditor::~LatLonEditor()
{
}

void LatLonEditor::resetFormat()
{
    _formatSelector->setCurrentIndex((int)_csController->angularFormat());
    _systemSelector->setCurrentIndex((int)_csController->currentSystemIndex());
    onSystemChanged();
}

double LatLonEditor::normalizeAngle180Deg(double angleDeg)
{
    angleDeg = std::fmod(angleDeg, 360.);
    return angleDeg > 180. ? angleDeg - 360. : (angleDeg < -180. ? angleDeg + 360. : angleDeg);
}

double LatLonEditor::normalizeAngle360Deg(double angleDeg)
{
    if(angleDeg > 0.0)
    {
        angleDeg = std::fmod(angleDeg, 360.0);
    }
    else if(angleDeg < 0.0)
    {
        angleDeg = std::fmod(angleDeg, 360.0) + 360.0;
    }
    return angleDeg;
}

mccgeo::LatLon LatLonEditor::latLon() const
{
    return positionWithConvertedAlt(0).latLon();
}

mccgeo::Position LatLonEditor::positionWithConvertedAlt(double alt) const
{
    mccgeo::Position converted(_latEditor->value(), _lonEditor->value(), alt);
    mccgeo::Position wgs84 = currentSystem().convertInverse(converted).position();

    if(!wgs84.isValid()) {
        wgs84.setLatitude(normalizeAngle360Deg(wgs84.latitude()));
        wgs84.setLongitude(normalizeAngle180Deg(wgs84.longitude()));
    }

    return wgs84;
}

void LatLonEditor::setEditMode(EditMode mode)
{
    switch (mode)
    {
    case EditMode::Latitude:
        _latEditor->selectInput();
        break;
    case EditMode::Longitude:
        _lonEditor->selectInput();
        break;
    }
}

bool LatLonEditor::isAltitudeVisible() const
{
    return _altEditor->isVisible();
}

bool LatLonEditor::isEditable() const
{
    return (_latEditor->isEnabled() && _lonEditor->isEnabled());
}

const CoordinateSystem& LatLonEditor::currentSystem() const
{
    return _csController->systems()[_systemIndex];
}

AngularFormat LatLonEditor::angularFormat() const
{
    return static_cast<AngularFormat>(_formatSelector->currentData().toInt());
}

void LatLonEditor::setAltitudeVisible(bool visible)
{
    _altEditor->setVisible(visible);
    _altLabel->setVisible(visible);
}

void LatLonEditor::setEditable(bool editable)
{
    _latEditor->setEnabled(editable);
    _lonEditor->setEnabled(editable);
    _altEditor->setEnabled(editable);
}

void LatLonEditor::onSystemChanged()
{
    std::size_t oldI = _systemIndex;
    std::size_t i = static_cast<std::size_t>(_systemSelector->currentData().toInt());

    _systemIndex = i;

    double alt = 0;
    if (_altEditor->isVisible()) {
        alt = _altEditor->text().toDouble();
    }

    mccgeo::Position oldPos(_latEditor->value(), _lonEditor->value(), alt);

    const auto& oldDesc = _csController->systems()[oldI];
    mccgeo::Coordinate wgs84coord = oldDesc.convertInverse(oldPos).position();

    const auto& desc = _csController->systems()[i];
    mccgeo::Position newPos = desc.convertForward(wgs84coord).position();

    bool isAngular = desc.hasAngularUnits();

    if(isAngular) {
        _latEditor->setToolTip("Редактировать широту");
        _lonEditor->setToolTip("Редактировать долготу");
    } else {
        _latEditor->setToolTip("Редактировать X-координату");
        _lonEditor->setToolTip("Редактировать Y-координату");
    }
    if (desc.converter()->vunits() == mccgeo::CoordinateUnits::None) {
        _altEditor->setToolTip("Редактировать Z-координату");
    } else {
        _altEditor->setToolTip("Редактировать высоту");
    }

    onAngularFormatChanged();

    _latEditor->setValue(newPos.latitude());
    _lonEditor->setValue(newPos.longitude());
    _altEditor->setText(QString::number(newPos.altitude(), 'f'));
    emit systemChanged();
}

void LatLonEditor::onAngularFormatChanged()
{
    const CoordinateSystem& cs = currentSystem();
    bool isAngular = cs.hasAngularUnits();
    CoordinateFormat fmt(angularFormat());
    if (!isAngular) {
        fmt.setLinear(cs.units());
    }

    if(fmt.isLinear()) {
        _formatSelector->hide();

        _latLabel->setText("X:");
        _lonLabel->setText("Y:");
        _altLabel->setText("Z:");

        //TODO: calc limits
        double d = std::numeric_limits<double>::infinity();// = mccgeo::LocalSystem::distanceLimit();

        _latEditor->setMinMax(-d, d);
        _lonEditor->setMinMax(-d, d);

        _latEditor->setCoordinateFormat(fmt);
        _lonEditor->setCoordinateFormat(fmt);
    } else {
        _formatSelector->show();

        _latLabel->setText("Ш:");
        _lonLabel->setText("Д:");
        _altLabel->setText("В:");

        _latEditor->setMinMax(-85.0, 85.0); //FIXME: 90?
        _lonEditor->setMinMax(-180.0, 180.0);

        _latEditor->setCoordinateFormat(fmt);
        _lonEditor->setCoordinateFormat(fmt);
    }
}

void LatLonEditor::copy()
{
    //TODO: refact
    const CoordinateSystem& cs = currentSystem();
    bool isAngular = cs.hasAngularUnits();
    CoordinateFormatter formatter;
    formatter.setVFormat(CoordinateFormat(cs.vunits()));
    if (isAngular) {
        formatter.setFormat(CoordinateFormat(angularFormat()));
    } else {
        formatter.setFormat(CoordinateFormat(cs.units()));
    }

    //TODO: use alt_editor
    mccgeo::LatLon latLon = this->latLon();
    QMimeData* data = formatter.makeMimeData(cs.converter(), cs.shortName(), latLon);

    QApplication::clipboard()->setMimeData(data);
}

void LatLonEditor::paste()
{
    auto coord = CoordinateFormatter::decodeFromMimeData(QApplication::clipboard()->mimeData());
    if (coord.isNone()) {
        return;
    }
    setLatLon(coord.unwrap().latLon());
}

void LatLonEditor::setLatLon(mccgeo::LatLon latLon)
{
    const CoordinateSystem& cs = currentSystem();
    mccgeo::LatLon newLatLon = cs.convertForward(latLon).latLon();

    _latEditor->setValue(newLatLon.latitude());
    _lonEditor->setValue(newLatLon.longitude());
    _altEditor->setText("0");
}

}
