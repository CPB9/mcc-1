#pragma once

#include "mcc/Config.h"

#include <QSortFilterProxyModel>
#include <QMap>
#include <QModelIndex>

namespace mccide {

class TraitsModel;
class TmParamTreeItem;

class MCC_IDE_DECLSPEC PlotTraitsModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit PlotTraitsModel(TraitsModel* traitsModel);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void subscribeTmParam(const QString& device, const QString& trait, const QString& name);

public slots:
    void clearAllParams();

signals:
    void tmParamSubscribeChanged(TmParamTreeItem* item, bool subscribe);

private:
    QMap<TmParamTreeItem*, QModelIndex> _requestedParams;
    TraitsModel* _traitsModel;
};
}
