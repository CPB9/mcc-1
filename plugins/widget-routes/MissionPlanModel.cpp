#include "MissionPlanModel.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/uav/Route.h"
#include "mcc/geo/Coordinate.h"

#include <QString>
#include <QFont>

using mccui::CoordinateSystemController;

MissionPlanModel::MissionPlanModel(const mccui::CoordinateSystemController* csController)
    : _route(nullptr)
    , _enabled(false)
    , _activePoint(-1)
    , _csController(csController)
{
    connect(_csController.get(), &CoordinateSystemController::changed, this, [this](){
        beginResetModel();
        endResetModel();
    });
}

MissionPlanModel::~MissionPlanModel()
{
}

int MissionPlanModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);
    if (_route == nullptr)
        return 0;

    return _route->waypointsCount();
}

int MissionPlanModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);
    return static_cast<int>(Columns::Count);
}

QVariant MissionPlanModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    Columns column = static_cast<Columns>(index.column());
    auto waypoint = _route->waypointAt(index.row());

    if (role == Qt::FontRole && index.row() == _activePoint)
    {
        QFont f;
        f.setBold(true);
        return f;
    }

    if (role == Qt::BackgroundRole)
    {
        if (column == Columns::Altitude)
        {
            if (!_route->isWaypointAltitudeValid(waypoint))
                return QColor(Qt::red);
        }
        else if (column == Columns::Speed)
        {
            if (!_route->isWaypointVelocityValid(waypoint))
                return QColor(Qt::red);
        }

        if(_route->isPointSelected(index.row()))
            return QColor("#0DE6DF");
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole)
        return QVariant();

    const mccgeo::CoordinateConverter* conv = _csController->currentConverter();

    const mccgeo::Position& pos = waypoint.position;

    //HACK: координаты нельзя редактировать отдельно, они связаны, колонки лат лон алт нада объединить
    switch (column)
    {
    case Columns::Index:
        return index.row() + 1;
    case Columns::Latitude:
        if (role == Qt::EditRole) {
            return QString::number(conv->convertForward(pos).position().latitude());
        } else {
            return _csController->formatValue(pos.latitude(), _csController->format());
        }
    case Columns::Longitude:
        if (role == Qt::EditRole) {
            return QString::number(conv->convertForward(pos).position().longitude());
        } else {
            return _csController->formatValue(pos.longitude(), _csController->format());
        }
    case Columns::Altitude:
        return QString::number(conv->convertForward(pos).position().altitude(), 'f', 0);
    case Columns::Speed:
        return QString::number(waypoint.speed, 'f', 0);
//     case Columns::WaypointType:
//         return static_cast<int>(waypoint.flags);
    default:
        break;
    }

    return QVariant();
}

Qt::ItemFlags MissionPlanModel::flags(const QModelIndex& index) const
{
    if (index.column() > 2 && _enabled)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractItemModel::flags(index);
}

QVariant MissionPlanModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    bool angularSystem = _csController->format().isAngular();

    Q_UNUSED(role);
    if (orientation != Qt::Orientation::Horizontal)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Columns column = static_cast<Columns>(section);

    switch (column)
    {
    case Columns::Index:
        return "#";
    case Columns::Latitude:
        if(angularSystem)
            return "Широта";
        else
            return "X";
    case Columns::Longitude:
        if(angularSystem)
            return "Долгота";
        else
            return "Y";
    case Columns::Altitude:
        return "Высота";
    case Columns::Speed:
        return "Скорость";
    case Columns::WaypointType:
        return "Флаги";
    default:
        break;
    }

    return QVariant();
}

bool MissionPlanModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
        return false;

    //Columns column = static_cast<Columns>(index.column());

    mccmsg::Waypoint wp = _route->waypointAt(index.row());
    const mccgeo::CoordinateConverter* conv = _csController->currentConverter();

    if (role == roleForColumn(Columns::Latitude)) {
        mccgeo::Position inverse = conv->convertForward(wp.position).position();
        inverse.latitude() = value.toDouble();
        wp.position = conv->convertInverse(inverse).position();
    } else if (role == roleForColumn(Columns::Longitude)) {
        mccgeo::Position inverse = conv->convertForward(wp.position).position();
        inverse.longitude() = value.toDouble();
        wp.position = conv->convertInverse(inverse).position();
    } else if (role == roleForColumn(Columns::Altitude)) {
        mccgeo::Position inverse = conv->convertForward(wp.position).position();
        inverse.altitude() = value.toDouble();
        wp.position = conv->convertInverse(inverse).position();
    } else if (role == roleForColumn(Columns::Speed)) {
        wp.speed = value.toDouble();
    } else if (role == roleForColumn(Columns::WaypointType)) {
//        wp.flags = static_cast<WaypointType>(value.toInt());
    }

    _route->setWaypoint(wp, index.row());

    return true;
}

