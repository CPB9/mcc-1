#pragma once

#include "mcc/net/NetLoggerInf.h"

namespace mccmav {

class MavLogWriter : public mccnet::ILogWriter
{
public:
    explicit MavLogWriter(const std::string& name);
    ~MavLogWriter() override;
    bool isOpen() const override;
    bmcl::SystemTime startTime() const override;
    void open(bmcl::StringView folder, bmcl::SystemTime time) override;
    void close(std::chrono::milliseconds time) override;
    void sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    void rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    void rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    static mccnet::LogWriteCreator creator(const std::string& name);
private:
    std::string     _name;
    std::string     _baseName;
    std::ofstream   _file;
    bmcl::SystemTime   _time;
};

class KlvLogWriter : public mccnet::ILogWriter
{
public:
    explicit KlvLogWriter(const std::string& name);
    ~KlvLogWriter() override;
    bool isOpen() const override;
    bmcl::SystemTime startTime() const override;
    void open(bmcl::StringView folder, bmcl::SystemTime time) override;
    void close(std::chrono::milliseconds time) override;
    void sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    void rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    void rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    static mccnet::LogWriteCreator creator(const std::string& name);
private:
    std::string     _name;
    std::string     _baseName;
    std::ofstream   _file;
    bmcl::SystemTime   _time;
};
}
