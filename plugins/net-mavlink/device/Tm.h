#pragma once
#include "mcc/Config.h"
#include <bmcl/Option.h>
#include <bmcl/OptionPtr.h>
#include "mcc/msg/TmView.h"
#include "mcc/msg/exts/ErrStorage.h"
#include "mcc/msg/exts/StateStorage.h"
#include "mcc/msg/exts/Position.h"
#include "mcc/msg/exts/NamedAccess.h"
#include "Firmware.h"

#include "device/Mavlink.h"

namespace mccmav {

class ITmUpdateVisitor;

class TmUpdateMavlink : public mccmsg::ITmViewUpdate
{
public:
    TmUpdateMavlink(const mccmsg::Device& device);
    ~TmUpdateMavlink() override;
    using mccmsg::ITmViewUpdate::visit;
    virtual void visit(ITmUpdateVisitor&) const = 0;
};

class TmUpdateParam: public TmUpdateMavlink
{
public:
    TmUpdateParam(const mccmsg::Device& device, const ParamValue& param);
    ~TmUpdateParam();
    using TmUpdateMavlink::visit;
    void visit(ITmUpdateVisitor&) const override;
    const ParamValue& value() const;
private:
    ParamValue _value;
};

class TmUpdateMode : public TmUpdateMavlink
{
public:
    TmUpdateMode(const mccmsg::Device& device, uint8_t baseMode, uint32_t customMode, uint8_t systemState);
    ~TmUpdateMode();
    using TmUpdateMavlink::visit;
    void visit(ITmUpdateVisitor&) const override;
    uint8_t baseMode() const;
    uint32_t customMode() const;
    uint8_t systemState() const;
private:
    uint8_t _baseMode;
    uint32_t _customMode;
    uint8_t _systemState;
};

class TmUpdateMsg : public TmUpdateMavlink
{
public:
    TmUpdateMsg(const mccmsg::Device& device, const MavlinkMessagePtr& msg);
    ~TmUpdateMsg();
    using TmUpdateMavlink::visit;
    void visit(ITmUpdateVisitor&) const override;
    const MavlinkMessagePtr& msg() const;
private:
    MavlinkMessagePtr _msg;
};

class ITmUpdateVisitor
{
public:
    ITmUpdateVisitor();
    virtual ~ITmUpdateVisitor();
    virtual void visit(const TmUpdateParam*) = 0;
    virtual void visit(const TmUpdateMode*) = 0;
    virtual void visit(const TmUpdateMsg*) = 0;
};

class TmView : public mccmsg::ITmView
{
public:
    TmView(const mccmsg::Device& device, const bmcl::OptionRc<const Firmware>& f);
    ~TmView() override;
    const bmcl::OptionRc<const Firmware>& firmware() const;
private:
    bmcl::OptionRc<const Firmware> _firmware;
};

class TmViewUpdate : public mccmsg::ITmViewUpdate
{
public:
    explicit TmViewUpdate(const mccmsg::Device& device);
    ~TmViewUpdate() override;
private:
};

class StateStorage : public mccmsg::IStateStorage
{
    friend class TmStorage;
public:
    StateStorage(const mccmsg::TmExtensionCounterPtr&);
    ~StateStorage() override;
    bmcl::Option<int> state() const override;
    bmcl::Option<int> subState() const override;
    const QString& stateStr() const override;
    const QString& subStateStr() const override;
    void set(const bmcl::OptionRc<const Firmware>& firmware);
    void set(const bmcl::SystemTime& t, uint8_t baseMode, uint32_t customMode, uint8_t systemState);
private:
    bmcl::Option<int> _state;
    bmcl::Option<int> _subState;
    QString _stateStr;
    QString _subStateStr;

    bmcl::OptionRc<const Firmware> _firmware;

    uint8_t _baseMode;
    uint32_t _customMode;
    uint8_t _systemStatus;
};

using NativeHandler = std::function<void(const ParamValue&)>;
class NamedAccess : public mccmsg::INamedAccess
{
    friend class TmStorage;
public:
    NamedAccess(const mccmsg::TmExtensionCounterPtr&);
    ~NamedAccess() override;
    void removeHandler(const bmcl::Option<mccmsg::HandlerId>&) override;
    void removeAllHandlers() override;

    bmcl::Option<mccmsg::SubHolder> addHandler(bmcl::StringView name, mccmsg::ValueHandler&& handler, bool onChangeOnly) override;
    bmcl::Option<mccmsg::SubHolder> addHandler(bmcl::StringView name, NativeHandler&& handler);
    mccmsg::SubHolder addHandler(NativeHandler&& handler);
private:
    void set(const ParamValue&);
    bmcl::OptionRc<const Firmware> _firmware;
    struct Item
    {
        Item(mccmsg::HandlerId, NativeHandler&&);
        ~Item();
        mccmsg::HandlerId id;
        NativeHandler h;
    };
    using NativeHandlers = std::vector<Item>;
    NativeHandlers _handlers;
};

class TmStorage : public mccmsg::ITmStorage, public ITmUpdateVisitor
{
public:
    explicit TmStorage(const bmcl::OptionRc<const TmView>&);
    ~TmStorage() override;

    const bmcl::OptionRc<const Firmware>& firmware() const;
    void set(const mccmsg::ITmView*) override;
    void update(const mccmsg::ITmViewUpdate*) override;
    bmcl::Option<ParamValue> valueByName(const std::string& name) const;
    NamedAccess& namedAccess();

private:
    void visit(const TmUpdateParam*) override;
    void visit(const TmUpdateMode*) override;
    void visit(const TmUpdateMsg*) override;

    bmcl::OptionRc<const Firmware> _firmware;

    using Values = std::vector<ParamValue>;
    Values _values;
    bmcl::Rc<StateStorage> _state;
    bmcl::Rc<NamedAccess> _names;
};

}
