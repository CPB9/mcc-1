#pragma once
#include <vector>
#include <QObject>
#include "mcc/Rc.h"
#include "mcc/msg/Cmd.h"
#include "mcc/msg/Msg.h"

namespace mccuav {

class MCC_UAV_DECLSPEC UavCommand
{
public:
    UavCommand(const mccmsg::DevReqPtr& cmd);
    void setState(const mccmsg::Request_StatePtr& state);
    mccmsg::RequestId id() const;

    const QString& info() const;
    uint8_t progress() const;
private:
    mccmsg::DevReqPtr _command;
    mccmsg::Request_StatePtr _state;
    QString _infoCached;
};

class MCC_UAV_DECLSPEC UavExecCommands : public QObject
{
    Q_OBJECT

public:
    explicit UavExecCommands(QObject* parent = nullptr);

    void addCommand(const mccmsg::DevReqPtr& cmd);
    void removeCommand(const mccmsg::DevReqPtr& cmd);
    void updateCommand(const mccmsg::DevReqPtr& cmd, const mccmsg::Request_StatePtr& state);

    const std::vector<UavCommand>& commands() const;
    bmcl::Option<const UavCommand&> command(mccmsg::RequestId id) const;

signals:
    void commandsChanged();
    void commandUpdated(mccmsg::RequestId id);

private:
    std::vector<UavCommand>::iterator findCommand(const mccmsg::DevReqPtr& cmd);
private:
    std::vector<UavCommand> _currentCommands;

    Q_DISABLE_COPY(UavExecCommands)
};

}

