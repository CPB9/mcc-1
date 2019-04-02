#include "MissionPlanDelegate.h"
#include "MissionPlanModel.h"
#include "mcc/ui/Settings.h"
#include "mcc/msg/Property.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalMapper>
#include <QDoubleValidator>

using namespace mccui;
using namespace mccuav;

MissionPlanDelegate::MissionPlanDelegate(mccui::Settings* settings, MissionPlanModel* sourceModel) : QItemDelegate()
{
    _sourceModel = sourceModel;
    _moveUpButtonMapper = new QSignalMapper();
    _moveDownButtonMapper = new QSignalMapper();
    _removeButtonMapper = new QSignalMapper();

    _latitudeValidator = new QDoubleValidator(-85, 85, 10);
    _longitudeValidator = new QDoubleValidator(-180, 180, 10);
    _altitudeValidator = new QDoubleValidator(0, 20000, 10);
    _speedValidator = new QDoubleValidator(0, 2000, 10);

    QLocale loc = QLocale::c();
    loc.setNumberOptions(QLocale::RejectGroupSeparator | QLocale::OmitGroupSeparator);
    _latitudeValidator->setLocale(loc);
    _longitudeValidator->setLocale(loc);
    _altitudeValidator->setLocale(loc);
    _speedValidator->setLocale(loc);

    connect(_moveUpButtonMapper, SIGNAL(mapped(int)), _sourceModel, SLOT(moveWaypointUp(int)));
    connect(_moveDownButtonMapper, SIGNAL(mapped(int)), _sourceModel, SLOT(moveWaypointDown(int)));
    connect(_removeButtonMapper, SIGNAL(mapped(int)), _sourceModel, SLOT(removeWaypoint(int)));

//     bool isAerobot = settings->read("ide/aerobotMode", false).toBool();
//     if (!isAerobot)
//     {
//         bool isSimple = settings->read("ide/simpleMode", false).toBool();
//         if (!isSimple)
//             _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Только для чтения", mccuav::WaypointType::ReadOnly));
// 
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Цель", WaypointType::Target));
// 
//         if (!isSimple)
//         {
//             _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Парашют", WaypointType::Parashute));
//             _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Разворот назад", WaypointType::TurnBack));
//             _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Разворот вперед", WaypointType::TurnForward));
//             _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Ожидание", WaypointType::Waiting));
//         }
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Переход", WaypointType::SwitchRoute));
// 
//         if (!isSimple)
//             _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Дом", WaypointType::Home));
// 
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Посадка", WaypointType::Landing));
//     }
//     else
//     {
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Рейнольдс", WaypointType::Reynolds));
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Формация", WaypointType::Formation));
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Змейка", WaypointType::Snake));
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Петля", WaypointType::Loop));
//         _waypointTypes.append(QPair<QString, mccuav::WaypointType>("Цель", WaypointType::Target));
//     }
}

MissionPlanDelegate::~MissionPlanDelegate()
{
    delete _latitudeValidator;
    delete _longitudeValidator;
    delete _altitudeValidator;
    delete _speedValidator;
    delete _moveUpButtonMapper;
    delete _moveDownButtonMapper;
    delete _removeButtonMapper;
}


QWidget* MissionPlanDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
        return new QWidget();

    MissionPlanModel::Columns column = static_cast<MissionPlanModel::Columns>(index.column());

    switch (column)
    {
    case MissionPlanModel::Columns::Latitude:
    {
        QLineEdit* editor = new QLineEdit(parent);
        editor->setValidator(_latitudeValidator);
        return editor;
    }
    case MissionPlanModel::Columns::Longitude:
    {
        QLineEdit* editor = new QLineEdit(parent);
        editor->setValidator(_longitudeValidator);
        return editor;
    }
    case MissionPlanModel::Columns::Altitude:
    {
        QLineEdit* editor = new QLineEdit(parent);
        editor->setValidator(_altitudeValidator);
        return editor;
    }
    case MissionPlanModel::Columns::Speed:
    {
        QLineEdit* editor = new QLineEdit(parent);
        editor->setValidator(_speedValidator);
        return editor;
    }
    default:
        break;
    }

    return QItemDelegate::createEditor(parent, option, index);
}

void MissionPlanDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    MissionPlanModel::Columns column = static_cast<MissionPlanModel::Columns>(index.column());
    switch (column)
    {
    case MissionPlanModel::Columns::Latitude:
    case MissionPlanModel::Columns::Longitude:
    case MissionPlanModel::Columns::Altitude:
    case MissionPlanModel::Columns::Speed:
    {
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);

        QString fooDouble = lineEdit->text();

        bool ok = false;
        double value = fooDouble.toDouble(&ok);
        if (!ok)
        {
            fooDouble.replace(",", ".");
            value = fooDouble.toDouble();
        }

        model->setData(index, value, MissionPlanModel::roleForColumn(column));
        break;
    }
    case MissionPlanModel::Columns::WaypointType:
    {
        QComboBox* comboEditor = static_cast<QComboBox*>(editor);

//         WaypointFlags flags;
//         for (int i = 0; i < comboEditor->count(); ++i)
//         {
//             if (comboEditor->itemData(i, Qt::CheckStateRole) == Qt::Checked)
//             {
//                 flags |= _waypointTypes[i].second;
//             }
//         }

        //model->setData(index, static_cast<uint>(flags), MissionPlanModel::roleForColumn(column));
    }
    default:
        break;
    }
}

void MissionPlanDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void MissionPlanDelegate::commitComboBox(QStandardItem* item)
{
    QWidget* editor = static_cast<QWidget*>(item->model()->parent());
    commitData(editor);
}
