#pragma once

#include <QSortFilterProxyModel>

namespace mccuav {

class UavErrorsFilteredModel : public QSortFilterProxyModel
{
public:
    enum Roles {
        ShowHiddenErrors = Qt::UserRole + 1
    };
    UavErrorsFilteredModel();
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
private:
    bool _hideErrors;
};

}

