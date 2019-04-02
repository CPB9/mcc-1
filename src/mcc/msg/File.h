#pragma once
#include "mcc/Config.h"
#include <bitset>
#include <bmcl/Option.h>
#include "mcc/msg/Tm.h"
#include "mcc/msg/Cmd.h"

namespace mccmsg {

class MCC_MSG_DECLSPEC FileAttributes
{
public:
    FileAttributes(bool readable, bool writable)
    {
        _bits[read] = readable;
        _bits[write] = writable;
    }
    bool isReadable() const { return _bits[read]; }
    bool isWritable() const { return _bits[write]; }
private:
    enum Values
    {
        read = 0,
        write = 1,
    };
    std::bitset<2> _bits;
};

struct MCC_MSG_DECLSPEC File
{
    File(const std::string& file, FileAttributes attribs, bmcl::Option<std::size_t> size, bmcl::Option<std::size_t> max_size, bmcl::Option<std::size_t> crc)
        : attribs(attribs), file(file), size(size), max_size(max_size), crc(crc)
    {
    }
    FileAttributes  attribs;
    std::string     file;
    bmcl::Option<std::size_t> size;
    bmcl::Option<std::size_t> max_size;
    bmcl::Option<std::size_t> crc;
};
using Files = std::vector<File>;

class MCC_MSG_DECLSPEC CmdFileGetListResp : public CmdRespAny
{
public:
    CmdFileGetListResp(const CmdFileGetList*, Files&& files, bmcl::Option<std::string>&& ui = bmcl::None);
    ~CmdFileGetListResp() override;
    void visit(CmdRespVisitor* visitor) const override;
    const Files& files() const;
    const bmcl::Option<std::string>& ui() const;
private:
    Files _files;
    bmcl::Option<std::string> _ui;
};

class MCC_MSG_DECLSPEC CmdFileGetList : public DevReq
{
public:
    CmdFileGetList(const Device& device);
    ~CmdFileGetList();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
};

class MCC_MSG_DECLSPEC CmdFileUpload : public DevReq
{
public:
    CmdFileUpload(const Device& device, bmcl::StringView local, bmcl::StringView remote);
    ~CmdFileUpload();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const std::string& local() const;
    const std::string& remote() const;
private:
    std::string _local;
    std::string _remote;
};

class MCC_MSG_DECLSPEC CmdFileDownload : public DevReq
{
public:
    CmdFileDownload(const Device& device, bmcl::StringView local, bmcl::StringView remote);
    ~CmdFileDownload();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const std::string& local() const;
    const std::string& remote() const;
private:
    std::string _local;
    std::string _remote;
};

class MCC_MSG_DECLSPEC CmdGetFrm : public DevReq
{
public:
    CmdGetFrm(const Device& device);
    ~CmdGetFrm();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
private:
};

}
