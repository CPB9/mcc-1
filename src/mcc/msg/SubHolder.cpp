#include "mcc/msg/SubHolder.h"
#include "mcc/msg/TmView.h"

namespace mccmsg {

SubHolder::SubHolder() {}
SubHolder::SubHolder(HandlerId id, ITmExtension* storage) : _id(id), _ext(storage) {}
SubHolder::SubHolder(SubHolder&& other) : _id(std::move(other._id)), _ext(std::move(other._ext)){}
SubHolder& SubHolder::operator=(SubHolder&& other)
{
    _id = std::move(other._id);
    _ext = std::move(other._ext);
    return *this;
}
SubHolder::~SubHolder()
{
    unsub();
}

bmcl::Option<HandlerId> SubHolder::takeId()
{
    _ext.reset();
    return _id;
}

void SubHolder::unsub()
{
    if (_ext.isNull())
        return;
    _ext->removeHandler(_id);
    _ext.reset();
}

}
