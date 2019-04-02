#include <bmcl/Buffer.h>
#include <bmcl/ColorStream.h>
#include <bmcl/StringView.h>

#include <QFile>
#include <QDir>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <string>
#include <utility>
#include <fstream>

BMCL_NORETURN void exitWithError(const QString& msg)
{
    bmcl::ColorStdError out;
    out << bmcl::ColorAttr::FgRed << "mcc-res-gen: ";
    out << bmcl::ColorAttr::Reset << msg.toStdString() << std::endl;
    std::exit(-1);
}

struct ResDesc {
    ResDesc()
        : name("")
        , isReplaced(false)
    {
    }

    const char* name;
    QString path;
    QByteArray fileData;
    bool isReplaced;
};

struct Data {
    bmcl::Buffer serBuf;
    rapidjson::Document jsonDoc;
    rapidjson::Document replaceJsonDoc;
    std::map<std::string, ResDesc> resources;
};

inline void writeString(bmcl::Buffer* dest, bmcl::StringView str)
{
    dest->write(str.data(), str.size());
}

static const char* _enumHeader = "\
#pragma once\n\
\n\
#include \"mcc/res/Config.h\"\n\
\n\
namespace mccres {\n\
\n\
enum class ResourceKind {\n\
"
;

static const char* _resHeader = "\
#include \"mcc/res/Config.h\"\n\
#include \"mcc/res/ResourceKind.h\"\n\
\n\
#include <bmcl/Bytes.h>\n\
#include <bmcl/Logging.h>\n\
\n\
#include <type_traits>\n\
\n\
#include <cstdint>\n\
#include <cassert>\n\
\n\
namespace mccres {\n\
\n\
"
;

void saveFileIfDifferent(const char* srcPath, const char* fname, bmcl::Bytes data)
{
    QString path = srcPath;
    path += QDir::separator();
    path += fname;
    QFile file(path);
    QByteArray prevData;
    if (file.exists()) { //trying to avoid recompile
        if (!file.open(QFile::ReadOnly)) {
            exitWithError("failed to open file: " + file.errorString());
        }
        prevData = file.readAll();
        if (bmcl::Bytes((const uint8_t*)prevData.data(), prevData.size()) == data) {
            return;
        }
        file.close();
    }
    if (!file.open(QFile::WriteOnly)) {
        exitWithError("failed to open file: " + file.errorString());
    }
    file.write((const char*)data.data(), data.size());
    if (file.error() != QFile::NoError) {
        exitWithError(QString("failed to read file: ") + file.errorString());
    }
}

static inline char nibbleToHex(uint8_t nibble)
{
    switch(nibble) {
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return 'a';
    case 11:
        return 'b';
    case 12:
        return 'c';
    case 13:
        return 'd';
    case 14:
        return 'e';
    case 15:
        return 'f';
    }
    assert(false);
    return '#';
}

void parseJsonDoc(const char* jsonPath, rapidjson::Document* doc)
{
    std::ifstream ifs(jsonPath);
    rapidjson::IStreamWrapper isw(ifs);

    doc->ParseStream(isw);
    if (doc->HasParseError()) {
        exitWithError(QString("failed to parse resource file: ") + rapidjson::GetParseError_En(doc->GetParseError()));
    }

    if (!doc->IsObject()) {
        exitWithError("root json element should be object");
    }
}

