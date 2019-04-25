#include <bmcl/StringView.h>
#include "mcc/msg/ptr/Tm.h"


namespace mccmsg {
namespace tm {

LogObj::LogObj(bmcl::LogLevel logLevel, bmcl::StringView sender, const Device& device, bmcl::StringView text)
    : _logLevel(logLevel), _device(device), _text(text.toStdString()), _sender(sender.toStdString()), _time(bmcl::SystemClock::now()) {}
LogObj::LogObj(bmcl::LogLevel logLevel, bmcl::StringView sender, bmcl::StringView text)
    : _logLevel(logLevel), _text(text.toStdString()), _sender(sender.toStdString()), _time(bmcl::SystemClock::now()) {}
LogObj::~LogObj() {}

const std::string& LogObj::text() const { return _text; }
const bmcl::Option<Device>& LogObj::device() const { return _device; }
const std::string& LogObj::sender() const { return _sender; }
bmcl::LogLevel LogObj::logLevel() const { return _logLevel; }
const bmcl::SystemTime& LogObj::time() const { return _time; }

LogObj::LogObj(const LogObj& other)
    : _device(other._device)
    , _text(other._text)
    , _sender(other._sender)
    , _logLevel(other._logLevel)
{
}

LogObj::LogObj(LogObj&& other)
    : _device(std::move(other._device))
    , _text(std::move(other._text))
    , _sender(std::move(other._sender))
    , _logLevel(std::move(other._logLevel))
{
}

LogObj& LogObj::operator=(const LogObj& other)
{
    _device = other._device;
    _text = other._text;
    _sender = other._sender;
    _logLevel = other._logLevel;
    return *this;
}

LogObj& LogObj::operator=(LogObj&& other)
{
    _device = std::move(other._device);
    _text = std::move(other._text);
    _sender = std::move(other._sender);
    _logLevel = std::move(other._logLevel);
    return *this;
}

}
}
