#pragma once
#include "mcc/Config.h"
#include <bmcl/TimeUtils.h>
#include "mcc/msg/NetVariant.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/Cmd.h"

namespace mccmsg {

class MCC_MSG_DECLSPEC CmdParamList : public DevReq
{
public:
    CmdParamList(const Device& device, bmcl::StringView trait, bmcl::StringView command, const CmdParams& params = CmdParams());
    ~CmdParamList();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const CmdParams&      params()  const;
    std::string paramsAsString(bmcl::StringView delimeter) const;
private:
    CmdParams _params;
};

class MCC_MSG_DECLSPEC CmdParamRead : public DevReq
{
public:
    CmdParamRead(const Device& device, bmcl::StringView trait, const std::vector<std::string>& vars);
    ~CmdParamRead();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const std::string& trait() const;
    const std::vector<std::string>& vars() const;
private:
    std::string _trait;
    std::vector<std::string> _vars;
};

class MCC_MSG_DECLSPEC CmdParamWrite : public DevReq
{
public:
    CmdParamWrite(const Device& device, bmcl::StringView trait, const std::vector<std::pair<std::string, NetVariant>>& vars);
    ~CmdParamWrite();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const std::string& trait() const;
    const std::vector<std::pair<std::string, NetVariant>>& vars() const;

private:
    std::string _trait;
    std::vector<std::pair<std::string, NetVariant>> _vars;
};

} // namespace std
