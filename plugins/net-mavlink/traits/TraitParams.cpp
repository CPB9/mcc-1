#include "../traits/TraitParams.h"
#include "../traits/Trait.h"
#include "device/Tm.h"

#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>
#include <fmt/format.h>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mavlink_param_value_t);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<mccmav::ParamValue>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NetVariant);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdParamReadPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdParamWritePtr);

namespace mccmav {

TraitParams::TraitParams(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings)
    : caf::event_based_actor(cfg), _helper(id, core, this), _id(id), _activated(false), _name(name), _core{ core }, _broker{ broker }, _settings(settings)
{

}

caf::behavior TraitParams::make_behavior()
{
    return
    {
        [this](activated_atom)
        {
            if (!_activated)
            {
                _activated = true;
            }
        }
      , [this](deactivated_atom)
        {
            if (_activated)
            {
                _activated = false;
                _queuedCmds.clear();
                _params.clear();
            }
        }
      , [this](params_dump_atom, const std::vector<ParamValue>& params)
        {
            _params = params;
            for(const auto& it : _params)
                _helper.sendTrait(new TmUpdateParam(_id.device(), it));

//             for (auto it : _queuedCmds)
//             {
//                 //mccnet::CmdPtr cmd = std::move(it);
//                 //mccmsg::DevReqPtr p = cmd->item();
//                 //execute(std::move(cmd), *bmcl::static_pointer_cast<const mccmsg::CmdParamRead>(p));
//             }
// 
//            _queuedCmds.clear();
        }
//       , [this](const mccmsg::DevReqPtr& msg)
//         {
//              TraitParamsCmdVisitor visitor{ this, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
//              msg->visit(&visitor);
//         }
      , [this](const mavlink_param_value_t& msg)
        {
            if (!_activated)
                return;

            processMavlinkMessage(msg);
        }
      ,[this](frmupdate_atom, const FirmwarePtr& frm)
        {
            _firmware = frm;
            _helper.sendTrait(new TmView(_helper.device(), _firmware));
        }
      , [this](const mccmsg::CmdParamWritePtr& cmd)
        {
            execute(bmcl::makeRc<mccnet::Cmd>(make_response_promise(), cmd), cmd);
        }
      , [this](const mccmsg::CmdParamReadPtr& cmd)
        {
            execute(bmcl::makeRc<mccnet::Cmd>(make_response_promise(), cmd), cmd);
        }
     };
}

const char* TraitParams::name() const
{
    return _name.c_str();
}

void TraitParams::writeParam(const std::string& id, const mccmsg::NetVariant& value)
{
    (void)id;
    (void)value;
}

void TraitParams::readParam(const std::string& id, const mccmsg::NetVariant& value)
{
    (void)id;
    (void)value;
}

void TraitParams::processMavlinkMessage(const mavlink_param_value_t& msg)
{
    auto len = strlen(msg.param_id);
    if (len > MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN)
        len = MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN;

    std::string id(msg.param_id, len);

    if (id == "_HASH_CHECK")
        return;

    mavlink_param_union_t converter;
    converter.param_float = msg.param_value;
    converter.type = msg.param_type;
    mccmsg::NetVariant value = toNetVariant(converter);
    send(_broker, set_param_atom::value, id, value);

    if (_params.empty())
        return;

    auto param = std::find_if(_params.begin(), _params.end(), [id](const ParamValue& p) { return p.name == id; });
    if (param == _params.end())
    {
        //assert(false);
        return;
    }


    param->value = value;
    _helper.sendTrait(new TmUpdateParam(_id.device(), *param));
    //_helper.sendTm( { mccmsg::TmParam("", id, param->value) });
}

void TraitParams::execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdParamReadPtr& msg)
{
//     if (_params.empty())
//     {
//         _queuedCmds.emplace_back(cmd);
//         return;
//     }
// 
//     mccmsg::TmParams reply;
// 
//     for (const auto& var : msg->vars())
//     {
//         const auto param = std::find_if(_params.begin(), _params.end(), [var](const ParamValue& p) { return p.name == var; });
//         if (param == _params.end())
//             continue;
//         reply.emplace_back(mccmsg::TmParam("", var, param->value));
//      }
// 
//     _helper.sendTm(std::move(reply));
    cmd->sendDone();
}

void TraitParams::execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdParamWritePtr& msg)
{
    for (const auto& varReq : msg->vars())
    {
        const auto var = std::find_if(_params.begin(), _params.end(), [&varReq](const ParamValue& p) { return p.name == varReq.first; });
        if (var == _params.end())
        {
            cmd->sendFailed(fmt::format("Неизвестная переменная: {}", varReq.first));
            return;
        }

        auto type = var->type;
        auto value = varReq.second;
        mavlink_param_union_t paramUnion;
        paramUnion.type = type;
        switch (type)
        {
        case MAV_PARAM_TYPE_UINT8:
            paramUnion.param_uint8 = value.toUint();
            break;
        case MAV_PARAM_TYPE_INT8:
            paramUnion.param_int8 = value.toInt();
            break;
        case MAV_PARAM_TYPE_UINT16:
            paramUnion.param_uint16 = value.toUint();
            break;
        case MAV_PARAM_TYPE_INT16:
            paramUnion.param_int16 = value.toInt();
            break;
        case MAV_PARAM_TYPE_UINT32:
            paramUnion.param_uint32 = value.toUint();
            break;
        case MAV_PARAM_TYPE_INT32:
            paramUnion.param_int32 = value.toInt();
            break;
        case MAV_PARAM_TYPE_REAL32:
            paramUnion.param_float = value.toDouble();
            break;
        default:
            //cmd->sendFailed("Неизвестный тип параметра: {} {}", varReq.first, varReq.second);
            return;
        }

        mavlink_param_set_t     p;
        p.param_value = paramUnion.param_float;
        p.param_type = paramUnion.type;
        p.target_system = _settings.system;
        p.target_component = (uint8_t)var->componentId;

        strncpy(p.param_id, var->name.c_str(), sizeof(p.param_id));

        MavlinkMessageRc* m = new MavlinkMessageRc;
        mavlink_msg_param_set_encode_chan(_settings.system,
                                          _settings.component,
                                          _settings.channel,
                                          m,
                                          &p);

        request(_broker, std::chrono::seconds(3), write_param_atom::value, MavlinkMessagePtr(m), var->index).then(
            [cmd, m, paramUnion](const MavlinkMessagePtr& message)
        {
            mavlink_param_value_t param_value;
            mavlink_msg_param_value_decode(message.get(), &param_value);

            mavlink_param_union_t paramUnion1;
            paramUnion1.param_float = param_value.param_value;
            if (paramUnion.param_uint32 == paramUnion1.param_uint32)
                cmd->sendDone();
            else
                cmd->sendFailed("Ошибка при записи параметра: неверное значение");
        }
            , [cmd](const caf::error& )
        {
            cmd->sendFailed("Ошибка записи параметра: таймаут");
        }
        );
    }
}
}