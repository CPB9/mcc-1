#pragma once

#include <QAbstractTableModel>

#include "mcc/msg/Objects.h"

#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ExchangeService.h"

class SessionsModel : public QAbstractTableModel
{
private:
    enum Columns
    {
        Info,
        Started,
        Finished,
        Folder,
        Devices,
        Channels
    };

public:
    SessionsModel(const mccuav::Rc<mccuav::UavController>& uavController, const mccuav::Rc<mccuav::ChannelsController>& channelsController);

    enum Roles
    {
        SessionId = Qt::UserRole + 1
    };

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


    void updateSession(const mccmsg::TmSessionDescription& session);
    void removeSession(const mccmsg::TmSession& session);
private:
    std::vector<mccmsg::TmSessionDescription> _sessions;
    mccuav::Rc<mccuav::UavController> _uavController;
    mccuav::Rc<mccuav::ChannelsController> _channelsController;
};

Q_DECLARE_METATYPE(mccmsg::TmSession);
