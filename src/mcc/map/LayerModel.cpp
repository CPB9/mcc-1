#include "mcc/map/LayerModel.h"
#include "mcc/map/LayerGroup.h"

#include <QColor>

namespace mccmap {

LayerModel::LayerModel(LayerGroup* layers, QObject* parent)
    : QAbstractTableModel(parent)
    , _layers(layers)
{
    connect(layers, &LayerGroup::layerAdded, this, [this](const Layer* layer, std::size_t pos) {
        (void)layer;
        (void)pos;
        beginResetModel();
        //TODO: optimize
        endResetModel();
    });
}

LayerModel::~LayerModel()
{
}

int LayerModel::rowCount(const QModelIndex& parent) const
{
    (void)parent;
    return (int)_layers->size();
}

int LayerModel::columnCount(const QModelIndex& parent) const
{
    (void)parent;
    return 3;
}

static inline Qt::CheckState boolToState(bool flag) {
    if (flag) {
        return Qt::Checked;
    }
    return Qt::Unchecked;
}

static inline bool stateToCheck(const QVariant& state)
{
    if (state == Qt::Checked) {
        return true;
    }
    return false;
}

QVariant LayerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::ToolTipRole) {
        switch (section) {
        case 0:
            return "Видимость";
        case 1:
            return "Редактирование";
        case 2:
            return "Имя слоя";
        }
    }

    else if (role == Qt::DisplayRole) {
        if (orientation != Qt::Horizontal) {
            return section + 1;
        }
        switch (section) {
        case 0:
            return "В";
        case 1:
            return "Р";
        case 2:
            return "Имя";
        }
    }

    return QVariant();
}

QVariant LayerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Layer* desc = _layers->layerAt(index.row());
    if (role == Qt::DisplayRole && index.column() == 2) {
        return desc->name();
    }

    if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case 0:
            return boolToState(desc->isVisible());
        case 1:
            return boolToState(desc->isEditable());
        }
    }

    if (role == Qt::BackgroundRole) {
        if (_layers->isActiveLayer(index.row())) {
            return QColor(Qt::green);
        }
    }

    return QVariant();
}

Qt::ItemFlags LayerModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable;

    if (index.column() == 0) {
        f |= Qt::ItemIsUserCheckable;
        f |= Qt::ItemIsEditable;
        f |= Qt::ItemIsEnabled;
    } else if (index.column() == 1) {
        f |= Qt::ItemIsUserCheckable;
        f |= Qt::ItemIsEditable;
        f |= Qt::ItemIsEnabled;
    } else {
        f |= Qt::ItemIsEnabled;
    }

    return f;
}

bool LayerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case 0: {
            _layers->setVisibleAt(index.row(), stateToCheck(value));
            QModelIndex newIndex = createIndex(index.row(), 0);
            QModelIndex newIndex2 = createIndex(index.row(), 1);
            emit dataChanged(newIndex, newIndex2);
            return true;
        }
        case 1:
            _layers->setEditableAt(index.row(), stateToCheck(value));
            return true;
        }
    }
    return false;
}

void LayerModel::setActiveLayer(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }
    //TODO: оптимизировать
    emit beginResetModel();
    _layers->setActiveLayerAt(index.row());
    emit endResetModel();
}
}
