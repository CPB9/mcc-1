#pragma once
#include <string>
#include <sqlite3pp.h>

namespace mccdb {

class DbHandler
{
public:
    DbHandler();
    ~DbHandler();

    bool open(const std::string& mccDbPath, const std::string& dbSchema);
    void close();
    inline sqlite3pp::database& db() { return _dbpp; }
private:
    bmcl::Option<std::string> fillHash(const std::string& shema_hash);
    bmcl::Option<std::string> checkHash(const std::string& shema_hash);
    bmcl::Option<std::string> create(const std::string& mccDbPath, const std::string& dbSchema, bool needFill);
    sqlite3pp::database _dbpp;
};

}
