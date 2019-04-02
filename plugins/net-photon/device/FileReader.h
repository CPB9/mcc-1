#pragma once
#include <decode/core/DataReader.h>
#include "mcc/net/Cmd.h"

namespace mccphoton {

class FileReader : public decode::DataReader
{
private:
    struct State;
public:
    explicit FileReader(std::unique_ptr<State>&&);
    ~FileReader();

    static bmcl::Result<bmcl::Rc<FileReader>, void> create(const mccnet::CmdPtr& cmd, const std::string& local_path, const std::string& remote_path);
    bmcl::Bytes readNext(std::size_t maxSize) override;
    std::size_t size() const override;
    std::size_t offset() const override;
    bool hasData() const override;

private:
    std::unique_ptr<State> _state;
};
}