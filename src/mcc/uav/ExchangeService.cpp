#include "mcc/uav/ExchangeService.h"

namespace mccuav {

ReqItem::ReqItem(mccmsg::RequestPtr&& req, OnSuccess&& onsuccess, bmcl::Option<OnError>&& onerror, bmcl::Option<OnState>&& onstate, bool isSilent)
    : _req(std::move(req)), _onsuccess(std::move(onsuccess)), _onerror(std::move(onerror)), _onstate(std::move(onstate)), _isSilent(isSilent), _isCanceling(false)
{
}

void ReqItem::cancel()
{
    _isCanceling = true;
}

const mccmsg::RequestPtr& ReqItem::req() const
{
    return _req;
}

bool ReqItem::isCanceling() const
{
    return _isCanceling;
}

void ReqItem::onsuccess(mccmsg::ResponsePtr&& r) const
{
    if (!_onsuccess)
    {
        //BMCL_DEBUG() << "Не задан обработчик успешного ответа на запрос: " << _req->requestId();
        return;
    }
    _onsuccess(r);
}

void ReqItem::onerror(mccmsg::ErrorDscr&& r) const
{
    if (_onerror.isNone() && !_isSilent)
    {
        //BMCL_DEBUG() << "Не задан обработчик ошибочного ответа на запрос: " << _req->requestId() << r.full();
        return;
    }
    if(_onerror.isSome())
        _onerror.unwrap()(r);
}

void ReqItem::onstate(mccmsg::Request_StatePtr&& r) const
{
    if (_onstate.isNone() && !_isSilent)
    {
        //BMCL_DEBUG() << "Не задан обработчик состояния исполнения запроса: " << _req->requestId();
        return;
    }
    if(_onstate.isSome())
        _onstate.unwrap()(r);
}

ExchangeServicePluginData::ExchangeServicePluginData(ExchangeService* service)
    : mccplugin::PluginData(id)
    , _service(service)
{
}

ExchangeServicePluginData::~ExchangeServicePluginData()
{
}

ExchangeService* ExchangeServicePluginData::service()
{
    return _service.get();
}
}
