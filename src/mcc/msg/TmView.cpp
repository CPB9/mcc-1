#include "mcc/msg/TmView.h"
#include <bmcl/Option.h>
#include <bmcl/OptionRc.h>
#include <bmcl/StringView.h>

namespace mccmsg {

ITmView::ITmView(const Device& device) : TmAny(device) {}
ITmView::~ITmView(){}

ITmViewUpdate::ITmViewUpdate(const Device& device) : TmAny(device) {}
ITmViewUpdate::~ITmViewUpdate() {}

ITmStorage::ITmStorage() : _counter(new TmExtensionCounter) {}
ITmStorage::~ITmStorage() { removeAllHandlers(); }
TmExtensionCounterPtr& ITmStorage::counter() { return _counter; }
bmcl::Option<Group> ITmStorage::group() const { return bmcl::None; }
void ITmStorage::removeAllHandlers()
{
    for (auto& i : _exts)
    {
        i.second->removeAllHandlers();
    }
    _exts.clear();
}
void ITmStorage::removeHandler(const bmcl::Option<HandlerId>& id)
{
    if (id.isNone())
        return;
    for (auto& i : _exts)
    {
        i.second->removeHandler(id);
    }
}

bmcl::OptionRc<ITmExtension> ITmStorage::getExtension(const TmExtension& name) const
{
    const auto i = _exts.find(name);
    if (i == _exts.end())
        return bmcl::None;
    return i->second;
}

void ITmStorage::addExtension(const ITmExtensionPtr& ext)
{
    assert(_exts.find(ext->name()) == _exts.end());
    _exts.emplace(ext->name(), ext);
}

void ITmStorage::removeAllExtensions()
{
    removeAllHandlers();
    _exts.clear();
}

void ITmStorage::removeExtension(const TmExtension& name)
{
    auto i = _exts.find(name);
    if (i == _exts.end())
        return;
    i->second->removeAllHandlers();
    _exts.erase(i);
}


ITmPacketResponse::ITmPacketResponse(const Device& device) : TmAny(device) {}
ITmPacketResponse::~ITmPacketResponse() {}

CmdPacketRequest::CmdPacketRequest(const Device& device) : DevReq(device, bmcl::StringView(), bmcl::StringView()){}
CmdPacketRequest::~CmdPacketRequest(){}
void CmdPacketRequest::visit(CmdVisitor* visitor) const { visitor->visit(this); }

}
