#include <ctime>

#include "../broker/LogWriter.h"

#include <fmt/format.h>
#include <fmt/time.h>
#include <bmcl/Logging.h>
#include <bmcl/StringView.h>
#include <mavlink/mavlink_types.h>

#include "mcc/path/Paths.h"

namespace mccmav {

MavLogWriter::MavLogWriter(const std::string& name) : _baseName(name)
{
}

MavLogWriter::~MavLogWriter()
{
    if (_file.is_open())
        close(bmcl::toMsecs(bmcl::SystemClock::now() - _time));
}

mccnet::LogWriteCreator MavLogWriter::creator(const std::string& name)
{
    return [name]() -> mccnet::ILogWriterPtr { return std::make_unique<MavLogWriter>(name); };
}

bool MavLogWriter::isOpen() const
{
    return _file.is_open();
}

bmcl::SystemTime MavLogWriter::startTime() const
{
    return _time;
}

void MavLogWriter::open(bmcl::StringView folder, bmcl::SystemTime time)
{
    if (_file.is_open())
        close(bmcl::toMsecs(time - _time));

    _time = time;
    std::time_t t = std::chrono::system_clock::to_time_t(time);
    fmt::string_view f(folder.data(), folder.size());
    _name = fmt::format("{}/{}_{:%Y%m%d_%H%M%S}.mavlink", f, _baseName, *std::localtime(&t));

    _file.open(_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (_file.fail())
    {
        BMCL_DEBUG() << "Не удалось открыть лог-файл " << _name;
        return;
    }
    BMCL_DEBUG() << "Открыт лог-файл " << _name;
}

void MavLogWriter::close(std::chrono::milliseconds time)
{
    if (!_file.is_open())
        return;
    _file.close();
    BMCL_DEBUG() << "Закрыт лог-файл " << _name;
}

void MavLogWriter::rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
}

void MavLogWriter::sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
}

void MavLogWriter::rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
    auto t = _time + time;
    uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(t.time_since_epoch()).count();
    us = bmcl::htobe(us);
    _file.write((const char*)&us, sizeof(us));
    _file.write((const char*)p->data(), p->size());
    _file.flush();
}

KlvLogWriter::KlvLogWriter(const std::string& name) : _baseName(name)
{
}

KlvLogWriter::~KlvLogWriter()
{
    if (_file.is_open())
        close(bmcl::toMsecs(bmcl::SystemClock::now() - _time));
}

mccnet::LogWriteCreator KlvLogWriter::creator(const std::string& name)
{
    return [name]() -> mccnet::ILogWriterPtr { return std::make_unique<MavLogWriter>(name); };
}

bool KlvLogWriter::isOpen() const
{
    return _file.is_open();
}

bmcl::SystemTime KlvLogWriter::startTime() const
{
    return _time;
}

void KlvLogWriter::open(bmcl::StringView folder, bmcl::SystemTime time)
{
    if (_file.is_open())
        close(bmcl::toMsecs(time - _time));

    _time = time;
    std::time_t t = std::chrono::system_clock::to_time_t(time);
    fmt::string_view f(folder.data(), folder.size());
    _name = fmt::format("{}/{}_{:%Y%m%d_%H%M%S}.klv", f, _baseName, *std::localtime(&t));

    _file.open(_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (_file.fail())
    {
        BMCL_DEBUG() << "Не удалось открыть лог-файл " << _name;
        return;
    }
    BMCL_DEBUG() << "Открыт лог-файл " << _name;
}

void KlvLogWriter::close(std::chrono::milliseconds time)
{
    if (!_file.is_open())
        return;
    _file.close();
    BMCL_DEBUG() << "Закрыт лог-файл " << _name;
}

void KlvLogWriter::rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
}

void KlvLogWriter::sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
}

void KlvLogWriter::rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
    _file.write((const char*)p->data() + 7, p->size() - 7);
    _file.write("\r\n", 2);
    _file.flush();
}
}
