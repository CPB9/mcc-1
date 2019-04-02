#include "mcc/ide/view/FileTransferWrapper.h"

#include <QObject>
#include <map>

#include "mcc/Config.h"
#include "mcc/uav/ExchangeService.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/messages/Cmd.h"

namespace mccide {

FileTransferWrapper::FileTransferWrapper(QObject* parent)
    : QObject(parent)
{
    connect(mccui::Context::instance()->exchangeService().get(), &mccuav::ExchangeService::cmdState, this, &FileTransferWrapper::cmdState);
}

void FileTransferWrapper::onDeviceFileLoad(const QString& service, const QString& device, const QString& localPath, const QString& remotePath, mcc::ui::ide::FileTransfer::Direction direction)
{
    Q_UNUSED(service);
    std::string          cmdName = (direction == mcc::ui::ide::FileTransfer::Direction::Up) ? "upload" : "download";
    mcc::messages::CmdParams params = { localPath.toStdString(), remotePath.toStdString() };

    auto r = mccui::Context::instance()->uavController()->sendCmd(mcc::messages::cmd::ParamList(mcc::messages::Device(device), "File", cmdName, params));
    _fileCmds.emplace(generateKey(device, localPath, remotePath), r);
}

void FileTransferWrapper::onDeviceFileLoadCancel(const QString& service, const QString& device, const QString& localPath, const QString& remotePath, const QString& reason)
{
    Q_UNUSED(service);
    Q_UNUSED(reason);
    auto it = _fileCmds.find(generateKey(device, localPath, remotePath));

    if (it == _fileCmds.end())
        return;

    mccui::Context::instance()->uavController()->cancelCmd(mcc::messages::Device(device), it->second->message_id());
}

void FileTransferWrapper::cmdState(const mcc::messages::cmd::StatePtr& state)
{
    using mcc::messages::cmd::State;

    auto it = _fileCmds.begin();
    while (it != _fileCmds.end())
    {
        if (it->second->message_id() == state->cmdId())
        {
            if (it->second->params().empty())
            {
                Q_ASSERT(false);
                return;
            }
            if (state->state() == State::Failed)
            {
                emit deviceFileLoadFailed(it->second->device().qstringify(), it->second->params()[0].asString().c_str(), QString::fromStdString(state->error()));
                _fileCmds.erase(it);
            }
            else if (state->state() == State::Canceled)
            {
                emit deviceFileLoadFailed(it->second->device().qstringify(), it->second->params()[0].asString().c_str(), "Отменено пользователем");
                _fileCmds.erase(it);
            }
            else if (state->state() == State::Done)
            {
                emit deviceFileLoaded(it->second->device().qstringify(), it->second->params()[0].asString().c_str());
                _fileCmds.erase(it);
            }
            else if(state->state() == State::InProgress)
            {
                emit fileUploadProgressChanged(it->second->device().qstringify(), it->second->params()[0].asString().c_str(), state->progress());
            }
            return;
        }
        ++it;
    }
}

std::string FileTransferWrapper::generateKey(const QString& device, const QString& local, const QString& remote) const
{
    return QString("%1:%2:%3").arg(device).arg(local).arg(remote).toStdString();
}
}
