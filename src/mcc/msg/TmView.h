#pragma once
#include "mcc/Config.h"
#include "mcc/msg/Msg.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/Cmd.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/OptionRc.h>
#include <map>

namespace mccmsg {

class MCC_MSG_DECLSPEC ITmViewUpdate : public TmAny
{
public:
    ITmViewUpdate(const Device& device);
    ~ITmViewUpdate() override;
    void visit(TmVisitor* visitor) const override;
private:
};

class MCC_MSG_DECLSPEC ITmView : public TmAny
{
public:
    ITmView(const Device& device);
    ~ITmView() override;
    void visit(TmVisitor* visitor) const override;
private:
};

class MCC_MSG_DECLSPEC ITmStorage : public mcc::RefCountable
{
public:
    ITmStorage();
    ~ITmStorage() override;

    virtual bmcl::Option<Group>   group() const;

    virtual void set(const mccmsg::ITmView*) = 0;
    virtual void update(const ITmViewUpdate*) = 0;
    void removeHandler(const bmcl::Option<HandlerId>&);
    void removeAllHandlers();

    bmcl::OptionRc<ITmExtension> getExtension(const TmExtension& name) const;
    template<typename T>
    bmcl::OptionRc<T> getExtension() const
    {
        auto r = getExtension(T::id());
        if (r.isNone())
            return bmcl::None;
        return bmcl::dynamic_pointer_cast<T>(r.unwrap());
    }

protected:
    TmExtensionCounterPtr& counter();
    void addExtension(const ITmExtensionPtr& ext);
    void removeAllExtensions();
    void removeExtension(const TmExtension& name);
    template<typename T>
    void removeExtension(const bmcl::Rc<T>&)
    {
        removeExtension(T::id());
    }
private:
    TmExtensionCounterPtr _counter;
    std::map<TmExtension, ITmExtensionPtr> _exts;
};
using ITmStoragePtr = bmcl::Rc<ITmStorage>;

class MCC_MSG_DECLSPEC ITmPacketResponse : public TmAny
{
public:
    ITmPacketResponse(const Device& device);
    virtual ~ITmPacketResponse();
    void visit(TmVisitor* visitor) const override;
private:
};

class MCC_MSG_DECLSPEC CmdPacketRequest : public DevReq
{
public:
    CmdPacketRequest(const Device& device);
    ~CmdPacketRequest();
    void visit(CmdVisitor* visitor) const override;
private:
};

}
