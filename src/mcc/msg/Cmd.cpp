#include <bmcl/StringView.h>
#include <bmcl/Utils.h>
#include <bmcl/DoubleEq.h>

#include "mcc/msg/Cmd.h"
#include "mcc/msg/Calibration.h"
#include "mcc/msg/File.h"
#include "mcc/msg/GroupState.h"
#include "mcc/msg/Route.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/FwdExt.h"


namespace mccmsg {

CmdVisitor::CmdVisitor(const DefaultHandler& handler) : _handler(handler) {}
CmdVisitor::~CmdVisitor(){}
void CmdVisitor::visit(const CmdParamList* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteSet* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteSetNoActive* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteSetActive* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteSetActivePoint* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteSetDirection* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteGetList* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteGet* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteCreate* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteRemove* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdRouteClear* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGetFrm* msg) { _handler(msg); }
void CmdVisitor::visit(const CmdFileGetList* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdFileUpload* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdFileDownload* msg) { _handler(msg); };
void CmdVisitor::visit(const CmdCalibrationStart *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdCalibrationCancel *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdParamRead *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdParamWrite *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGroupNew *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGroupDelete *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGroupAttach *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGroupDetach *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGroupSwitch *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdPacketRequest *msg) { _handler(msg); };
void CmdVisitor::visit(const CmdGetTmView *msg) { _handler(msg); };

CmdRespVisitor::CmdRespVisitor() {}
CmdRespVisitor::~CmdRespVisitor() {}
void CmdRespVisitor::visit(const CmdFileGetListResp* msg) {}
void CmdRespVisitor::visit(const CmdRespEmpty* msg) {}

CmdRespAny::CmdRespAny(const DevReq* cmd) : Response(cmd) {}
CmdRespAny::CmdRespAny(const DevReqPtr& cmd) : Response(cmd.get()) {}
CmdRespAny::~CmdRespAny(){}

void CmdFileGetListResp::visit(CmdRespVisitor* visitor) const { visitor->visit(this); }

CmdRespEmpty::CmdRespEmpty(const DevReq* cmd) : CmdRespAny(cmd) {}
CmdRespEmpty::CmdRespEmpty(const DevReqPtr& cmd) : CmdRespAny(cmd) {}
CmdRespEmpty::~CmdRespEmpty() {}
void CmdRespEmpty::visit(CmdRespVisitor* visitor) const { visitor->visit(this); }

std::string CmdParamList::paramsAsString(bmcl::StringView delimeter) const
{
    if (_params.empty())
        return std::string();

    std::string r = _params[0].stringify();
    for (std::size_t i = 1; i < _params.size(); ++i)
    {
        r += delimeter.toStdString() + _params[i].stringify();
    }
    return r;
}

CmdFileGetListResp::CmdFileGetListResp(const CmdFileGetList* cmd, Files&& files, bmcl::Option<std::string>&& ui) : CmdRespAny(cmd), _files(std::move(files)), _ui(std::move(ui)) {}
CmdFileGetListResp::~CmdFileGetListResp() {}
const Files& CmdFileGetListResp::files() const { return _files; }
const bmcl::Option<std::string>& CmdFileGetListResp::ui() const { return _ui; }


CmdParamList::CmdParamList(const Device& device, bmcl::StringView trait, bmcl::StringView command, const CmdParams& params)
    : DevReq(device, trait, command), _params(params)
{
}
CmdParamList::~CmdParamList() {}
void CmdParamList::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdParamList::nameXXX() const { return "ParamList";}
const char* CmdParamList::info() const { return ""; }
const CmdParams& CmdParamList::params()  const { return _params; }

CmdRouteSet::CmdRouteSet(const Device& device, const Route& route) : DevReq(device, "Navigation.Routes", "set"), _route(route){}
CmdRouteSet::CmdRouteSet(const Device& device, Route&& route) : DevReq(device, "Navigation.Routes", "set"), _route(std::move(route)) {}
CmdRouteSet::~CmdRouteSet() {}
void CmdRouteSet::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteSet::nameXXX() const { return "RouteSet"; }
const char* CmdRouteSet::info() const { return "Загрузка маршрута"; }
const Route& CmdRouteSet::route() const { return _route; }

CmdRouteSetNoActive::CmdRouteSetNoActive(const Device& device) : DevReq(device, "Navigation.Routes", "setNoActiveRoute") {}
CmdRouteSetNoActive::~CmdRouteSetNoActive() {}
void CmdRouteSetNoActive::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteSetNoActive::nameXXX() const { return "RouteSetNoActive"; }
const char* CmdRouteSetNoActive::info() const { return "Нет активного маршрута"; }

CmdRouteSetActive::CmdRouteSetActive(const Device& device, uint32_t route, const bmcl::Option<uint32_t>& point) : DevReq(device, "Navigation.Routes", "setActiveRoute"), _route(route), _point(point) { }
CmdRouteSetActive::~CmdRouteSetActive() {}
void CmdRouteSetActive::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteSetActive::nameXXX() const { return "RouteSetActive"; }
const char* CmdRouteSetActive::info() const { return "Задание акт. маршрута"; }
uint32_t CmdRouteSetActive::route() const { return _route; }
const bmcl::Option<uint32_t>& CmdRouteSetActive::point() const { return _point; }

CmdRouteSetActivePoint::CmdRouteSetActivePoint(const Device& device, uint32_t route, const bmcl::Option<uint32_t>& point)
    : DevReq(device, "Navigation.Routes", "setActivePoint"), _route(route), _point(point)
{}
CmdRouteSetActivePoint::~CmdRouteSetActivePoint() {}
void CmdRouteSetActivePoint::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteSetActivePoint::nameXXX() const { return "RouteSetActivePoint"; }
const char* CmdRouteSetActivePoint::info() const { return "Задание акт. точки"; }
uint32_t CmdRouteSetActivePoint::route() const { return _route; }
const bmcl::Option<uint32_t>& CmdRouteSetActivePoint::point() const { return _point; }

CmdRouteSetDirection::CmdRouteSetDirection(const Device& device, uint32_t route, bool isForward)
    : DevReq(device, "Navigation.Routes", "setDirection"), _route(route), _isForward(isForward) {}
void CmdRouteSetDirection::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteSetDirection::nameXXX() const { return "RouteSetDirection"; }
const char* CmdRouteSetDirection::info() const { return "Задание напр. дв. по маршруту"; }
CmdRouteSetDirection::~CmdRouteSetDirection() {}
uint32_t CmdRouteSetDirection::route() const { return _route; }
bool CmdRouteSetDirection::isForward() const { return _isForward; }

CmdRouteGetList::CmdRouteGetList(const Device& device) : DevReq(device, "Navigation.Routes", "getList") {}
CmdRouteGetList::~CmdRouteGetList() {}
void CmdRouteGetList::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteGetList::nameXXX() const { return "RouteGetList"; }
const char* CmdRouteGetList::info() const { return "Запрос списка маршрутов"; }

CmdRouteGet::CmdRouteGet(const Device& device, uint32_t route) : DevReq(device, "Navigation.Routes", "get"), _route(route) {}
CmdRouteGet::~CmdRouteGet() {}
void CmdRouteGet::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteGet::nameXXX() const { return "RouteGet"; }
const char* CmdRouteGet::info() const { return "Запрос маршрута"; }
uint32_t CmdRouteGet::route() const { return _route; }

CmdRouteCreate::CmdRouteCreate(const Device& device, uint32_t route) : DevReq(device, "Navigation.Routes", "create"), _route(route) {}
CmdRouteCreate::~CmdRouteCreate() {}
void CmdRouteCreate::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteCreate::nameXXX() const { return "RouteCreate"; }
const char* CmdRouteCreate::info() const { return "Создание маршрута"; }
uint32_t CmdRouteCreate::route() const { return _route; }

CmdRouteRemove::CmdRouteRemove(const Device& device, uint32_t route) : DevReq(device, "Navigation.Routes", "remove"), _route(route) {}
CmdRouteRemove::~CmdRouteRemove() {}
void CmdRouteRemove::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteRemove::nameXXX() const { return "RouteRemove"; }
const char* CmdRouteRemove::info() const { return "Удаление маршрута"; }
uint32_t CmdRouteRemove::route() const { return _route; }

CmdRouteClear::CmdRouteClear(const Device& device, uint32_t route) : DevReq(device, "Navigation.Routes", "clear"), _route(route) {}
CmdRouteClear::~CmdRouteClear() {}
void CmdRouteClear::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdRouteClear::nameXXX() const { return "RouteClear"; }
const char* CmdRouteClear::info() const { return "Очистка маршрута"; }
uint32_t CmdRouteClear::route() const { return _route; }

CmdFileGetList::CmdFileGetList(const Device& device) : DevReq(device, "File", "getList") {}
CmdFileGetList::~CmdFileGetList() {}
void CmdFileGetList::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdFileGetList::nameXXX() const { return "FileGetList"; }
const char* CmdFileGetList::info() const { return "Запрос списка файлов"; }

CmdFileUpload::CmdFileUpload(const Device& device, bmcl::StringView local, bmcl::StringView remote)
    : DevReq(device, "File", "upload"), _local(local.toStdString()), _remote(remote.toStdString())
{
}
CmdFileUpload::~CmdFileUpload() {}
void CmdFileUpload::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdFileUpload::nameXXX() const { return "FileUpload"; }
const char* CmdFileUpload::info() const { return "Загрузка файла"; }
const std::string& CmdFileUpload::local() const { return _local; }
const std::string& CmdFileUpload::remote() const { return _remote; }

CmdFileDownload::CmdFileDownload(const Device& device, bmcl::StringView local, bmcl::StringView remote)
    : DevReq(device, "File", "download"), _local(local.toStdString()), _remote(remote.toStdString())
{
}
CmdFileDownload::~CmdFileDownload() {}
void CmdFileDownload::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdFileDownload::nameXXX() const { return "FileDownload"; }
const char* CmdFileDownload::info() const { return "Скачивание файла"; }
const std::string& CmdFileDownload::local() const { return _local; }
const std::string& CmdFileDownload::remote() const { return _remote; }

CmdGetFrm::CmdGetFrm(const Device& device) : DevReq(device, "Frm", "upload"){}
CmdGetFrm::~CmdGetFrm(){}
void CmdGetFrm::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGetFrm::nameXXX() const { return "GetFrm"; }
const char* CmdGetFrm::info() const { return "Регистрация"; }

CmdCalibrationStart::CmdCalibrationStart(const Device& device, CalibrationSensor component,
                                   const bmcl::Option<CalibrationCmd>& specialCmd,
                                   const bmcl::Option<CalibrationFlightModes>& flightModes)
    : DevReq(device, "Device", "calibrationStart")
    , _component(component)
    , _specialCmd(specialCmd)
    , _flightModes(flightModes)
{
}
CmdCalibrationStart::~CmdCalibrationStart() {}
CalibrationSensor CmdCalibrationStart::component() const { return _component; }
const bmcl::Option<CalibrationCmd>& CmdCalibrationStart::specialCmd() const { return _specialCmd; };
const bmcl::Option<CalibrationFlightModes>& CmdCalibrationStart::flightModes() const { return _flightModes; };
void CmdCalibrationStart::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdCalibrationStart::nameXXX() const { return "CalibrationStart"; }
const char* CmdCalibrationStart::info() const { return "Начало калибровки"; }

CmdCalibrationCancel::CmdCalibrationCancel(const Device& device)
    : DevReq(device, "Device", "calibrationCancel")
{
}
CmdCalibrationCancel::~CmdCalibrationCancel() {}
void CmdCalibrationCancel::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdCalibrationCancel::nameXXX() const { return "CalibrationCancel"; }
const char* CmdCalibrationCancel::info() const { return "Отмена калибровки"; }

CmdParamRead::CmdParamRead(const Device& device, bmcl::StringView trait, const std::vector<std::string>& vars)
    : DevReq(device, "Params", "paramRead")
    , _trait(trait.toStdString())
    , _vars(vars)
{
}
CmdParamRead::~CmdParamRead() {}
void CmdParamRead::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdParamRead::nameXXX() const { return "ParamRead"; }
const char* CmdParamRead::info() const { return "Чтение параметра"; }
const std::string& CmdParamRead::trait() const { return _trait; }
const std::vector<std::string>& CmdParamRead::vars() const { return _vars; }

CmdParamWrite::CmdParamWrite(const Device& device, bmcl::StringView trait, const std::vector<std::pair<std::string, NetVariant>>& vars)
    : DevReq(device, "Params", "paramWrite")
    , _trait(trait.toStdString())
    , _vars(vars)
{
}

CmdParamWrite::~CmdParamWrite() {}
void CmdParamWrite::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdParamWrite::nameXXX() const { return "ParamWrite"; }
const char* CmdParamWrite::info() const { return "Запись параметра"; }
const std::string& CmdParamWrite::trait() const { return _trait; }
const std::vector<std::pair<std::string, NetVariant>>& CmdParamWrite::vars() const { return _vars; }

CmdGroupNew::CmdGroupNew(const Group& group, const Devices& members) : DevReq(group, "Group", "new"), _members(members)
{
}
CmdGroupNew::CmdGroupNew(const Group& group, Devices&& members) : DevReq(group, "Group", "new"), _members(std::move(members))
{
}
CmdGroupNew::~CmdGroupNew() {}
const Devices& CmdGroupNew::members() const { return _members; }
void CmdGroupNew::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGroupNew::nameXXX() const { return "GroupNew"; }
const char* CmdGroupNew::info() const { return "Создание группы"; }

CmdGroupDelete::CmdGroupDelete(const Group&  group) : DevReq(group, "Group", "delete")
{
}
CmdGroupDelete::~CmdGroupDelete() {}
void CmdGroupDelete::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGroupDelete::nameXXX() const { return "GroupDelete"; }
const char* CmdGroupDelete::info() const { return "Удаление группы"; }

CmdGroupAttach::CmdGroupAttach(const Group& group, const Device& attachable) : DevReq(group, "Group", "attach"), _attachable(attachable)
{
}
CmdGroupAttach::~CmdGroupAttach() {}
void CmdGroupAttach::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGroupAttach::nameXXX() const { return "GroupAttach"; }
const char* CmdGroupAttach::info() const { return "Присоединение к группе"; }
const Device& CmdGroupAttach::attachable() const { return _attachable; }

CmdGroupDetach::CmdGroupDetach(const bmcl::Option<Group>& group, const Device& device) : DevReq(group.unwrapOr(Group::createNil()), "Group", "detach"), _detachable(device)
{
}
CmdGroupDetach::~CmdGroupDetach() {}
void CmdGroupDetach::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGroupDetach::nameXXX() const { return "GroupDetach"; }
const char* CmdGroupDetach::info() const { return "Исключение из группы"; }
const Device& CmdGroupDetach::detachable() const { return _detachable; }

CmdGroupSwitch::CmdGroupSwitch(const Device& device, const Group&  group, const Group&  to) : DevReq(group, "Group", "switch"), _to(to)
{
}
CmdGroupSwitch::~CmdGroupSwitch() {}
const Group&  CmdGroupSwitch::to() const { return _to; }
void CmdGroupSwitch::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGroupSwitch::nameXXX() const { return "GroupSwitch"; }
const char* CmdGroupSwitch::info() const { return "Смена группы"; }

CmdGetTmView::CmdGetTmView(const Device& device) : DevReq(device, "Group", "switch")
{
}
CmdGetTmView::~CmdGetTmView() {}
void CmdGetTmView::visit(CmdVisitor* visitor) const { visitor->visit(this); }
const char* CmdGetTmView::nameXXX() const { return "CmdGetTmView"; }
const char* CmdGetTmView::info() const { return "Запрос текущего среза тм"; }

}
