#include "../widgets/MavlinkMonitorWidget.h"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QMimeData>
#include <QList>
#include <QTreeWidgetItem>
#include <QMetaObject>

#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/PlotData.h"

#include "mcc/msg/ParamList.h"
#include <bmcl/TimeUtils.h>

Q_DECLARE_METATYPE(mccmsg::Device);

namespace mccmav {

class MonitorModel : public QAbstractItemModel
{
    enum Roles
    {
        DeviceRole = Qt::UserRole,
        TmParamRole
    };
public:
    MonitorModel() : QAbstractItemModel()
    {
        //startTimer(500);
    }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
//         if(!parent.isValid())
//             return (int)_params.size();
        return 0;
    }

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return 3;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const override
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation != Qt::Horizontal)
            return QVariant();

        switch (section)
        {
            case 0:
                return "Параметр";
            case 1:
                return "Значение";
            case 2:
                return "Время обновления";
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

//         const auto& item = _params[index.row()];
//         if (role == Qt::DisplayRole)
//         {
//             switch (index.column())
//             {
//                 case 0:
//                     return QString::fromStdString(item.short_name());
//                 case 1:
//                     return item.value().qstringify();
//                 case 2: {
//                     const char* const format = "yyyy-MM-dd HH:mm:ss.zzz";
//                     return QDateTime::fromMSecsSinceEpoch(bmcl::toMsecs(_times[index.row()].time_since_epoch()).count()).toString(format);
//                 }
//                 default:
//                     break;
//             }
//         }
// 
//         if (role == Roles::DeviceRole)
//         {
//             return QVariant::fromValue(_device);
//         }
// 
//         if (role == Roles::TmParamRole)
//         {
//             return QVariant::fromValue(item);
//         }

        return QVariant();
    }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        return createIndex(row, column);
    }

    virtual QModelIndex parent(const QModelIndex &child) const override
    {
        return QModelIndex();
    }

    virtual QStringList mimeTypes() const override
    {
        return { mccuav::PlotData::mimeDataStr() };
    }

    virtual QMimeData * mimeData(const QModelIndexList &indexes) const override
    {
        return new QMimeData();
//         auto param = data(indexes[0], Roles::TmParamRole).value<mccmsg::TmParam>();
//         auto id = data(indexes[0], Roles::DeviceRole).value<mccmsg::Device>();
// 
//         std::string name = param.status();
//         std::string trait = param.trait();
//         std::string desc = param.short_name();
// 
//         auto plotData = new mccuav::PlotData(id,
//                                                   trait,
//                                                   name,
//                                                   desc);
// 
//         return mccuav::PlotData::packMimeData(plotData, mccuav::PlotData::mimeDataStr());
    }

    virtual Qt::DropActions supportedDropActions() const override { return Qt::IgnoreAction; }
    virtual Qt::DropActions supportedDragActions() const override { return Qt::CopyAction; }

    void clear()
    {
        beginResetModel();
        _device = mccmsg::Device();
        endResetModel();
    }

//     void tmParamList(const mccmsg::TmParamListPtr& params)
//     {
//         _device = params->device();
// 
//         for (const auto& updateParam : params->params())
//         {
//             const auto it = std::find(_params.begin(), _params.end(), updateParam);
//             if (it == _params.end())
//             {
//                 beginInsertRows(QModelIndex(), _params.size(), _params.size());
//                 _params.push_back(updateParam);
//                 _times.push_back(params->time());
//                 endInsertRows();
//             }
//             else
//             {
//                 it->set_value(updateParam.value());
//                 auto row = (int)std::distance(_params.begin(), it);
//                 _times[row] = params->time();
//             }
//         }
//     }
private:
    mccmsg::Device _device;
protected:
//     virtual void timerEvent(QTimerEvent *event) override
//     {
//         for (int row = 0; row < rowCount(); ++row)
//         {
//             emit dataChanged(index(row, 1), index(row, 2), { Qt::DisplayRole });
//         }
//     }

};

MavlinkMonitorWidget::MavlinkMonitorWidget(mccuav::UavController* uavController, QWidget* parent)
    : QWidget(parent)
    , _uav(nullptr)
    , _uavController(uavController)
{
    QVBoxLayout* layout = new QVBoxLayout;
    _widget = new QTreeView(this);
    layout->addWidget(_widget);

    setLayout(layout);

    _model = new MonitorModel();
    _widget->setModel(_model);
    _widget->setDragEnabled(true);

    using mccuav::UavController;

    auto manager = _uavController.get();
    connect(manager, &UavController::selectionChanged, this, &MavlinkMonitorWidget::selectionChanged);
    startTimer(500);
}

MavlinkMonitorWidget::~MavlinkMonitorWidget()
{
    delete _model;
}

void MavlinkMonitorWidget::selectionChanged(mccuav::Uav* uav)
{
    _uav = uav;
    _model->clear();
    _params.clear();
}

// void MavlinkMonitorWidget::tmParamList(const mccmsg::TmParamListPtr& params)
// {
//     if (!_uav)
//         return;
// 
//     if (params->device() != _uav->device())
//         return;
// 
//     _model->tmParamList(params);
// }

void MavlinkMonitorWidget::timerEvent(QTimerEvent *event)
{
    _widget->viewport()->update();
}

}