#pragma once
#include "mcc/Config.h"
#include <memory>
#include <string>
#include <caf/event_based_actor.hpp>
#include "mcc/plugin/Fwd.h"

namespace mccdb {

namespace dbobj { class DbObjInternal; }

class MCC_DB_DECLSPEC DbObj : public caf::event_based_actor
{
    friend class ReqVisitorDb;
    friend class NoteVisitorDb;
public:
    DbObj(caf::actor_config& cfg);
    DbObj(caf::actor_config& cfg, std::string&& binPath);
    ~DbObj() override;
    caf::behavior make_behavior() override;
    const char* name() const override;
    void on_exit() override;
private:
    std::string _binPath;
    const std::string _dbName = "mcc.sqlite";
    const std::string _dbSchema = ":/db/obj/Schema.sql";
    std::unique_ptr<dbobj::DbObjInternal> _db;
};

}