int main(int argc, char** argv)
{
    if (argc != 6) {
        exitWithError("expected three arguments: src json, replace json, src folder, dest folder");
    }

    const char* argSrcJson = argv[1];
    const char* argReplaceJson = argv[2];
    const char* argSrcFolder = argv[3];
    const char* argReplaceSrcFolder = argv[4];
    const char* argDestFolder = argv[5];

    Data data;

    parseJsonDoc(argSrcJson, &data.jsonDoc);
    parseJsonDoc(argReplaceJson, &data.replaceJsonDoc);

    QString srcFolder = argSrcFolder;
    srcFolder += QDir::separator();

    QString replaceSrcFolder = argReplaceSrcFolder;
    replaceSrcFolder += QDir::separator();

    //data.resources.reserve(data.jsonDoc.GetObject().MemberCount());

    std::size_t totalSizeOfFiles = 0;

    //build from default file
    for (const auto& kv : data.jsonDoc.GetObject()) {
        if (!kv.name.IsString()) {
            exitWithError("resource keys must be strings");
        }
        if (!kv.value.IsString()) {
            exitWithError("resource values must be strings");
        }
        auto it = data.resources.emplace(std::piecewise_construct,
                                         std::forward_as_tuple(kv.name.GetString()),
                                         std::forward_as_tuple());
        it.first->second.name = kv.name.GetString();
        it.first->second.path = kv.value.GetString();
    }

    //replace resources
    for (const auto& kv : data.replaceJsonDoc.GetObject()) {
        if (!kv.name.IsString()) {
            exitWithError("resource keys must be strings");
        }
        if (!kv.value.IsString()) {
            exitWithError("resource values must be strings");
        }
        auto it = data.resources.emplace(std::piecewise_construct,
                                         std::forward_as_tuple(kv.name.GetString()),
                                         std::forward_as_tuple());
        if (it.second) {
            exitWithError(QString("no resource with name to replace in default file: ") + kv.name.GetString());
        }
        it.first->second.path = kv.value.GetString();
        it.first->second.isReplaced = true;
    }

    for (auto& kv : data.resources) {
        ResDesc& desc = kv.second;

        if (desc.path.isEmpty()) {
            continue;
        }

        if (desc.isReplaced) {
            desc.path = replaceSrcFolder + desc.path;
        } else {
            desc.path = srcFolder + desc.path;
        }

        QFile file(desc.path);
        if (!file.open(QFile::ReadOnly)) {
            exitWithError(QString("failed to open file: ") + desc.path);
        }
        desc.fileData = file.readAll();
        if (file.error() != QFile::NoError) {
            exitWithError(QString("failed to read file: ") + desc.path);
        }

        totalSizeOfFiles += desc.fileData.size();
    }

    data.serBuf.resize(0);
    writeString(&data.serBuf, _enumHeader);
    for (const auto& desc : data.resources) {
        writeString(&data.serBuf, "    ");
        writeString(&data.serBuf, desc.second.name);
        writeString(&data.serBuf, ",\n");
    }
    writeString(&data.serBuf, "};\n}\n");

    saveFileIfDifferent(argDestFolder, "_generated_ResourceKind.h", data.serBuf);

    data.serBuf.resize(0);
    writeString(&data.serBuf, _resHeader);

    writeString(&data.serBuf, "static const std::uint8_t _data[");
    writeString(&data.serBuf, std::to_string(totalSizeOfFiles + 1)); //1 byte reserved
    writeString(&data.serBuf, "] = {\n");

    char byteStr[6];
    byteStr[0] = '0';
    byteStr[1] = 'x';
    byteStr[2] = '_';
    byteStr[3] = '_';
    byteStr[4] = ',';
    byteStr[5] = ' ';
    for (const auto& desc : data.resources) {
        for (uint8_t byte : desc.second.fileData) {
            byteStr[2] = nibbleToHex((byte & 0xf0) >> 4);
            byteStr[3] = nibbleToHex(byte & 0x0f);
            writeString(&data.serBuf, bmcl::StringView::fromStaticArray(byteStr));
        }
    }

    writeString(&data.serBuf, "0x00\n};\n\n"); // 1 additional byte

    writeString(&data.serBuf, "bmcl::Bytes _generated_loadResource(ResourceKind kind)\n{\n    switch (kind) {\n");

    std::size_t currentOffset = 0;
    for (const auto& desc : data.resources) {
        writeString(&data.serBuf, "    case ResourceKind::");
        writeString(&data.serBuf, desc.second.name);
        writeString(&data.serBuf, ":\n        return bmcl::Bytes(&_data[");
        writeString(&data.serBuf, std::to_string(currentOffset));
        writeString(&data.serBuf, "], ");
        writeString(&data.serBuf, std::to_string(desc.second.fileData.size()));
        writeString(&data.serBuf, ");\n");

        currentOffset += desc.second.fileData.size();
    }

    writeString(&data.serBuf, "    }\n    BMCL_CRITICAL() << \"Invalid resource ID: \""
                              " << std::underlying_type<ResourceKind>::type(kind);\n  "
                              "  return bmcl::Bytes(&_data[0], 0);\n}\n}\n");

    saveFileIfDifferent(argDestFolder, "_generated_ResourceImpl.cpp", data.serBuf);

    return 0;
}
