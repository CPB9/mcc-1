#include <QFile>
#include <fmt/format.h>
#include <bmcl/Logging.h>
#include <bmcl/Buffer.h>
#include <bmcl/String.h>
#include <bmcl/Sha3.h>
#include <bmcl/Panic.h>
#include "mcc/net/db/DbHandler.h"

namespace mccdb {

DbHandler::DbHandler()
{
}

DbHandler::~DbHandler()
{
    close();
}

void DbHandler::close()
{
    if (!_dbpp.is_connected())
        return;

    auto r = _dbpp.disconnect();
    if (r.isSome())
    {
        BMCL_DEBUG() << "db is successfully closed";
        return;
    }
}

bmcl::Option<std::string> DbHandler::checkHash(const std::string& shema_hash)
{
    const char* stmt = "select value from property where name=\"schema_hash\";";
    sqlite3pp::selecter s(_dbpp, stmt);
    auto r = s.exec();
    if (r.isSome())
        return std::string("Не удалось проверить контрольную сумму схемы бд: ") + sqlite3pp::to_string(r.unwrap());
    if (!s.next())
        return std::string("Не удалось проверить контрольную сумму схемы бд: не найдено предыдущее значение!");
    auto row = s.get_row();
    bmcl::StringView stored_value = row.get<bmcl::StringView>("value");
    if (stored_value != shema_hash)
        return std::string("Контрольные суммы схемы бд не совпали! Удалите или сохраните БД!");
    return bmcl::None;
}

bmcl::Option<std::string> DbHandler::fillHash(const std::string& shema_hash)
{
    std::string stmt = fmt::format("insert into property (name, value) values (\"schema_hash\", \"{}\")", shema_hash);
    sqlite3pp::inserter i(_dbpp, stmt);
    auto r = i.exec();
    if (r.isSome())
        return std::string(sqlite3pp::to_string(r.unwrap()));
    return bmcl::None;
}

bmcl::Option<std::string> DbHandler::create(const std::string& mccDbPath, const std::string& dbSchema, bool needFill)
{
    auto r = _dbpp.connect(mccDbPath, sqlite3pp::OPEN_READWRITE | sqlite3pp::OPEN_CREATE);
    if (r.isSome())
        return std::string(sqlite3pp::to_string(r.unwrap()));

    QFile schema(QString::fromStdString(dbSchema));
    if (!schema.open(QIODevice::ReadOnly))
        return "failed to open db schema file: " + dbSchema;

    auto content = schema.readAll();
    auto buf = bmcl::Sha3<512>::calcInOneStep(bmcl::Bytes((const uint8_t*)content.data(), content.size()));
    std::string hash = bmcl::bytesToHexStringLower(buf);

    if (!needFill)
        return checkHash(hash);

    sqlite3pp::batch batch(_dbpp, bmcl::StringView(content.data(), content.size()), sqlite3pp::nocopy);
    r = batch.execute_all();
    if (r.isSome())
        return fmt::format("Не удалось failed to create db from schema file: {}, {}", sqlite3pp::to_string(r.unwrap()), _dbpp.err_msg().unwrapOr(""));

    return fillHash(hash);
}

bool DbHandler::open(const std::string& mccDbPath, const std::string& dbSchema)
{
    close();

    auto dbFilePath = QString::fromStdString(mccDbPath);
    bool needFill = !QFile::exists(dbFilePath);
    auto r = create(mccDbPath, dbSchema, needFill);
    if (r.isSome())
    {
        close();
        std::string e = fmt::format("{} {}\n", r.unwrap(), mccDbPath);
        bmcl::panic(e.c_str());
        return false;
    }

    return true;
}

}
