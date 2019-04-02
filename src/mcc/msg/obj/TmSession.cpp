#include <ctime>
#include <fmt/format.h>
#include <bmcl/Result.h>
#include "mcc/msg/obj/TmSession.h"

namespace mccmsg {

TmSessionDescriptionObj::TmSessionDescriptionObj(const TmSession& name, bmcl::StringView info): _name(name), _info(info.toStdString()) {}

TmSessionDescriptionObj::TmSessionDescriptionObj(const TmSession& name
    , bmcl::StringView info
    , bmcl::StringView folder
    , const bmcl::SystemTime& started
    , const bmcl::Option<bmcl::SystemTime>& finished)
    : _name(name)
    , _info(info.toStdString())
    , _folder(folder.toStdString())
    , _started(started)
    , _finished(finished)
{
}

TmSessionDescriptionObj::TmSessionDescriptionObj
    (const TmSession& name
    , bmcl::StringView info
    , bmcl::StringView folder
    , bool hasScreenRecord
    , const Devices& devices
    , const Channels& channels
    , const bmcl::SystemTime& started
    , const bmcl::Option<bmcl::SystemTime>& finished)
    :_name(name)
    , _info(info.toStdString())
    , _folder(folder.toStdString())
    , _hasScreenRecord(hasScreenRecord)
    , _devices(devices)
    , _channels(channels)
    , _started(started)
    , _finished(finished)
{
}

const TmSession& TmSessionDescriptionObj::name() const { return _name; }
const bmcl::SystemTime& TmSessionDescriptionObj::started() const { return _started; }
const bmcl::Option<bmcl::SystemTime>& TmSessionDescriptionObj::finished() const { return _finished; }
const std::string& TmSessionDescriptionObj::info() const { return _info; }
const std::string& TmSessionDescriptionObj::folder() const { return _folder; }
const Devices& TmSessionDescriptionObj::devices() const { return _devices; }
const Channels& TmSessionDescriptionObj::channels() const { return _channels; }
bool TmSessionDescriptionObj::hasScreenRecord() const { return _hasScreenRecord; }

struct CfgName
{
    CfgName(const bmcl::Uuid& uuid, bmcl::StringView kind, bmcl::StringView info, bmcl::SystemTime started)
        : uuid(uuid), kind(kind), info(info), started(started) {}
    bmcl::Uuid       uuid;
    bmcl::StringView kind;
    bmcl::StringView info;
    bmcl::SystemTime started;
};

std::string uuidToName(bmcl::StringView kind, bmcl::Uuid uuid, bmcl::StringView info = bmcl::StringView())
{
    char timeStr[20];
    std::time_t t = std::time(nullptr);
    std::strftime(timeStr, sizeof(timeStr), "%Y%m%dT%H%M%S", std::localtime(&t));

    auto kindStr = fmt::string_view(kind.data(), kind.size());
    auto infoStr = fmt::string_view(info.data(), info.size());

    if (info.isEmpty())
         return fmt::format("{}.{}.{}", kindStr, timeStr, uuid.toStdString());
    return fmt::format("{}.{}.{}.{}", kindStr, timeStr, uuid.toStdString(), infoStr);
}

bmcl::Option<CfgName> nameToUuid(bmcl::StringView text)
{
    auto _1 = text.findFirstOf(".");
    if (_1.isNone())
        return bmcl::None;

    auto _2 = text.findFirstOf(".", _1.unwrap()+1);
    if (_2.isNone())
        return bmcl::None;

    auto _3 = text.findFirstOf(".", _2.unwrap()+1);

    bmcl::StringView str1 = text.sliceTo(_1.unwrap());//kind
    bmcl::StringView str2 = text.slice(_1.unwrap()+1, _2.unwrap());//time
    bmcl::StringView str3 = text.slice(_2.unwrap()+1, _3.unwrapOr(text.size()));//uuid
    bmcl::StringView str4;
    if (_3.isSome())
        str4 = text.sliceFrom(_3.unwrap() + 1);//info

    auto uuid = bmcl::Uuid::createFromString(str3);
    if (uuid.isErr())
        return bmcl::None;

    return CfgName(uuid.unwrap(), str1, str4, bmcl::SystemClock::now());
}

std::string TmSessionDescriptionObj::genTmSessionFile(const mccmsg::TmSession& session)
{ 
    return uuidToName("session", session);
}

std::string TmSessionDescriptionObj::genDeviceFile(const mccmsg::Device& device)
{
    return uuidToName("device", device);
}

std::string TmSessionDescriptionObj::genChannelFile(const mccmsg::Channel& channel)
{
    return uuidToName("channel", channel);
}

bmcl::Option<TmSession> TmSessionDescriptionObj::getTmSession(bmcl::StringView text)
{
    auto c = nameToUuid(text);
    if (c.isNone())
        return bmcl::None;

    const CfgName& cfg = c.unwrap();
    if (cfg.kind != "session")
        return bmcl::None;

    return TmSession(cfg.uuid);
}

bmcl::Option<Device> TmSessionDescriptionObj::getDevice(bmcl::StringView text)
{
    auto c = nameToUuid(text);
    if (c.isNone())
        return bmcl::None;

    const CfgName& cfg = c.unwrap();
    if (cfg.kind != "device")
        return bmcl::None;

    return Device(cfg.uuid);
}

bmcl::Option<Channel> TmSessionDescriptionObj::getChannel(bmcl::StringView text)
{
    auto c = nameToUuid(text);
    if (c.isNone())
        return bmcl::None;

    const CfgName& cfg = c.unwrap();
    if (cfg.kind != "channel")
        return bmcl::None;

    return Channel(cfg.uuid);
}

}