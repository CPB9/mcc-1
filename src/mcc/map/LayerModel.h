#pragma once

#include "mcc/Config.h"
#include "mcc/map/Rc.h"

#include <QAbstractTableModel>

namespace mccmap {

class LayerGroup;

class MCC_MAP_DECLSPEC LayerModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit LayerModel(LayerGroup* layers, QObject* parent = nullptr);
    ~LayerModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void setActiveLayer(const QModelIndex& index);

private:
    Rc<LayerGroup> _layers;
};
}
