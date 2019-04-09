#pragma once
#include "mcc/Config.h"
#include <string>
#include <vector>
#include <bmcl/Fwd.h>
#include "mcc/msg/Calibration.h"
#include "mcc/msg/File.h"
#include "mcc/msg/GroupState.h"
#include "mcc/msg/Route.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/ptr/Message.h"
#include "mcc/msg/Packet.h"
#include <bmcl/Logging.h>

namespace mccmsg {
namespace tm {

using TmAnyPtr = bmcl::Rc<const mccmsg::TmAny>;

class MCC_MSG_DECLSPEC LogObj {
public:
    LogObj(bmcl::LogLevel logLevel, bmcl::StringView sender, const Device& device, bmcl::StringView text);
    LogObj(bmcl::LogLevel logLevel, bmcl::StringView sender, bmcl::StringView text);
    ~LogObj();

    LogObj(const LogObj& other);
    LogObj(LogObj&& other);
    LogObj& operator=(const LogObj& other);
    LogObj& operator=(LogObj&& other);

    const std::string& text() const;
    const bmcl::Option<Device>& device() const;
    const std::string& sender() const;
    bmcl::LogLevel logLevel() const;
    const bmcl::SystemTime& time() const;
private:
    bmcl::Option<Device> _device;
    std::string _text;
    std::string _sender;
    bmcl::LogLevel _logLevel;
    bmcl::SystemTime _time;
};

class MCC_MSG_DECLSPEC DumpObj {
public:
    DumpObj();
    DumpObj(const Device& device, bmcl::StringView dir, const bmcl::Option<bmcl::SystemTime>& from,
            const bmcl::Option<bmcl::SystemTime>& to, bool changesOnly);
    ~DumpObj();

    DumpObj(const DumpObj& other);
    DumpObj(DumpObj&& other);
    DumpObj& operator=(const DumpObj& other);
    DumpObj& operator=(DumpObj&& other);

    Device _device;
    std::string _dir;
    bmcl::Option<bmcl::SystemTime> _from;
    bmcl::Option<bmcl::SystemTime> _to;
    bool _changesOnly;
};

class MCC_MSG_DECLSPEC Dump_Response_Tag {
};

} // namespace tm

MCC_MSG_DECLSPEC NotificationPtr makeTm(const TmAny*);

MSG_DECLARE_REQ(tm, Dump_Request, DumpObj, Dump_Response, Dump_Response_Tag);
MSG_DECLARE_NOT(tm, Item, tm::TmAnyPtr);
MSG_DECLARE_NOT(tm, Log, tm::LogObj);

} // namespace mccmsg
