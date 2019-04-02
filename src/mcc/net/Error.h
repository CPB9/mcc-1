#pragma once
#include "mcc/Config.h"
#include <string>
#include "mcc/msg/Error.h"

namespace caf { class actor_system_config; }
namespace caf { class error; }
namespace bmcl { class StringView; }

namespace mccmsg
{
MCC_ERROR_DECLSPEC mccmsg::ErrorDscr to_errordscr(const caf::error&);
MCC_ERROR_DECLSPEC bool isMccError(const caf::error& e);
MCC_ERROR_DECLSPEC bool isMccCancel(const caf::error& e);
MCC_ERROR_DECLSPEC caf::error make_error(mccmsg::Error);
MCC_ERROR_DECLSPEC caf::error make_error(mccmsg::Error x, const char* text);
MCC_ERROR_DECLSPEC caf::error make_error(mccmsg::Error x, const bmcl::StringView& text);
MCC_ERROR_DECLSPEC caf::error make_error(mccmsg::Error x, std::string&& text);
MCC_ERROR_DECLSPEC void add_renderer(caf::actor_system_config& cfg);
}
