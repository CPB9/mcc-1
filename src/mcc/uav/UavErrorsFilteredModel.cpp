#include "UavErrorsFilteredModel.h"
#include "UavErrors.h"

namespace mccuav {

bool mccuav::UavErrorsFilteredModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!_hideErrors)
        return true;
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return !index.data(UavErrors::HiddenRole).toBool();
}

UavErrorsFilteredModel::UavErrorsFilteredModel()
    : _hideErrors(true)
{

}

bool UavErrorsFilteredModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == UavErrorsFilteredModel::Roles::ShowHiddenErrors)
    {
        beginResetModel();
        _hideErrors = value.toBool();
        endResetModel();
    }
    return false;
}

}