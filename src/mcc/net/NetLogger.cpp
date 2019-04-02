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
#include "mcc/net/NetLogger.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/path/Paths.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Update_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::SystemTime);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Channel);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::LogWriteCreator);

namespace mccnet {

Logger::Logger(caf::actor_config& cfg, bool isConsole) : caf::event_based_actor(cfg), _isConsole(isConsole)
{
    join(system().groups().get_local("notes"));
    join(system().groups().get_local(":log"));
}

void Logger::on_exit()
{
    _files.clear();
}

const char* Logger::name() const { return "net.logger"; }

void Logger::handle(caf::actor_id id, bmcl::LogLevel level, const char* msg, bool hasEndl)
{
    if ((int)level > (int)bmcl::logLevel())
    {
        return;
    }
    const char* prefix;
    bmcl::ColorAttr attr;
    switch (level)
    {
    case bmcl::LogLevel::Debug:
        prefix = "DEBUG:   ";
        attr = bmcl::ColorAttr::FgBlack;
        break;
    case bmcl::LogLevel::Info:
        prefix = "INFO:    ";
        attr = bmcl::ColorAttr::FgCyan;
        break;
    case bmcl::LogLevel::Warning:
        prefix = "WARNING: ";
        attr = bmcl::ColorAttr::FgYellow;
        break;
    case bmcl::LogLevel::Critical:
        prefix = "CRITICAL:";
        attr = bmcl::ColorAttr::FgRed;
        break;
    case bmcl::LogLevel::Panic:
        prefix = "PANIC:   ";
        attr = bmcl::ColorAttr::FgRed;
        break;
    default:
        prefix = "????:    ";
    }

    char timeStr[20];
    std::time_t t = std::time(nullptr);
    std::strftime(timeStr, sizeof(timeStr), "%T", std::localtime(&t));
    bmcl::ColorStdError out;
    out << bmcl::ColorAttr::Bright << timeStr << " [" << std::setfill(' ') << std::setw(2) << id << "] ";
    out << attr << prefix << bmcl::ColorAttr::Reset << ' ';
#ifdef BMCL_HAVE_QT
    if (_isConsole)
        out << QString::fromUtf8(msg).toLocal8Bit().constData();
    else
        out << msg;
#else
    out << msg;
#endif
    if (!hasEndl)
        out << std::endl;
    if (level == bmcl::LogLevel::Panic)
        abort();
}

caf::behavior Logger::make_behavior()
{
    set_down_handler( [this](caf::down_msg&)
    {
    });

    set_default_handler(caf::print_and_drop);

    using std::chrono::milliseconds;

    return
    {
        [this](log_start_atom, const mccmsg::Channel& c, LogWriteCreator creator)
        {
            if (_files.find(c) != _files.end())
                return;
            _files.emplace(c, creator());
        }
      , [this](log_finish_atom, const mccmsg::Channel& c, milliseconds time)
        {
            const auto& f = _files.find(c);
            if (f == _files.end())
                return;
            f->second->close(time);
            _files.erase(f);
            _toOpen.erase(c);
        }
      , [this](activated_atom, const mccmsg::Channel& c, bmcl::SystemTime time)
        {
            auto f = _files.find(c);
            if (f == _files.end())
                return;
            if (_folder.empty())
            {
                _toOpen[c] = time;
                return;
            }
            f->second->open(_folder, time);
            _toOpen.erase(c);
        }
      , [this](deactivated_atom, const mccmsg::Channel& c, milliseconds time)
        {
            auto f = _files.find(c);
            if (f == _files.end())
                return;
            f->second->close(time);
        }
      , [this](atom_rcvd_bad, const mccmsg::Channel& c, milliseconds time, mccmsg::PacketPtr& p)
        {
            auto f = _files.find(c);
            if (f == _files.end())
                return;
            f->second->rcvdBad(time, p);
        }
      , [this](atom_rcvd, const mccmsg::Channel& c, milliseconds time, mccmsg::PacketPtr& p)
        {
            auto f = _files.find(c);
            if (f == _files.end())
                return;
            f->second->rcvd(time, p);
        }
      , [this](atom_sent, const mccmsg::Channel& c, milliseconds time, mccmsg::PacketPtr& p)
        {
            auto f = _files.find(c);
            if (f == _files.end())
                return;
            f->second->sent(time, p);
        }
      , [this](const mccmsg::NotificationPtr& note)
        {
        }
      , [this](caf::actor_id id, bmcl::LogLevel level, const std::string& msg)
        {
            handle(id, level, msg.c_str(), false);
        }
      , [this](std::string& virtual_file, const std::string& line)
        {
            (void)virtual_file;
            using std::string;
            std::size_t begin = line.find("id: ") + 4;
            char* end;
            caf::actor_id aid = std::strtoul(line.c_str() + begin, &end, 10);
            if (errno == ERANGE)
            {
                aid = id();
                errno = 0;
            }
            handle(aid, bmcl::LogLevel::Warning, line.c_str(), true);
        }
      , [this](caf::actor_id id, const mccmsg::tm::LogPtr& log)
        {
            const auto& data = log->data();
            handle(id, data.logLevel(), data.text().c_str(), false);
        }
      , [this](log_set_atom, const std::string& folder)
        {
            _folder = folder;
            for(auto& i: _files)
            {
                ILogWriterPtr& f = i.second;
                if (f->isOpen())
                {
                    f->open(_folder, f->startTime());
                    continue;
                }

                const auto tmp = _toOpen.find(i.first);
                if (tmp == _toOpen.end())
                    continue;

                f->open(_folder, tmp->second);
                _toOpen.erase(tmp);
            }
        }
    };
}

}
