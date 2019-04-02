#pragma once

#include "mcc/uav/Structs.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ide/Fwd.h"
#include "mcc/uav/Fwd.h"

#include <QItemDelegate>
#include <QVector>
#include <QPair>
#include <QString>

class QStandardItem;
class QStandardItemModel;
class QDoubleValidator;
class QSignalMapper;
class MissionPlanModel;

class MissionPlanDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    MissionPlanDelegate(mccui::Settings* settings, MissionPlanModel* sourceModel);
    ~MissionPlanDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private slots:
    void commitComboBox(QStandardItem* item);

private:
    MissionPlanModel* _sourceModel;
    QDoubleValidator* _latitudeValidator;
    QDoubleValidator* _longitudeValidator;
    QDoubleValidator* _altitudeValidator;
    QDoubleValidator* _speedValidator;

    QSignalMapper* _moveUpButtonMapper;
    QSignalMapper* _moveDownButtonMapper;
    QSignalMapper* _removeButtonMapper;

    mutable QVector<QStandardItemModel*> _models;
    //QVector<QPair<QString, mccuav::WaypointType>> _waypointTypes;
};
