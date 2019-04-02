#include <caf/actor_system_config.hpp>
#include <caf/error.hpp>
#include <caf/atom.hpp>
#include <caf/make_message.hpp>
#include <fmt/format.h>
#include <bmcl/StringView.h>
#include "mcc/net/Error.h"
#include "mcc/msg/Error.h"

using MccAtom = caf::atom_constant<caf::atom("mcc")>;

namespace mccmsg
{

bool isMccError(const caf::error& e)
{
    return e.category() == MccAtom::value;
}

bool isMccCancel(const caf::error& e)
{
    return isMccError(e) && ((mccmsg::Error)e.code() == Error::Canceled);
}

caf::error make_error(mccmsg::Error x)
{
  return {static_cast<uint8_t>(x), MccAtom::value };
}

caf::error make_error(mccmsg::Error x, std::string&& text)
{
    return { static_cast<uint8_t>(x), MccAtom::value, caf::make_message(std::move(text)) };
}

caf::error make_error(mccmsg::Error x, const bmcl::StringView& text)
{
    return make_error(x, text.toStdString());
}

caf::error make_error(mccmsg::Error x, const char* text)
{
    return make_error(x, std::string(text));
}

void add_renderer(caf::actor_system_config& cfg)
{
    auto renderer = [](uint8_t x, caf::atom_value, const caf::message& m) -> std::string
    {
        return fmt::format("mcc error({}): {}, {}", x, mccmsg::to_string((mccmsg::Error)x), to_string(m));
        //return "mcc error (" + caf::deep_to_string_as_tuple(static_cast<Error>(x));
    };
    cfg.add_error_category(MccAtom::value, renderer);
}

mccmsg::ErrorDscr to_errordscr(const caf::error& e)
{
    if (isMccError(e))
    {
        if (e.context().empty())
            return mccmsg::ErrorDscr((mccmsg::Error)e.code());
        return mccmsg::ErrorDscr((mccmsg::Error)e.code(), caf::to_string(e.context()));
    }
    return mccmsg::ErrorDscr(mccmsg::Error::UnknownError, caf::to_string(e));
}

}
