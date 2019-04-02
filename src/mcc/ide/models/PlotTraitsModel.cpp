#include "mcc/ide/models/PlotTraitsModel.h"
#include "mcc/ui/TreeItem.h"

namespace mccide {

PlotTraitsModel::PlotTraitsModel(TraitsModel* traitsModel)
    : QSortFilterProxyModel()
    , _traitsModel(traitsModel)
{
    setSourceModel(traitsModel);
}

Qt::ItemFlags PlotTraitsModel::flags(const QModelIndex& index) const
{
    QModelIndex srcModelIndex = mapToSource(index);

    auto treeItem = static_cast<TreeItem*>(srcModelIndex.internalPointer());

    if (treeItem->isTmParam())
    {
        return (sourceModel()->flags(srcModelIndex) | Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemIsEnabled);
    }

    return sourceModel()->flags(srcModelIndex);
}

QVariant PlotTraitsModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    QModelIndex srcModelIndex = mapToSource(index);
    auto treeItem = static_cast<TreeItem*>(srcModelIndex.internalPointer());

    if (role != Qt::CheckStateRole || !treeItem->isTmParam())
        return sourceModel()->data(srcModelIndex, role);

    if (srcModelIndex.column() != 0)
        return sourceModel()->data(srcModelIndex, role);

    auto tmParamItem = static_cast<TmParamTreeItem*>(treeItem);

    if (_requestedParams.contains(tmParamItem))
        return Qt::Checked;

    return Qt::Unchecked;
}

bool PlotTraitsModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if (!index.isValid())
        return false;

    QModelIndex srcModelIndex = mapToSource(index);

    if (role != Qt::CheckStateRole)
        return sourceModel()->setData(srcModelIndex, value, role);

    auto treeItem = static_cast<TreeItem*>(srcModelIndex.internalPointer());

    if (srcModelIndex.column() != 0)
        return sourceModel()->setData(srcModelIndex, value, role);

    auto tmParamItem = static_cast<TmParamTreeItem*>(treeItem);

    if (_requestedParams.contains(tmParamItem))
    {
        _requestedParams.remove(tmParamItem);

        emit tmParamSubscribeChanged(tmParamItem, false);
    }
    else
    {
        _requestedParams[tmParamItem] = index;

        emit tmParamSubscribeChanged(tmParamItem, true);
    }

    return true;
}

void PlotTraitsModel::subscribeTmParam(const QString& device, const QString& trait, const QString& name)
{
    QModelIndex sourceIndex = _traitsModel->findParam(mccmsg::Device(device), trait, name);
    if (!sourceIndex.isValid())
    {
        Q_ASSERT(false);
        return;
    }

    setData(mapFromSource(sourceIndex), true, Qt::CheckStateRole);
}

void PlotTraitsModel::clearAllParams()
{
    for (const auto& index : _requestedParams.values())
    {
        setData(index, false, Qt::CheckStateRole);
    }
}
}
