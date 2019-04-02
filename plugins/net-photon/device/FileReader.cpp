#include <QFile>
#include <QByteArray>
#include <bmcl/Result.h>
#include <bmcl/Utils.h>
#include "../device/FileReader.h"

namespace mccphoton {

struct FileReader::State
{
    QFile file;
    std::size_t offset = 0;
    std::size_t size = 1;
    QByteArray block;
    mccnet::CmdPtr cmd;
};

FileReader::FileReader(std::unique_ptr<State>&& state) : _state(std::move(state))
{
    _state->cmd->sendProgress(0);
}

FileReader::~FileReader()
{
}

bmcl::Bytes FileReader::readNext(std::size_t maxSize)
{
    _state->cmd->sendProgress(100 * _state->offset / _state->size);
    _state->block = _state->file.read(maxSize);
    _state->offset += _state->block.size();
    return bmcl::Bytes((const uint8_t*)_state->block.data(), _state->block.size());
}

std::size_t FileReader::size() const
{
    return _state->file.size();
}

std::size_t FileReader::offset() const
{
    return _state->offset;
}

bool FileReader::hasData() const
{
    return _state->file.bytesAvailable() > 0;
}

bmcl::Result<bmcl::Rc<FileReader>, void> FileReader::create(const mccnet::CmdPtr& cmd, const std::string& path, const std::string& remote_path)
{
    (void)remote_path;
    auto p = std::make_unique<State>();

    p->file.setFileName(QString::fromStdString(path));
    if (!p->file.open(QIODevice::ReadOnly))
    {
        cmd->sendFailed(mccmsg::Error::CantOpen);
        return bmcl::Result<bmcl::Rc<FileReader>, void>();
    }

    std::size_t size = p->file.size();
    if (size == 0)
    {
        cmd->sendFailed(mccmsg::Error::InconsistentData);
        return bmcl::Result<bmcl::Rc<FileReader>, void>();
    }

    p->offset = 0;
    p->size = size;
    p->cmd = cmd;
    bmcl::Rc<FileReader> fr = new FileReader(std::move(p));
    return fr;
}
}
