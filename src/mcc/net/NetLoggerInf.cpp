#include <ctime>
#include <fstream>
#include <iomanip>

#include <fmt/ostream.h>
#include <fmt/time.h>
#include <caf/send.hpp>
#include <caf/allowed_unsafe_message_type.hpp>
#include <bmcl/Logging.h>
#include <bmcl/Utils.h>
#include <bmcl/TimeUtils.h>
#include <bmcl/Logging.h>
#include <bmcl/ColorStream.h>
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/Packet.h"
#include "mcc/msg/obj/TmSession.h"
#include "mcc/net/NetLogger.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/path/Paths.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Update_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::SystemTime);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Channel);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::LogWriteCreator);

namespace mccnet {

enum class Dir : uint8_t
{
    Sent,
    Rcvd,
    RcvdBad,
};

void write(std::ofstream& f, std::chrono::milliseconds time, mccmsg::PacketPtr& p, Dir dir)
{
    if (dir == Dir::Sent)
        return;
    uint8_t d = dir == Dir::Rcvd ? 1 : 0;
    auto ms = time.count();
    //auto reader = p->reader();

    f.write((const char*)&d, sizeof(d));
    f.write((const char*)&ms, sizeof(ms));
    f.write((const char*)p->data(), p->size());
    f.flush();
}

ILogWriter::~ILogWriter()
{
}

DefaultLogWriter::DefaultLogWriter(const mccmsg::Channel& channel) : _channel(channel)
{
}

DefaultLogWriter::~DefaultLogWriter()
{
    if (_file.is_open())
        close(bmcl::toMsecs(bmcl::SystemClock::now() - _time));
}

LogWriteCreator DefaultLogWriter::creator(const mccmsg::Channel& name)
{
    return [name]() -> ILogWriterPtr { return std::make_unique<DefaultLogWriter>(name); };
}

bool DefaultLogWriter::isOpen() const
{
    return _file.is_open();
}

bmcl::SystemTime DefaultLogWriter::startTime() const
{
    return _time;
}

void DefaultLogWriter::open(bmcl::StringView folder, bmcl::SystemTime time)
{
    if (_file.is_open())
        close(bmcl::toMsecs(time - _time));

    _time = time;
    //std::time_t t = std::chrono::system_clock::to_time_t(time);
    fmt::string_view f(folder.data(), folder.size());
    _name = fmt::format("{}/{}/{}.bin", mccpath::getLogsPath(), f, mccmsg::TmSessionDescriptionObj::genChannelFile(_channel));

    _file.open(_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (_file.fail())
    {
        BMCL_DEBUG() << "Не удалось открыть лог-файл " << _name;
        return;
    }
    BMCL_DEBUG() << "Открыт лог-файл " << _name;
}

void DefaultLogWriter::close(std::chrono::milliseconds)
{
    if (!_file.is_open())
        return;
    _file.close();
    BMCL_DEBUG() << "Закрыт лог-файл " << _name;
}

void DefaultLogWriter::rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
    write(_file, time, p, Dir::Rcvd);
}

void DefaultLogWriter::rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
    write(_file, time, p, Dir::RcvdBad);
}

void DefaultLogWriter::sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p)
{
    write(_file, time, p, Dir::Sent);
}

LogSender::LogSender(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger, const LogWriteCreator& creator)
    : log(false), channel(channel), broker(broker), logger(logger), _creator(creator)
{
    caf::send_as(broker, logger, log_start_atom::value, channel, _creator);
}

LogSender::~LogSender()
{
    caf::send_as(broker, logger, log_finish_atom::value, channel, timer.passed());
}

void LogSender::changeLog(bool logState, bool isConnected)
{
    if (logState == log) return;

    log = logState;
    if (log)
        timer.start();

    if (isConnected)
    {
        if (log)
            caf::send_as(broker, logger, activated_atom::value, channel, timer.started().unwrapOr(bmcl::SystemTime()));
        else
            caf::send_as(broker, logger, deactivated_atom::value, channel, timer.passed());
    }
}

void LogSender::onRcv(const mccmsg::PacketPtr& p)
{
    if (log) caf::send_as(broker, logger, atom_rcvd::value, channel, timer.passed(), p);
}

void LogSender::onRcvBad(const void * start, std::size_t size)
{
    const uint8_t* s = (const uint8_t*)start;
    if (log) caf::send_as(broker, logger, atom_rcvd_bad::value, channel, timer.passed(), mccmsg::PacketPtr(new mccmsg::Packet(s, size)));
}

void LogSender::onRcvBad(const mccmsg::PacketPtr& pkt)
{
    if (log) caf::send_as(broker, logger, atom_rcvd_bad::value, channel, timer.passed(), pkt);
}

void LogSender::onSent(std::size_t req_id, const mccmsg::PacketPtr& pkt, const caf::error& err)
{
    (void)req_id;
    (void)err;
    if (log) caf::send_as(broker, logger, atom_sent::value, channel, timer.passed(), pkt);
}

void LogSender::onConnected()
{
    timer.start();
    if (log) caf::send_as(broker, logger, activated_atom::value, channel, timer.started().unwrapOr(bmcl::SystemTime()));
}

void LogSender::onDisconnected(const caf::error& err)
{
    (void)err;
    if (log) caf::send_as(broker, logger, deactivated_atom::value, channel, timer.passed());
}

}
