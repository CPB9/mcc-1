#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <unordered_map>
#include <bmcl/Logging.h>
#include <bmcl/Rc.h>

#include "mcc/plugin/PluginData.h"

#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/obj/Firmware.h"
#include "mcc/msg/obj/Protocol.h"
#include "mcc/msg/obj/Device.h"
#include "mcc/msg/obj/DeviceUi.h"

#include "mcc/msg/File.h"
#include "mcc/msg/Nav.h"
#include "mcc/msg/Route.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/Error.h"
#include "mcc/msg/FwdExt.h"

#include "mcc/msg/ptr/Message.h"
#include "mcc/msg/ptr/Tm.h"

Q_DECLARE_METATYPE(QVector<QString>);
Q_DECLARE_METATYPE(mccmsg::ProtocolId);
Q_DECLARE_METATYPE(mccmsg::ProtocolDescription);
Q_DECLARE_METATYPE(mccmsg::ProtocolDescriptions);
Q_DECLARE_METATYPE(mccmsg::ChannelDescription);
Q_DECLARE_METATYPE(mccmsg::ChannelDescriptions);
Q_DECLARE_METATYPE(mccmsg::DeviceDescription);
Q_DECLARE_METATYPE(mccmsg::DeviceDescriptions);
Q_DECLARE_METATYPE(mccmsg::FirmwareDescription);
Q_DECLARE_METATYPE(mccmsg::FirmwareDescriptions);
Q_DECLARE_METATYPE(mccmsg::DeviceUi);
Q_DECLARE_METATYPE(mccmsg::DeviceUis);
Q_DECLARE_METATYPE(mccmsg::DeviceUiDescription);
Q_DECLARE_METATYPE(mccmsg::DeviceUiDescriptions);
Q_DECLARE_METATYPE(mccmsg::TmMotionPtr);
Q_DECLARE_METATYPE(mccmsg::TmRoutePtr);
Q_DECLARE_METATYPE(mccmsg::TmRoutesListPtr);
Q_DECLARE_METATYPE(mccmsg::ErrorDscr);
Q_DECLARE_METATYPE(bmcl::Rc<mccmsg::ITmViewUpdate>);
Q_DECLARE_METATYPE(bmcl::Rc<mccmsg::ITmView>);

namespace mccuav {

using OnError = std::function<void(const mccmsg::ErrorDscr&)>;
using OnSuccess = std::function<void(const mccmsg::ResponsePtr&)>;
using OnState = std::function<void(const mccmsg::Request_StatePtr&)>;

class MCC_UAV_DECLSPEC ReqItem
{
public:
    ReqItem(mccmsg::RequestPtr&& req, OnSuccess&& onsuccess, bmcl::Option<OnError>&& onerror = bmcl::None, bmcl::Option<OnState>&& onstate = bmcl::None, bool isSilent = false);
    void cancel();
    const mccmsg::RequestPtr& req() const;
    void onsuccess(mccmsg::ResponsePtr&& r) const;
    void onerror(mccmsg::ErrorDscr&& r) const;
    void onstate(mccmsg::Request_StatePtr&& r) const;
    bool isCanceling() const;

private:
    bool _isCanceling;
    bool _isSilent;
    mccmsg::RequestPtr _req;
    OnSuccess _onsuccess;
    bmcl::Option<OnError> _onerror;
    bmcl::Option<OnState> _onstate;
};
using ReqMap = std::unordered_map<mccmsg::RequestId, ReqItem>;


class MCC_UAV_DECLSPEC ExchangeService: public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
    friend class NoteVisitorX;
    friend class ReqVisitorS;

public:

    template<typename R>
    class ResponseHandle
    {
    public:
        ~ResponseHandle()
        {
            if (!_sent)
                BMCL_DEBUG() << "ответ не обрабатывается";
        }

