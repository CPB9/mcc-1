#pragma once

#include "mcc/Config.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"

#include <map>
#include <vector>
#include <QAbstractListModel>

namespace mccide {
class NetStatisticsWidget;
};

class ChannelsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        StatsWidgetRole = Qt::UserRole + 1,
    };

    explicit ChannelsListModel(mccuav::ChannelsController* chanController, QObject *parent = nullptr);
    ~ChannelsListModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setVisibleAmount(size_t amount);
    size_t visibleAMount() const {return _visibleAmount;}

    bool hasMoreChannels() const;

public slots:
    void updateChannels();

private slots:
    void updateChannelsStats(const mccmsg::Channel& channel, const mccmsg::StatChannel& stat);

private:
    void prepareStatsWidgets();

    mccmsg::Channels                        _activeChannels;
    mccmsg::Channels                        _visibleChannels;
    size_t                                  _visibleAmount;

    std::vector
    <mccide::NetStatisticsWidget*>          _allStatsWidgets;
    std::map<mccmsg::Channel,
    mccide::NetStatisticsWidget*>           _visibleStatsWidgets;
    mccui::Rc<mccuav::ChannelsController>    _chanController;

    Q_DISABLE_COPY(ChannelsListModel)
};
