#include "mcc/ide/models/LogMessagesModel.h"
#include "mcc/msg/Error.h"

#include <memory>

#include <QColor>
#include <QAbstractTableModel>
#include <bmcl/Utils.h>

#include <bmcl/TimeUtils.h>

#include "mcc/uav/UavController.h"

namespace mccide {

AbstractLogMessage::AbstractLogMessage(bmcl::LogLevel logLevel)
    : _logLevel(logLevel)
{
}

AbstractLogMessage::~AbstractLogMessage()
{
}

class SystemLogMessage : public AbstractLogMessage
{
public:
    SystemLogMessage(const mccmsg::tm::LogPtr& msgPtr, mccuav::UavController* contextPtr);
};

class CmdLogMessage : public AbstractLogMessage
{
public:
    CmdLogMessage(const mccmsg::DevReqPtr& req, const bmcl::Result<mccmsg::CmdRespAnyPtr, mccmsg::ErrorDscr>& rep);


    bool canCancel() const override;
    mccmsg::RequestId cmdId() const override;

private:
    mccmsg::DevReqPtr _msgPtr;
};

const QString& AbstractLogMessage::time() const
{
    return _time;
}

const QString& AbstractLogMessage::component() const
{
    return _component;
}

const QString& AbstractLogMessage::deviceName() const
{
    return _deviceName;
}

const QString& AbstractLogMessage::message() const
{
    return _message;
}

const QColor&  AbstractLogMessage::messageColor() const
{
    return _messageColor;
}

bool AbstractLogMessage::canCancel() const
{
    return false;
}

mccmsg::RequestId AbstractLogMessage::cmdId() const
{
    return mccmsg::RequestId(-1);
}

SystemLogMessage::SystemLogMessage(const mccmsg::tm::LogPtr& msgPtr, mccuav::UavController* contextPtr)
    : AbstractLogMessage(msgPtr->data().logLevel())
{
    switch (msgPtr->data().logLevel())
    {
    case bmcl::LogLevel::Critical:
    case bmcl::LogLevel::Panic:
        _messageColor.setRgb(255, 0, 0, 255);
        break;
    case bmcl::LogLevel::Warning:
        _messageColor.setRgb(255, 255, 0, 255);
        break;
    case bmcl::LogLevel::Debug:
        _messageColor.setRgb(192, 192, 192, 192);
        break;
    case bmcl::LogLevel::Info:
        _messageColor.setRgb(0, 255, 0, 255);
        break;
    case bmcl::LogLevel::None:
        _messageColor.setRgb(255, 255, 255, 255);
        break;
    }

    _time = QDateTime::fromMSecsSinceEpoch(bmcl::toMsecs(msgPtr->data().time().time_since_epoch()).count()).toString("HH:mm:ss.zzz");
    _message = QString::fromStdString(msgPtr->data().text());

    if (msgPtr->data().device().isSome())
    {
        auto dev = contextPtr->uav(msgPtr->data().device().unwrap());
        if (dev.isSome())
        {
            _deviceName = dev->getInfo();
            _device = dev->device();
            _deviceColor = dev->color();
        }
        else
            _deviceName = msgPtr->data().device().unwrap().toQString();
    }

    _component = QString::fromStdString(msgPtr->data().sender());
}

CmdLogMessage::CmdLogMessage(const mccmsg::DevReqPtr& /*req*/, const bmcl::Result<mccmsg::CmdRespAnyPtr, mccmsg::ErrorDscr>& /*rep*/)
    : AbstractLogMessage(bmcl::LogLevel::Info)
{
    //visitor here
    //using mccmsg::cmd::State;

//     _time = mccmsg::toDateTime(req->message_time()).toString("HH:mm:ss.zzz");
//     _component = QString::fromStdString(req->device());
//     _color = QColor(Qt::darkCyan);
//     _message = QString::number(msgPtr->requestId());
//
//     auto cmd = contextPtr->getSentCmd(msgPtr->requestId());
//     if (!cmd)
//     {
//         return;
//     }
//
//     auto dev = contextPtr->device(cmd->device().toQString());
//     if (dev.isSome())
//         _device = (*dev)->getInfo();
//     else
//         _device = cmd->device().toQString();
//
//     QString commandWithArgs = QString::fromStdString(fmt::format("{} ({})", cmd->short_name(), cmd->stringify_params()));
//
//     _message = QString("%1: %2").arg(msgPtr->toString(msgPtr->state()), commandWithArgs);
//     if (msgPtr->state() == State::Failed)
//         _message += ". " + QString::fromStdString(msgPtr->error());
//
//     switch (msgPtr->state())
//     {
//     case State::Done:       _color = QColor(128, 255, 128); return;
//     case State::Failed:     _color = QColor(Qt::red); return;
//     case State::Canceled:   _color = QColor("#ffffb3"); return;
//     default: _color = Qt::white;
//     }
}

bool CmdLogMessage::canCancel() const
{
    return true;
}

mccmsg::RequestId CmdLogMessage::cmdId() const
{
    return _msgPtr->requestId();
}

LogMessagesModel::LogMessagesModel(QObject* parent)
    : QAbstractItemModel(parent)
    , _context(nullptr)
{
    startTimer(100);
}

LogMessagesModel::~LogMessagesModel()
{
}

void LogMessagesModel::setContext(mccuav::UavController* context)
{
    _context = context;

    //connect(_context, &mccuav::UavController::cmdState, this, &LogMessagesModel::onCmdState);
    connect(_context, &mccuav::UavController::log, this, &LogMessagesModel::onLog);

}

QModelIndex LogMessagesModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

int LogMessagesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(_messages.size());
}

int LogMessagesModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return 4;
}

QVariant LogMessagesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == LogMessagesModel::CanCancel)
    {
        return _messages[static_cast<size_t>(index.row())]->canCancel();
    }
    else if (role == LogMessagesModel::CmdId)
    {
        return QVariant::fromValue(_messages[static_cast<size_t>(index.row())]->cmdId());
    }
    else if (role == LogMessagesModel::LogLevel)
    {
        return static_cast<int>(_messages[static_cast<size_t>(index.row())]->logLevel());
    }

    size_t i = static_cast<size_t>(index.row());
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0:
            return _messages[i]->time();
        case 1:
            return _messages[i]->component();
        case 2:
            return _messages[i]->deviceName();
        case 3:
            return _messages[i]->message().left(_messages[i]->message().indexOf('\n'));
        }
    }
    else if (role == Qt::BackgroundColorRole || role == LogMessagesModel::MessageColorRole)
        return _messages[i]->messageColor();
    else if (role == LogMessagesModel::DeviceColorRole)
        return _messages[i]->deviceColor();
    else if (role == LogMessagesModel::DeviceRole)
    {
        if(_messages[i]->device().isSome())
            return QVariant::fromValue(_messages[i]->device().unwrap());
        else
            return QVariant();
    }
    else if (role == LogMessagesModel::TimeRole)
        return _messages[i]->time();
    else if (role == LogMessagesModel::ComponentRole)
        return _messages[i]->component();
    else if (role == LogMessagesModel::DeviceNameRole)
        return _messages[i]->deviceName();
    else if (role == LogMessagesModel::MessageRole)
        return _messages[i]->message().left(_messages[i]->message().indexOf('\n'));


    return QVariant();
}

QVariant LogMessagesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
    case 0:
        return "Время";
    case 1:
        return "Компонент";
    case 2:
        return "Аппарат";
    case 3:
        return "Сообщение";
    }

    Q_ASSERT(false);
    return "Неизвестный столбец";
}

void LogMessagesModel::addMessage(LogMessagePtr&& msgPtr)
{
    emit onLogMessage(msgPtr);
    _queue.emplace_back(std::move(msgPtr));
}

void LogMessagesModel::onLog(const mccmsg::tm::LogPtr& logMessage)
{
    addMessage(std::make_unique<SystemLogMessage>(logMessage, _context));
}

void LogMessagesModel::onCmdResponse(const mccmsg::DevReqPtr& req, const bmcl::Result<mccmsg::CmdRespAnyPtr, mccmsg::ErrorDscr>& rep)
{
    addMessage(std::make_unique<CmdLogMessage>(req, rep));
}

QModelIndex LogMessagesModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

void LogMessagesModel::timerEvent(QTimerEvent*)
{
    if (_queue.empty())
        return;

    int startRow = static_cast<int>(_messages.size());
    int endRow = startRow + static_cast<int>(_queue.size()) - 1;
    beginInsertRows(QModelIndex(), startRow, endRow);
    while (!_queue.empty())
    {
        _messages.push_back(std::move(_queue.front()));
        _queue.pop_front();
    }
    endInsertRows();
}
}