int MissionPlanModel::roleForColumn(Columns column)
{
    return Qt::UserRole + static_cast<int>(column);
}

mccuav::Route::EditMode MissionPlanModel::columnToEditMode(Columns column)
{
    switch (column)
    {
        case MissionPlanModel::Columns::Latitude: return mccuav::Route::EditMode::Latitude;
        case MissionPlanModel::Columns::Longitude: return mccuav::Route::EditMode::Longitude;
        case MissionPlanModel::Columns::Altitude: return mccuav::Route::EditMode::Altitude;
        case MissionPlanModel::Columns::Speed: return mccuav::Route::EditMode::Speed;
        case MissionPlanModel::Columns::WaypointType: return mccuav::Route::EditMode::Flags;
        default:
            return mccuav::Route::EditMode::Latitude;
    }
}

void MissionPlanModel::setRoute(mccuav::Route* route)
{
    using mccuav::Route;

    if (_route != nullptr)
        disconnect(_route, 0, this, 0);

    beginResetModel();
    _route = route;
    if (_route && _route->activePointIndex().isSome())
    {
        _activePoint = (int)_route->activePointIndex().unwrap();
    }
    else
    {
        _activePoint = -1;
    }
    endResetModel();

    if (_route == nullptr)
        return;

    connect(_route, &Route::waypointChanged, this,
        [this](const mccmsg::Waypoint& waypoint, int index)
    {
        Q_UNUSED(waypoint);
        dataChanged(this->index(index, 0), this->index(index, columnCount()));
    })
        ;

    connect(_route, &Route::waypointOnlyAltChanged, this,
            [this](const mccmsg::Waypoint& waypoint, int index)
    {
        dataChanged(this->index(index, 0), this->index(index, columnCount()));
    }
    );

    connect(_route, &Route::waypointInserted, this,
        [this](const mccmsg::Waypoint& waypoint, int index)
    {
        Q_UNUSED(waypoint);
        Q_UNUSED(index);
        beginResetModel();
        endResetModel();
    })
        ;

    connect(_route, &Route::waypointRemoved, this,
        [this](int index)
    {
        Q_UNUSED(index);
        beginResetModel();
        endResetModel();
    });

    connect(_route, &Route::waypointMoved, this,
        [this](int oldIndex, int newIndex)
    {
        Q_UNUSED(oldIndex);
        Q_UNUSED(newIndex);
        beginResetModel();
        endResetModel();
    });

    connect(_route, &Route::allWaypointsChanged, this, [this]()
    {
        beginResetModel();
        endResetModel();
    });

    connect(_route, &Route::activeWaypointChanged, this,
        [this](bmcl::Option<std::size_t> index)
        {
            if (index.isSome())
                _activePoint = (int)index.unwrap();
            else
                _activePoint = -1;

            beginResetModel();
            endResetModel();
        }
    );

    connect(_route, &Route::selectedWaypointsChanged, this,
            [this]()
            {
                emit dataChanged(index(0, 0), index(rowCount(), columnCount()), QVector<int>() << Qt::BackgroundRole);
            }
    );
}

void MissionPlanModel::moveWaypointUp(int index)
{
    beginResetModel();
    _route->moveWaypointUp(index);
    endResetModel();
}

void MissionPlanModel::moveWaypointDown(int index)
{
    beginResetModel();
    _route->moveWaypointDown(index);
    endResetModel();
}

void MissionPlanModel::removeWaypoint(int index)
{
    beginResetModel();
    _route->removeWaypoint(index);
    endResetModel();
}

void MissionPlanModel::setEnabled(bool mode)
{
    _enabled = mode;
    beginResetModel();
    endResetModel();
}

void MissionPlanModel::setEmptyRoute()
{
    setRoute(nullptr);
}

void MissionPlanModel::setActivePoint(int point)
{
    BMCL_UNUSED(point);
//                 _activePoint = point;
//                 beginResetModel();
//                 endResetModel();
}

bool MissionPlanModel::isEnabled() const
{
    return _enabled;
}
