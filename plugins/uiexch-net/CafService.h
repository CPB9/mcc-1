#pragma once
#include "mcc/Config.h"
#include <unordered_map>
#include <caf/actor.hpp>
#include "mcc/uav/ExchangeService.h"
#include "mcc/plugin/Fwd.h"

namespace caf { class actor; };

class CafService: public mccuav::ExchangeService
{
    Q_OBJECT
public:
    CafService();
    virtual ~CafService();

    void setCore(const caf::actor& core);

    bool event(QEvent* event) override;

    void onLog(bmcl::LogLevel logLevel, const mccmsg::Device& device, const std::string& text) override;
    void onLog(const mccmsg::Device& device, const std::string& text) override;
    void onLog(bmcl::LogLevel logLevel, const std::string& text) override;
    void onLog(const std::string& text) override;

    const mccuav::ReqMap& requests() const override;
    void cancel(const mccmsg::RequestPtr& req) override;
    void cancel(mccmsg::RequestId) override;
protected:
    void addResponseHandler(mccuav::ReqItem&& item) override;

private:
    bmcl::Option<mccuav::ReqItem&> get(mccmsg::RequestId id);
    mccuav::ReqMap _requests;

    caf::actor _core;
    caf::actor _helper;
};

