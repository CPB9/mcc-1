#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/msg/Objects.h"
#include "mcc/msg/FwdExt.h"
#include <bmcl/Option.h>
#include <bmcl/StringView.h>
#include <bmcl/TimeUtils.h>
#include <string>

namespace mccmsg {

class MCC_MSG_DECLSPEC TmSessionDescriptionObj : public mcc::RefCountable
{
public:
    TmSessionDescriptionObj(const TmSession& name
                        , bmcl::StringView info);
    TmSessionDescriptionObj(const TmSession& name
                        , bmcl::StringView info
                        , bmcl::StringView folder
                        , const bmcl::SystemTime& started
                        , const bmcl::Option<bmcl::SystemTime>& finished = bmcl::None);
    TmSessionDescriptionObj(const TmSession& name
                        , bmcl::StringView info
                        , bmcl::StringView folder
                        , bool hasScreenRecord
                        , const Devices& devices
                        , const Channels& channels
                        , const bmcl::SystemTime& started
                        , const bmcl::Option<bmcl::SystemTime>& finished = bmcl::None);
    const TmSession& name() const;
    const bmcl::SystemTime& started() const;
    const bmcl::Option<bmcl::SystemTime>& finished() const;
    const std::string& info() const;
    const std::string& folder() const;
    const Devices& devices() const;
    const Channels& channels() const;
    bool hasScreenRecord() const;

    static std::string genVideoFile();
    static std::string genTmSessionFile(const mccmsg::TmSession&);
    static std::string genDeviceFile(const mccmsg::Device&);
    static std::string genChannelFile(const mccmsg::Channel&);

    static bmcl::Option<TmSession> getTmSession(bmcl::StringView);
    static bmcl::Option<Device> getDevice(bmcl::StringView);
    static bmcl::Option<Channel> getChannel(bmcl::StringView);

private:
    TmSession _name;
    bmcl::SystemTime _started;
    bmcl::Option<bmcl::SystemTime> _finished;
    std::string _info;
    std::string _folder;
    Devices     _devices;
    Channels    _channels;
    bool        _hasScreenRecord;
};
using TmSessionDescription = bmcl::Rc<const TmSessionDescriptionObj>;
using TmSessionDescriptions = std::vector<TmSessionDescription>;

}