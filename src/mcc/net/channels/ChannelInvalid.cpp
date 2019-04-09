#include <caf/error.hpp>
#include <asio/io_context.hpp>
#include "mcc/msg/obj/Channel.h"
#include "mcc/net/Error.h"
#include "mcc/net/channels/ChannelInvalid.h"


namespace mccnet {

ChannelInvalid::ChannelInvalid(asio::io_context& io_context, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings)
    : ChannelImpl(io_context, id, std::move(exch), settings)
{
}

void ChannelInvalid::update_(const mccmsg::ChannelDescription&)
{
}

bool ChannelInvalid::is_open_() const
{
    return false;
}

void ChannelInvalid::connect_(OpCompletion&& f)
{
    f(mccmsg::make_error(mccmsg::Error::ChannelUnknown));
}

void ChannelInvalid::disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&& e)
{
    ChannelImpl::disconnected_(std::move(f), std::move(e));
}

void ChannelInvalid::data_send_(mccmsg::PacketPtr&&, OpCompletion&& f)
{
    f(mccmsg::make_error(mccmsg::Error::ChannelUnknown));
}

void ChannelInvalid::data_read_()
{
}

}
