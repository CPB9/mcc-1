#include "SessionsModel.h"

#include <bmcl/TimeUtils.h>

#include "mcc/msg/obj/TmSession.h"
#include "mcc/msg/ptr/TmSession.h"

#include <QAbstractTableModel>


SessionsModel::SessionsModel(const mccuav::Rc<mccuav::UavController>& uavController, const mccuav::Rc<mccuav::ChannelsController>& channelsController) : _uavController(uavController)
, _channelsController(channelsController)
{

}

int SessionsModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return _sessions.size();
}

int SessionsModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 6;
}

QVariant SessionsModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    int column = index.column();

    if (index.row() >= _sessions.size())
        return QVariant();

    const auto& session = _sessions[index.row()];

    if (role == Qt::DisplayRole)
    {
        const char* const format = "yyyy-MM-dd HH:mm:ss.zzz";

        if (column == Columns::Info)
        {
            return QString::fromStdString(session->info());
        }
        else if (column == Columns::Started)
        {
            return QDateTime::fromMSecsSinceEpoch(bmcl::toMsecs(session->started().time_since_epoch()).count()).toString(format);
        }
        else if (column == Columns::Finished)
        {
            auto finished = session->finished();
            if (finished.isSome())
                return QDateTime::fromMSecsSinceEpoch(bmcl::toMsecs(finished->time_since_epoch()).count()).toString(format);
            else
                return "In progress";
        }
        else if (column == Columns::Folder)
        {
            return QString::fromStdString(session->folder());
        }
        else if (column == Columns::Devices)
        {
            std::string result;
            for (int i = 0; i < session->devices().size(); ++i)
            {
                result += " ";
                auto dev = _uavController->uav(session->devices()[i]);
                if (dev.isSome())
                    result += dev->deviceDescription()->info();
                else
                    result += session->devices()[i].toStdString();

                if (i != session->devices().size() - 1)
                    result += ", ";
            }

            return QString::fromStdString(result);
        }
        else if (column == Columns::Channels)
        {
            std::string result;
            for (int i = 0; i < session->channels().size(); ++i)
            {
                result += " ";
                auto ch = _channelsController->channelInformation(session->channels()[i]);
                if (ch.isSome())
                    result += ch->channelDescription()->info();
                else
                    result += session->channels()[i].toStdString();

                if (i != session->channels().size() - 1)
                    result += ", ";
            }

            return QString::fromStdString(result);
        }
    }
    if (role == SessionId)
    {
        return QVariant::fromValue(session->name());
    }

    return QVariant();
}

QVariant SessionsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (section)
    {
        case Columns::Info:     return "Info";
        case Columns::Started:  return "Started";
        case Columns::Finished: return "Finished";
        case Columns::Folder:   return "Folder";
        case Columns::Devices:  return "Devices";
        case Columns::Channels: return "Channels";
        default:                return QVariant();
    }
}

void SessionsModel::updateSession(const mccmsg::TmSessionDescription& session)
{
    auto it = std::find_if(_sessions.begin(), _sessions.end(), [&session](const mccmsg::TmSessionDescription& d) { return d->name() == session->name(); });
    if (it == _sessions.end())
    {
        beginInsertRows(QModelIndex(), _sessions.size(), _sessions.size());
        _sessions.push_back(session);
        endInsertRows();
        return;
    }
    *it = session;
    auto i = std::distance(_sessions.begin(), it);

    emit dataChanged(index(i, 0), index(i, columnCount()));
}

void SessionsModel::removeSession(const mccmsg::TmSession& session)
{
    auto it = std::find_if(_sessions.begin(), _sessions.end(), [&session](const mccmsg::TmSessionDescription& d) { return d->name() == session; });
    if (it == _sessions.end())
    {
        BMCL_WARNING() << "Попытка удалить несуществующую сессию: " << session.toStdString();
        return;
    }
    auto i = std::distance(_sessions.begin(), it);
    beginRemoveRows(QModelIndex(), i, i);
    _sessions.erase(it);
    endRemoveRows();
}
