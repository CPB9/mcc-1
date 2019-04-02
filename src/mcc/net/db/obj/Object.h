#pragma once
#include "mcc/net/Error.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/DbObjInternal.h"
#include <bmcl/UuidHash.h>
#include <unordered_map>

namespace mccdb {
namespace dbobj {

inline void myReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    std::string::size_type pos = 0u;
    while ((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

using ObjectId = int64_t;

template<typename T, typename N>
class Objects
{
public:
    T add(ObjectId i, const N& n, T t)
    {
        _byId[i] = t;
        _byName[n] = t;
        return t;
    }
    T get(ObjectId id) const
    {
        const auto i = _byId.find(id);
        if (i == _byId.end())
            return nullptr;
        return i->second;
    }
    T get(const N& name, ObjectId* id = nullptr) const
    {
        const auto i = _byName.find(name);
        if (i == _byName.end())
            return nullptr;
        if (id == nullptr)
            return i->second;
        for (const auto& j : _byId)
        {
            if (j.second == i->second)
            {
                *id = j.first;
                return i->second;
            }
        }
        assert(false);
        return i->second;
    }
    void remove(ObjectId i)
    {
        const auto j = _byId.find(i);
        if (j == _byId.end())
            return;
        T t = j->second;
        _byId.erase(j);
        for (const auto& k : _byName)
        {
            if (k.second == t)
            {
                _byName.erase(k.first);
                break;
            }
        }
    }
    void remove(const N& n)
    {
        const auto j = _byName.find(n.uuid());
        if (j == _byName.end())
            return;
        T t = j->second;
        _byName.erase(j);

        for (const auto& i : _byId)
        {
            if (i.second == t)
            {
                _byId.erase(i.first);
                break;
            }
        }
    }
private:
    std::unordered_map<ObjectId, T> _byId;
    std::unordered_map<bmcl::Uuid, T> _byName;
};

template<typename T, typename N>
class QueryObject
{
public:
    using Getter = std::function<bmcl::Option<T>(const sqlite3pp::selecter::row&)>;
    using Inserter = std::function<bmcl::Result<N, caf::error>(const T&, ObjectId& id)>;
    QueryObject(DbObjInternal* db, const std::string& table, const std::string& fields, const Getter& getter, const Inserter& inserter)
        : _db(db)
        , _table(table)
        , _getter(getter)
        , _inserter(inserter)
        , _queryId(db->db())
        , _queryName(db->db())
        , _queryList(db->db())
        , _queryOne(db->db())
        , _queryAll(db->db())
        , _queryReg(db->db())
        , _queryRemoveOne(db->db())
    {
        std::string fields_set = fmt::format("id, name, info, {}", fields);

        sql_prepare(_queryId, fmt::format("select id from {} where name = :name;", _table).c_str());
        sql_prepare(_queryName, fmt::format("select name from {} where id = :id;", _table).c_str());
        sql_prepare(_queryList, fmt::format("select name from {};", _table).c_str());
        sql_prepare(_queryOne, fmt::format("select {} from {} where name = :name;", fields_set, _table).c_str());
        sql_prepare(_queryAll, fmt::format("select {} from {};", fields_set, _table).c_str());
        sql_prepare(_queryRemoveOne, fmt::format("delete from {} where name = :name;", _table).c_str());

        std::string ins_fields_set = fields_set;
        myReplace(ins_fields_set, ", ", ", :");
        sql_prepare(_queryReg, fmt::format("insert into {} ({}) values (:{});", table, fields_set, ins_fields_set).c_str());
    }
    bmcl::Result<std::vector<N>, caf::error> getList()
    {
        _queryList.reset();
        print(exec(&_queryList));
        std::vector<N> items;
        while (_queryList.next())
        {
            auto row = _queryList.get_row();
            auto name = N::createFromString(row.template get<bmcl::StringView>("name"));
            items.emplace_back(name.unwrap());
        }
        return items;
    }
    bmcl::Result<T, caf::error> getOne(const N& name, ObjectId* id = nullptr, bool forceUpdate = false)
    {
        if (!forceUpdate)
        {
            auto t = _objs.get(name, id);
            if (!t.isNull())
                return t;
        }

        _queryOne.reset();
        std::string s = name.toStdString();
        print(binds(&_queryOne, ":name", s, sqlite3pp::nocopy));
        auto p = print(exec(&_queryOne));
        if (p.isSome())
            return mccmsg::make_error(mccmsg::Error::NotFound, p.take());
        if (!_queryOne.next())
            return mccmsg::make_error(mccmsg::Error::NotFound);
        auto row = _queryOne.get_row();
        auto r = _getter(row);
        if (r.isNone())
            return mccmsg::make_error(mccmsg::Error::InconsistentData);

        ObjectId i = row.template get<ObjectId>("id");
        _objs.add(i, name, r.unwrap());
        if (id)
            *id = i;
        return r.take();
    }
    bmcl::Result<T, caf::error> getOne(ObjectId id, bool forceUpdate = false)
    {
        auto t = _objs.get(id);
        if (!t.isNull())
            return t;

        auto name = getName(id);
        if (name.isNone())
            return mccmsg::make_error(mccmsg::Error::NotFound);
        return getOne(name.unwrap(), nullptr, forceUpdate);
    }
    bmcl::Result<std::vector<T>, caf::error> getAll()
    {
        _queryAll.reset();
        auto p = print(exec(&_queryAll));
        if (p.isSome())
            return mccmsg::make_error(mccmsg::Error::CantGet, p.take());
        std::vector<T> pds;
        while (_queryAll.next())
        {
            auto row = _queryAll.get_row();
            ObjectId id = row.template get<int64_t>("id");
            auto t = _objs.get(id);
            if (!t.isNull())
            {
                pds.emplace_back(t);
            }
            else
            {
                auto r = _getter(row);
                if (r.isNone())
                {
                    BMCL_WARNING() << "Невалидный кеш! ";// << r.unwrap().name;
                    _objs.remove(id);
                    continue;
                }
                auto t = r.take();
                _objs.add(id, t->name(), t);
                pds.emplace_back(t);
            }
        }
        return pds;
    }
    bmcl::Option<ObjectId> getId(const N& name)
    {
        _queryId.reset();
        print(binds(&_queryId, ":name", name.toStdString(), sqlite3pp::copy));
        print(exec(&_queryId));

        if (_queryId.next())
            return _queryId.get_row().template get<ObjectId>("id");
        return bmcl::None;
    }
    bmcl::Option<N> getName(ObjectId id)
    {
        _queryName.reset();
        print(binds(&_queryName, ":id", id));
        print(exec(&_queryName));
        if (_queryName.next())
        {
            auto n = N::createFromString(_queryName.get_row().template get<bmcl::StringView>("name"));
            return N(n.unwrapOption().unwrapOr(N::createNil()));
        }
        return bmcl::None;
    }
    bmcl::Option<caf::error> removeOne(const N& name)
    {
        _objs.remove(name);
        _queryRemoveOne.reset();
        std::string s = name.toStdString();
        print(binds(&_queryRemoveOne, ":name", s, sqlite3pp::nocopy));
        auto p = print(exec(&_queryRemoveOne));
        if (p.isSome())
            return mccmsg::make_error(mccmsg::Error::NotFound, p.take());
        return bmcl::None;
    }
    bmcl::Option<T> updated(const N& name)
    {
        auto r = getOne(name, nullptr, true);
        if (r.isErr())
        {
            assert(false);
            return bmcl::None;
        }
        _db->updated(r.unwrap());
        return r.take();
    }
    void registered(const N& name, bool isRegistered)
    {
        if (!isRegistered)
            _objs.remove(name);
        _db->registered(name, isRegistered);
    }

private:
    Objects<T, N> _objs;

    std::string _table;
    Getter _getter;
    Inserter _inserter;
    sqlite3pp::selecter _queryId;
    sqlite3pp::selecter _queryName;
    sqlite3pp::selecter _queryList;
    sqlite3pp::selecter _queryOne;
    sqlite3pp::selecter _queryAll;
    sqlite3pp::statement _queryRemoveOne;
protected:
    DbObjInternal* _db;
    sqlite3pp::inserter _queryReg;
};

extern template class QueryObject<mccmsg::ChannelDescription, mccmsg::Channel>;
extern template class QueryObject<mccmsg::DeviceDescription, mccmsg::Device>;
extern template class QueryObject<mccmsg::DeviceUiDescription, mccmsg::DeviceUi>;
extern template class QueryObject<mccmsg::FirmwareDescription, mccmsg::Firmware>;
extern template class QueryObject<mccmsg::ProtocolDescription, mccmsg::Protocol>;
extern template class QueryObject<mccmsg::RadarDescription, mccmsg::Radar>;
extern template class QueryObject<mccmsg::TmSessionDescription, mccmsg::TmSession>;

}
}
