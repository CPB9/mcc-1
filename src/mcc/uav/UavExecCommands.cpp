#include "mcc/uav/UavExecCommands.h"

#include <bmcl/Logging.h>

namespace mccuav {

UavCommand::UavCommand(const mccmsg::DevReqPtr& cmd)
{
    _command = cmd;
    _infoCached = QString::fromStdString(_command->info());
}

void UavCommand::setState(const mccmsg::Request_StatePtr& state)
{
    _state = state;
}

mccmsg::RequestId UavCommand::id() const
{
    return _command->requestId();
}

const QString& UavCommand::info() const
{
    return _infoCached;
}

uint8_t UavCommand::progress() const
{
    if (!_state.isNull())
        return _state->progress();
    return 0;
}

const std::vector<UavCommand>& UavExecCommands::commands() const
{
    return _currentCommands;
}

bmcl::Option<const UavCommand&> UavExecCommands::command(mccmsg::RequestId id) const
{
    auto it = std::find_if(_currentCommands.begin(), _currentCommands.end(), [&id](const UavCommand& c) { return id == c.id(); });
    if(it == _currentCommands.end())
        return bmcl::Option<const UavCommand&>();

    return bmcl::Option<const UavCommand&>(*it);
}

std::vector<UavCommand>::iterator UavExecCommands::findCommand(const mccmsg::DevReqPtr& cmd)
{
    return std::find_if(_currentCommands.begin(), _currentCommands.end(), [&cmd](const UavCommand& c) { return cmd->requestId() == c.id(); });
}

UavExecCommands::UavExecCommands(QObject* parent)
    : QObject (parent)
{}

void UavExecCommands::addCommand(const mccmsg::DevReqPtr& cmd)
{
    _currentCommands.push_back(cmd);

    emit commandsChanged();
}

void UavExecCommands::removeCommand(const mccmsg::DevReqPtr& cmd)
{
    const auto it = findCommand(cmd);
    if (it == _currentCommands.end())
    {
        return;
    }
    _currentCommands.erase(it);

    emit commandsChanged();
}

void UavExecCommands::updateCommand(const mccmsg::DevReqPtr& cmd, const mccmsg::Request_StatePtr& state)
{
    auto it = findCommand(cmd);
    if (it == _currentCommands.end())
    {
        return;
    }
    it->setState(state);
    emit commandUpdated(it->id());
}

}
