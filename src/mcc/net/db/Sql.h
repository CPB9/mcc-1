#pragma once

#include "mcc/Config.h"

#include <bmcl/Logging.h>
#include <fmt/format.h>
#include <sqlite3pp.h>

#include "mcc/msg/NetVariant.h"

#include <utility>

#define exit_void(expr) {auto r = expr; if (r.isSome()) return;}
#define exit_val(expr) {auto r = expr; if (r.isSome()) return r.take();}
#define exit_ins(id, expr) int64_t id; {auto r = expr; if (r.isErr()) return r.takeErr(); else id = r.take();}

#define sql_prepare(query, expr) print(prepare(&query, expr), bmcl::LogLevel::Critical);
#define sql_bind(query, name, val) exit_val(print(binds(&query, name, val)));
#define sql_bind_str(query, name, val, c) exit_val(print(binds(&query, name, val, sqlite3pp::c)));
#define sql_insert(query, id) exit_ins(id, print(insert(query)));

namespace mccdb {

typedef bmcl::Option<std::string> SqlErrorX;
typedef bmcl::Result<int64_t, std::string> SqlInsErrorX;

inline SqlErrorX&& print(SqlErrorX&& val, bmcl::LogLevel level = bmcl::LogLevel::Debug)
{
    if (val.isSome())
    {
        BMCL_LOG(level) << val.unwrap().c_str();
    }
    return std::move(val);
}

inline SqlInsErrorX&& print(SqlInsErrorX&& val, bmcl::LogLevel level = bmcl::LogLevel::Debug)
{
    if (val.isErr())
    {
        BMCL_LOG(level) << val.unwrapErr().c_str();
    }
    return std::move(val);
}

inline bmcl::Option<std::string> prepare(sqlite3pp::statement* stmt, const char* text)
{
    auto r = stmt->prepare(text);
    if (r.isSome())
    {
        return fmt::format("failed to prepare {} {}", stmt->err_msg().unwrapOr(""), text);
    }
    return bmcl::None;
}

template<typename T>
inline SqlErrorX binds(sqlite3pp::statement* stmt, const char* name, T value)
{
    auto r = stmt->bind(name, value);
    if (r.isSome())
    {
        return fmt::format("failed to bind {}={} {}", name, stmt->err_msg().unwrapOr(""), stmt->sql().non_null());
    }
    return bmcl::None;
}

template<typename T>
inline SqlErrorX binds(sqlite3pp::statement* stmt, const char* name, T&& value, sqlite3pp::copy_semantic fcopy)
{
    auto r = stmt->bind(name, std::forward<T>(value), fcopy);
    if (r.isSome())
    {
        return fmt::format("failed to bind {} {} {}", name, stmt->err_msg().unwrapOr(""), stmt->sql().non_null());
    }
    return bmcl::None;
}

inline SqlErrorX binds(sqlite3pp::statement* stmt, const char* name, const mccmsg::NetVariant& value)
{
    switch (value.type())
    {
    case mccmsg::NetVariantType::Double: return binds(stmt, name, value.toDouble());
    case mccmsg::NetVariantType::Float:  return binds(stmt, name, value.toFloat());
    case mccmsg::NetVariantType::Bool:
    case mccmsg::NetVariantType::Int:
    case mccmsg::NetVariantType::Uint:   return binds(stmt, name, value.toInt());
    case mccmsg::NetVariantType::QString:
    case mccmsg::NetVariantType::String: return binds(stmt, name, value.stringify(), sqlite3pp::copy);
    case mccmsg::NetVariantType::None:   return binds(stmt, name, nullptr);
    default:
        break;
    }
    return bmcl::None;
}

inline SqlErrorX exec(sqlite3pp::statement* stmt)
{
    auto r = stmt->exec();
    if (r.isSome())
    {
        return fmt::format("failed to execute {} {}", stmt->sql().non_null(), stmt->err_msg().unwrapOr(""));
    }
    return bmcl::None;
}

inline SqlInsErrorX insert(sqlite3pp::inserter& stmt)
{
    auto r = stmt.insert();
    if (r.isErr())
    {
        return fmt::format("failed to insert {} {}", stmt.sql().non_null(), stmt.err_msg().unwrapOr(""));
    }
    return r.unwrap();
}

}
