#include "ChannelsListModel.h"

#include "mcc/uav/ChannelsController.h"
#include "mcc/ide/view/NetStatisticsWidget.h"

#include <algorithm>

ChannelsListModel::ChannelsListModel(mccuav::ChannelsController* chanController, QObject* parent)
    : QAbstractListModel(parent)
    , _visibleAmount(3)
    , _chanController(chanController)
{
    prepareStatsWidgets();

    connect(_chanController.get(), &mccuav::ChannelsController::channelsChanged,
            this, &ChannelsListModel::updateChannels);
    connect(_chanController.get(), &mccuav::ChannelsController::channelActiveChanged,
            this, &ChannelsListModel::updateChannels);

    connect(_chanController.get(), &mccuav::ChannelsController::channelStatsUpdated,
            this, &ChannelsListModel::updateChannelsStats);
}

ChannelsListModel::~ChannelsListModel()
{
    for(auto w : _allStatsWidgets)
        delete w;
}

int ChannelsListModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_visibleChannels.size());
}

QVariant ChannelsListModel::data(const QModelIndex& indexx, int role) const
{
    size_t index = static_cast<size_t>(indexx.row());
    if (!indexx.isValid() || index >= _visibleChannels.size())
        return QVariant();

    const mccmsg::Channel& channel = _visibleChannels[index];

    if(role == Qt::DisplayRole)
    {
        auto chInfo = _chanController->channelInformation(channel);
        if(chInfo.isNone())
            return QVariant();

        if(chInfo->channelDescription().isSome())
            return QString::fromStdString(chInfo->channelDescription()->info());
        else
            return chInfo->channel().toQString();
    }
    else if(role == StatsWidgetRole)
    {
        return QVariant::fromValue(_visibleStatsWidgets.at(channel));
    }

    return QVariant();
}

void ChannelsListModel::setVisibleAmount(size_t amount)
{
    if(_visibleAmount == amount)
        return;

    prepareStatsWidgets();
    updateChannels();
}

bool ChannelsListModel::hasMoreChannels() const
{
    return (_activeChannels.size() > _visibleAmount);
}

void ChannelsListModel::updateChannels()
{
    beginResetModel();

    const mccuav::ChannelInformations& channelInfos = _chanController->channelInformations();

    _activeChannels.clear();
    _activeChannels.reserve(channelInfos.size());

    _visibleChannels.clear();
    _visibleChannels.reserve(_visibleAmount);

    _visibleStatsWidgets.clear();

    for(auto& chInfo : channelInfos)
    {
        if(chInfo.isActive())
            _activeChannels.push_back(chInfo.channel());
    }

    std::sort(_activeChannels.begin(), _activeChannels.end(),
              [channelInfos](const mccmsg::Channel& ch1, const mccmsg::Channel& ch2)
    {
        auto it1 = std::find_if(channelInfos.begin(), channelInfos.end(),
                                [&ch1](const mccuav::ChannelInformation& info) { return info.channel() == ch1; });
        auto it2 = std::find_if(channelInfos.begin(), channelInfos.end(),
                                [&ch2](const mccuav::ChannelInformation& info) { return info.channel() == ch2; });

        if(it1->channelDescription().isSome() && it2->channelDescription().isSome())
            return it1->channelDescription()->info() < it2->channelDescription()->info();

        return false;
    });

    size_t copyAmount(std::min(_visibleAmount, _activeChannels.size()));
    for(size_t i = 0; i < copyAmount; ++i)
    {
        _visibleChannels.push_back(_activeChannels[i]);
        _visibleStatsWidgets[_activeChannels[i]] = _allStatsWidgets[i];
    }

    endResetModel();
}

void ChannelsListModel::updateChannelsStats(const mccmsg::Channel& channel, const mccmsg::StatChannel& stat)
{
    auto it = _visibleStatsWidgets.find(channel);
    if(it == _visibleStatsWidgets.end())
        return;

    size_t ind(_visibleChannels.size());
    for(size_t i = 0; i < ind; ++i)
        if(channel == _visibleChannels[i])
        {
            ind = i;
            break;
        }
    if(ind == _visibleChannels.size())
        return;

    it->second->updateStats(stat._sent, stat._rcvd, stat._bad);

    QModelIndex mIndex = index(static_cast<int>(ind), 0);
    emit dataChanged(mIndex, mIndex);
}

void ChannelsListModel::prepareStatsWidgets()
{
    for(auto w : _allStatsWidgets)
        delete w;

    for(size_t i = 0; i < _visibleAmount; ++i)
    {
        mccide::NetStatisticsWidget* widget = new mccide::NetStatisticsWidget();
        widget->setFixedSize(60, 40);
        widget->setFocusPolicy(Qt::ClickFocus);
        widget->setStyleSheet("border: 0px; "
                              "background-color: rgba(0, 0, 0, 255);"
                              "color: white;");
        widget->setTextColor(Qt::white);
        QFont f = widget->textFont();
        f.setPixelSize(9);
        widget->setTextFont(f);

        widget->showDetails(false);

        _allStatsWidgets.emplace_back(std::move(widget));
    }
}
