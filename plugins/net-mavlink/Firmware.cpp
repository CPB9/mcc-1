#include "Firmware.h"

#include <QDomDocument>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

#include <bmcl/MakeRc.h>
#include <bmcl/Logging.h>
#include <bmcl/Buffer.h>
#include <bmcl/StringView.h>
#include <bmcl/OptionRc.h>

#include <fmt/format.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>

namespace mccmav {

const char* toString(MAV_PARAM_TYPE type)
{
    switch (type)
    {
    case MAV_PARAM_TYPE_UINT8: return "u8";
    case MAV_PARAM_TYPE_INT8: return "i8";
    case MAV_PARAM_TYPE_UINT16: return "u16";
    case MAV_PARAM_TYPE_INT16: return "i16";
    case MAV_PARAM_TYPE_UINT32: return "u32";
    case MAV_PARAM_TYPE_INT32: return "i32";
    case MAV_PARAM_TYPE_UINT64: return "u64";
    case MAV_PARAM_TYPE_INT64: return "i64";
    case MAV_PARAM_TYPE_REAL32: return "f32";
    case MAV_PARAM_TYPE_REAL64: return "f64";
    default:
        return "unknown";
    }
}

// std::string Firmware::createProperties(const std::string& longDesc, const std::string& shortDesc, bmcl::Option<double> min, bmcl::Option<double> max, bmcl::Option<double> defaultValue) const
// {
//     std::string descProp = longDesc;
//     if (min.isSome())
//         descProp = fmt::format("{}\nМинимум: {}", descProp, min.unwrap());
//     if (max.isSome())
//         descProp = fmt::format("{}\nМаксимум: {}", descProp, max.unwrap());
//     if (defaultValue.isSome())
//         descProp = fmt::format("{}\nПо умолчанию: {}", descProp, defaultValue.unwrap());
//
//     QJsonObject propsObj;
//     propsObj["readonly"] = false;
//     propsObj["description"] = QString::fromStdString(descProp);
//     propsObj["short_description"] = QString::fromStdString(shortDesc);
//     if(defaultValue.isSome())
//         propsObj["default"] = QString::fromStdString(fmt::format("{}", defaultValue.unwrap()));
//
//     QJsonDocument doc;
//     doc.setObject(propsObj);
//     return doc.toJson(QJsonDocument::Compact).toStdString();
// }

Firmware::Firmware(const mccmsg::ProtocolValue& id, MAV_AUTOPILOT autopilot, const std::vector<ParamValue>& params, const mccmsg::PropertyDescriptionPtrs& req, const mccmsg::PropertyDescriptionPtrs& opt)
    : mccmsg::IFirmware(id, req, opt), _autopilotBoard(autopilot)
{
    loadXml(params);
}

Firmware::Firmware(const mccmsg::ProtocolValue& id, MAV_AUTOPILOT autopilot, const std::vector<ParameterDescription>& params, const mccmsg::PropertyDescriptionPtrs& req, const mccmsg::PropertyDescriptionPtrs& opt)
    : mccmsg::IFirmware(id, req, opt), _autopilotBoard(autopilot), _paramsDescription(params)
{
}

Firmware::~Firmware()
{
}

void Firmware::loadXml(const std::vector<ParamValue>& params)
{
    QFile file(":/net-mavlink/device/PX4ParameterFactMetaData.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        BMCL_WARNING() << "Can not open PX4ParameterFactMetaData.xml";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        BMCL_WARNING() << "Can not read PX4ParameterFactMetaData.xml";
        return;
    }

    QDomNodeList xmlGroups = doc.elementsByTagName("group");
    for (int i = 0; i < xmlGroups.size(); i++) {
        QDomNode n = xmlGroups.item(i);
        if (!n.isElement())
            continue;

        QDomElement xmlElement = n.toElement();

        QDomAttr groupNameAttr = xmlElement.attributeNode("name");
        if (groupNameAttr.isNull())
        {
            Q_ASSERT(false);
            continue;
        }

        std::string groupName = groupNameAttr.value().toStdString();

        QDomNodeList parametersNodeList = n.childNodes();
        for (auto j = 0; j < parametersNodeList.size(); ++j)
        {
            QDomNode paramNode = parametersNodeList.item(j);
            if (paramNode.isNull())
            {
                Q_ASSERT(false);
                continue;
            }

            QDomElement paramElement = paramNode.toElement();
            QString defaultAttr = paramElement.attribute("default");
            QString nameAttr = paramElement.attribute("name");
            QString typeAttr = paramElement.attribute("type");

            QDomElement shortDescElement = paramElement.firstChildElement("short_desc");
            QDomElement longDescElement = paramElement.firstChildElement("long_desc");
            QDomElement unitElement = paramElement.firstChildElement("unit");
            QDomElement minElement = paramElement.firstChildElement("min");
            QDomElement maxElement = paramElement.firstChildElement("max");

            auto paramIt = std::find_if(params.begin(), params.end(), [nameAttr](const ParamValue& p) { return p.name == nameAttr.toStdString(); });
            if(paramIt == params.end())
                continue;

            ParameterDescription description;
            description.index = paramIt->index;
            description.name = paramIt->name;
            description.defaultValue = defaultAttr.toDouble();
            description.type = typeAttr.toStdString();
            description.shortDesc = shortDescElement.text().toStdString();
            description.longDesc = longDescElement.text().toStdString();
            description.min = minElement.text().toDouble();
            description.max = maxElement.text().toDouble();
            description.category = groupName;

            QDomElement valuesElement = paramElement.firstChildElement("values");
            if (!valuesElement.isNull())
            {
                auto valuesCodes = valuesElement.childNodes();

                for (auto z = 0; z < valuesCodes.size(); ++z)
                {
                    auto valueCodeElement = valuesCodes.item(z).toElement();
                    QString valueCode = valueCodeElement.attribute("code");
                    description.values[valueCode.toInt()] = valueCodeElement.text().toStdString();
                }
            }

            _paramsDescription.emplace_back(std::move(description));
        }
    }
    file.close();

    for (const auto& p : params)
    {
        auto it = std::find_if(_paramsDescription.begin(), _paramsDescription.end(), [&p](const ParameterDescription& desc) { return desc.name == p.name; });
        if (it != _paramsDescription.end())
        {
            it->type = toString(p.type);
            continue;
        }

        ParameterDescription desc;
        desc.name = p.name;
        desc.index = p.index;
        desc.defaultValue = 0.0;
        desc.shortDesc = "";
        desc.longDesc = "";
        desc.unit = "";
        desc.min = 0.0;
        desc.max = 0.0;
        desc.type = toString(p.type);
        desc.category = "!Root";

        _paramsDescription.emplace_back(std::move(desc));
    }

    std::sort(_paramsDescription.begin(), _paramsDescription.end(), [](const ParameterDescription& a, const ParameterDescription& b) { return a.index < b.index; });

}

bmcl::Buffer Firmware::encode() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    writer.StartObject();
    writer.Key("name");
    writer.String(id().value());
    writer.Key("autopilot");
    writer.Int(_autopilotBoard);
    writer.Key("params");
    writer.StartArray();
    for (const auto& p : _paramsDescription)
    {
        writer.StartObject();
        writer.Key("index");
        writer.Int(p.index);
        writer.Key("id");
        writer.String(p.name);
        writer.Key("defaultValue");
        writer.Double(p.defaultValue);
        writer.Key("shortInfo");
        writer.String(p.shortDesc);
        writer.Key("longInfo");
        writer.String(p.longDesc);
        writer.Key("unit");
        writer.String(p.unit);
        writer.Key("min");
        writer.Double(p.min);
        writer.Key("max");
        writer.Double(p.max);
        writer.Key("type");
        writer.String(p.type);
        writer.Key("category");
        writer.String(p.category);

        if (!p.values.empty())
        {
            writer.Key("values");
            writer.StartArray();
            for (const auto& v : p.values)
            {
                writer.StartObject();
                writer.Key("value");
                writer.Int(v.first);
                writer.Key("id");
                writer.String(v.second);
                writer.EndObject();
            }
            writer.EndArray();
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    assert(writer.IsComplete());
    bmcl::Buffer tmp;
    tmp.write(buffer.GetString(), buffer.GetSize());
    return tmp;
}

bmcl::OptionRc<const Firmware> Firmware::decode(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes)
{
    rapidjson::Document d;
    if (d.Parse((const char*)bytes.data(), bytes.size()).HasParseError())
    {
        assert(false);
        return bmcl::None;
    }

    assert(d.IsObject());
    assert(d.HasMember("name") && d["name"].IsString());
    assert(d.HasMember("params") && d["params"].IsArray());
    MAV_AUTOPILOT ap = MAV_AUTOPILOT_INVALID;
    if (d.HasMember("autopilot"))
        ap = (MAV_AUTOPILOT)d["autopilot"].GetInt();

    std::vector<ParameterDescription> descriptions;
    for (const auto& p : d["params"].GetArray())
    {
        assert(p.IsObject() && p.HasMember("id") && p.HasMember("defaultValue") && p.HasMember("shortInfo")
            && p.HasMember("longInfo") && p.HasMember("unit") && p.HasMember("min") && p.HasMember("max"));

        ParameterDescription desc;
        desc.index = p["index"].GetInt();
        desc.name = p["id"].GetString();
        desc.defaultValue = p["defaultValue"].GetDouble();
        desc.shortDesc = p["shortInfo"].GetString();
        desc.longDesc = p["longInfo"].GetString();
        desc.unit = p["unit"].GetString();
        desc.min = p["min"].GetDouble();
        desc.max = p["max"].GetDouble();
        desc.type = p["type"].GetString();
        desc.category = p["category"].GetString();

        if (p.HasMember("values"))
        {
            for (const auto& v : p["values"].GetArray())
            {
                double val       = v["value"].GetInt();
                std::string id   = v["id"].GetString();
                desc.values[val] = id;
            }
        }
        descriptions.emplace_back(std::move(desc));
    }
    assert(id.value() == d["name"].GetString());
    return bmcl::makeRc<const Firmware>(id, ap,  descriptions, mccmsg::PropertyDescriptionPtrs(), mccmsg::PropertyDescriptionPtrs());
}

bool Firmware::isPx4() const
{
    return _autopilotBoard == MAV_AUTOPILOT_PX4;
}

const std::vector<ParameterDescription>& Firmware::paramsDescription() const { return _paramsDescription; }

}