        ResponseHandle(ExchangeService* self, const R* r)
            : _sent(false), _self(self), _r(r) {}
        template <class F>
        bmcl::Rc<const R> then(F&& f, OnError&& e = nullptr, OnState&& s = nullptr)
        {
            then_impl(std::move(f), std::move(e), std::move(s));
            return _r;
        }
    private:
        template <class F>
        void then_impl(F&& f, OnError&& ef, OnState&& st)
        {
            _sent = true;
            auto s = [f](const mccmsg::ResponsePtr& rep)
            {
                const auto p = bmcl::static_pointer_cast<const typename R::Response>(rep);
                if (!p.isNull())
                    f(p);
            };

            if (ef == nullptr)
            {
                _self->addResponseHandler(ReqItem{ _r, std::move(s) });
                return;
            }

            if (st == nullptr)
            {
                _self->addResponseHandler(ReqItem{ _r, std::move(s), std::move(ef) });
                return;
            }

            _self->addResponseHandler(ReqItem{ _r, std::move(s), std::move(ef), std::move(st) });
        }

        bool _sent;
        ExchangeService* _self;
        bmcl::Rc<const R> _r;
    };

    template<typename R>
    ResponseHandle<R> requestXX(R* p)
    {
        return ResponseHandle<R>(this, p);
    }

    virtual void cancel(const mccmsg::RequestPtr& req) = 0;
    virtual void cancel(mccmsg::RequestId) = 0;

    virtual const ReqMap& requests() const = 0;
public slots:
    virtual void onLog(bmcl::LogLevel logLevel, const mccmsg::Device& device, const std::string& text) = 0;
    virtual void onLog(const mccmsg::Device& device, const std::string& text) = 0;
    virtual void onLog(bmcl::LogLevel logLevel, const std::string& text) = 0;
    virtual void onLog(const std::string& text) = 0;
signals:
    void log(const mccmsg::tm::LogPtr& logMessage);
    void setTmView(const bmcl::Rc<const mccmsg::ITmView>& view);
    void updateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>&);
    void tmPaketResponse(const bmcl::Rc<const mccmsg::ITmPacketResponse>& pkt);

    void channelRegistered(const mccmsg::Channel& channel);
    void channelUnRegistered(const mccmsg::Channel& channel);
    void channelUpdated(const mccmsg::ChannelDescription&);
    void channelActivated(const mccmsg::Channel& channel, bool isActive);
    void channelState(const mccmsg::StatChannel& state);

    void deviceRegistered(const mccmsg::Device& device);
    void deviceUnRegistered(const mccmsg::Device& device);
    void deviceUpdated(const mccmsg::DeviceDescription&);
    void deviceActivated(const mccmsg::Device& device, bool isActive);
    void deviceState(const mccmsg::StatDevice& state);
    void deviceConnected(const mccmsg::Channel& channel, const mccmsg::Device& device);
    void deviceDisconnected(const mccmsg::Channel& channel, const mccmsg::Device& device, const QString& error = QString());
    void tmSessionUpdated(const mccmsg::TmSessionDescription&);
    void tmSessionRegistered(const mccmsg::TmSession& session, bool isRegistered);

    void protocolRegistered(const mccmsg::Protocol& protocol);

    void traitNavigationMotion(const mccmsg::TmMotion*);
    void traitRouteState(const mccmsg::TmRoute*);
    void traitRoutesList(const mccmsg::TmRoutesList*);
    void traitGroupState(const mccmsg::TmGroupState*);
    void traitCalibration(const mccmsg::TmCalibration*);
    void traitCommonCalibrationStatus(const mccmsg::TmCommonCalibrationStatus*);

    void requestAdded(const mccmsg::DevReqPtr& req);
    void requestStateChanged(const mccmsg::DevReqPtr& req, const mccmsg::Request_StatePtr&);
    void requestRemoved(const mccmsg::DevReqPtr& req);

protected:
    virtual void addResponseHandler(ReqItem&& item) = 0;
};

class MCC_UAV_DECLSPEC ExchangeServicePluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mccuav::ExchangeServicePluginData";

    ExchangeServicePluginData(ExchangeService* service);
    ~ExchangeServicePluginData();

    ExchangeService* service();

private:
    Rc<ExchangeService> _service;
};
}
