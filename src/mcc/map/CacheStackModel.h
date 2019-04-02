#pragma once

#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/Fwd.h"
#include "mcc/map/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/plugin/Fwd.h"

#include <QAbstractTableModel>

#include <unordered_map>

namespace mccmap {

class MCC_MAP_DECLSPEC CacheStackModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CacheStackModel(mccui::Settings* settings, QObject* parent = nullptr);
    ~CacheStackModel() override;

    void loadPlugins(const mccplugin::PluginCache* cache);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void selectOnlineMap(const std::string& fullName);
    void setOnlineCachePath(const QString& path);

    void addOmcfCache(const QString& path);
    bool canRemoveAt(int index);

    bool removeRows(int row, int count, const QModelIndex& parent) override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

    bool hasOnlineMap(const std::string& fullName);

    inline StackCache* stack() const;

    inline const std::string& currentStaticMap() const;
    inline mccgeo::MercatorProjection currentProjection() const;

    inline bool isEnabled() const;
    inline const std::vector<Rc<OnlineCache>>& caches() const;
    OnlineCache* onlineMapByName(const std::string& fullName);

    void apply();
    QByteArray saveOld() const;
    void restoreOld(const QByteArray& state);

public slots:
    void setEnabled(bool isEnabled);

signals:
    void stackChanged();
    void enabled(bool isEnabled);

private:
    void saveSettings() const;
    void updateEnabled();
    void readSettings(const QByteArray& state);

    Rc<StackCache> _stack;
    mccgeo::MercatorProjection::ProjectionType _currentProj;
    std::string _currentStaticMap;
    std::vector<Rc<OnlineCache>> _onlineCaches;
    Rc<OnlineCache> _builtinCache;
    std::unordered_map<std::string, Rc<OnlineCache>> _onlineCachesMap;
    bool _isEnabled;
    Rc<mccui::Settings> _settings;
    Rc<mccui::SettingsReader> _cachePathReader;
    Rc<mccui::SettingsWriter> _stackStateWriter;
};

inline mccgeo::MercatorProjection CacheStackModel::currentProjection() const
{
    return _currentProj;
}

inline StackCache* CacheStackModel::stack() const
{
    return _stack.get();
}

inline const std::string& CacheStackModel::currentStaticMap() const
{
    return _currentStaticMap;
}

inline bool CacheStackModel::isEnabled() const
{
    return _isEnabled;
}

inline const std::vector<Rc<OnlineCache>>& CacheStackModel::caches() const
{
    return _onlineCaches;
}
}
