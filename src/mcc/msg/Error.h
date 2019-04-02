#pragma once
#include "mcc/Config.h"
#include <string>

class QString;
namespace bmcl { class StringView; }

namespace mccmsg
{

enum Error : uint8_t
{
    CoreDisconnected = 1,
    Canceled,
    CantCancel,
    CmdUnknownTrait,
    CmdUnknown,
    CmdIncorrect,
    CmdFailed,
    CmdOtherOngoing,
    GroupUnknown,
    GroupWithoutLeader,
    GroupLeaderUnknown,
    GroupNotSet,
    GroupUnreachable,
    ChannelUnknown,
    ChannelClosed,
    ChannelError,
    ChannelCantShare,
    DeviceUnregistered,
    DeviceUnknown,
    DeviceUiUnknown,
    DeviceInactive,
    NoChannelAvailable,
    Timeout,
    CantRegister,
    CantUpdate,
    CantUnRegister,
    CantJoin,
    CantBeNull,
    NotFound,
    CantGet,
    InconsistentData,
    NotImplemented,
    ProtocolUnknown,
    ProtocolsShouldBeSame,
    RadarUnknown,
    FirmwareUnknown,
    FirmwareIncompatible,
    FirmwareChanged,
    CantOpen,
    UnknownError,
    TmSessionUnknown,
    RouteUnknown,
    RouteBusy,
    FileBusy,
    NotAFile,

};

MCC_MSG_DECLSPEC const char* to_string(Error);

class MCC_MSG_DECLSPEC ErrorDscr
{
public:
    ErrorDscr();
    ErrorDscr(Error);
    ErrorDscr(Error, std::string&&);
    ErrorDscr(Error, bmcl::StringView&);
    ~ErrorDscr();
    Error value() const;
    const char* valuestr() const;
    const std::string& text() const;
    std::string full() const;
    QString qfull() const;

    bool isCanceled() const;
private:
    Error _value;
    std::string _text;
};

}
