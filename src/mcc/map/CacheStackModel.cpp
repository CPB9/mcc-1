#include "mcc/map/CacheStackModel.h"
#include "mcc/map/StackCache.h"
#include "mcc/map/OmcfCache.h"
#include "mcc/map/OsmBasicCache.h"
#include "mcc/map/CachePlugin.h"
#include "mcc/map/OnlineCache.h"
#include "mcc/ui/Settings.h"
#include "mcc/plugin/PluginCache.h"

#include <bmcl/Buffer.h>
#include <bmcl/Result.h>
#include <bmcl/MemReader.h>
#include <bmcl/Logging.h>

#include <QColor>
#include <QStandardPaths>

namespace mccmap {

static constexpr uint64_t currentMagic = 0xffffffffffff0001;

void CacheStackModel::readSettings(const QByteArray& state)
{
    bmcl::MemReader reader(state.data(), state.size());
    uint64_t magic = reader.readUint64Le();
    if (magic != currentMagic) {
        return;
    }
    _currentProj = (mccgeo::MercatorProjection::ProjectionType)reader.readUint8();

    uint64_t size = reader.readUint64Le();
    if (reader.readableSize() < size) {
        return;
    }
    _currentStaticMap = std::string((char*)reader.current(), size);
    reader.skip(size);

    while (!reader.isEmpty()) {
        if (reader.readableSize() < 3) {
            break;
        }
        uint type = reader.readUint8();
        bool isEnabled = reader.readUint8();
        if (type == 0) {
            reader.readUint8(); // для обратной совместимости со старыми версиями
        } else {
            if (reader.readableSize() < 8) {
                break;
            }

            uint64_t size = reader.readUint64Le();
            if (reader.readableSize() < size) {
                break;
            }

            if (type == 1) {
                QString path = QString::fromUtf8((char*)reader.current(), size);
                reader.skip(size);
                auto rv = OmcfCache::create(path);
                if (rv.isOk()) {
                    _stack->append(rv.unwrap().get());
                    _stack->setEnabled(_stack->size() - 1, isEnabled);
                }
            } else {
                std::string fullName((char*)reader.current(), size);
                reader.skip(size);
                auto it = _onlineCachesMap.find(fullName);
                if (it != _onlineCachesMap.end()) {
                    _stack->append(it->second.get());
                    _stack->setEnabled(_stack->size() - 1, isEnabled);
                }
            }
        }
    }
}

CacheStackModel::CacheStackModel(mccui::Settings* settings, QObject* parent)
    : QAbstractTableModel(parent)
    , _currentProj(mccgeo::MercatorProjection::SphericalMercator)
    , _isEnabled(false)
    , _settings(settings)
{
    auto defaultCacheLoc = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    _cachePathReader = settings->acquireReader("map/webmapCachePath", defaultCacheLoc);
    _stackStateWriter = settings->acquireUniqueWriter("map/webmapCacheStackState").unwrap();

    _stack = StackCache::create();
    _builtinCache = OnlineCache::create<OsmBasicCache>();
    _builtinCache->setPath(_cachePathReader->read().toString(), _builtinCache->createServiceName()); //HACK
    _onlineCaches.insert(_onlineCaches.begin(), _builtinCache);
    _onlineCachesMap[_builtinCache->fullName()] = _builtinCache;
    _stack->setProjection(_builtinCache->projection());

    updateEnabled();
    if (!hasOnlineMap(_currentStaticMap)) {
        _currentStaticMap = _builtinCache->fullName();
    }
    emit stackChanged();
}

void CacheStackModel::loadPlugins(const mccplugin::PluginCache* cache)
{
    _onlineCaches.clear();
    _onlineCachesMap.clear();

    _onlineCaches.insert(_onlineCaches.begin(), _builtinCache);

    for (const mccplugin::PluginPtr& plugin : cache->plugins()) {
        if (plugin->hasTypeId(CachePlugin::id)) {
            OnlineCache* cache = static_cast<const CachePlugin*>(plugin.get())->cache();
            _onlineCaches.push_back(cache);
        }
    }

    for (const auto& cache : _onlineCaches) {
        QString base = _cachePathReader->read().toString();
        cache->setPath(base, cache->createServiceName());
        _onlineCachesMap[cache->fullName()] = cache;
    }

    _cachePathReader->onChange(this, [this]() {
        QString path = _cachePathReader->read().toString();
        for (const auto& cache : _onlineCaches) {
            cache->setBasePath(path);
        }
    });

    QByteArray state = _stackStateWriter->read().toByteArray();

    restoreOld(state);

    saveSettings();
    emit stackChanged();
}

bool CacheStackModel::hasOnlineMap(const std::string& fullName)
{
    return _onlineCachesMap.find(fullName) != _onlineCachesMap.end();
}

void CacheStackModel::saveSettings() const
{
    QByteArray state = saveOld();
    _stackStateWriter->write(state);
}

CacheStackModel::~CacheStackModel()
{
    saveSettings();
}

void CacheStackModel::setOnlineCachePath(const QString& path)
{
    for (const Rc<OnlineCache>& cache : _onlineCaches) {
        cache->setBasePath(path);
    }
}

void CacheStackModel::selectOnlineMap(const std::string& fullName)
{
    beginResetModel();
    if (hasOnlineMap(fullName)) {
        _currentStaticMap = fullName;
    }
    endResetModel();
}

void CacheStackModel::addOmcfCache(const QString& path)
{
    auto newCache = OmcfCache::create(path);
    if (newCache.isErr()) {
        return;
    }

    beginInsertRows(QModelIndex(), 0, 0);
    _stack->prepend(newCache.unwrap().get());
    bool isEnabled = newCache.unwrap()->projection().isA(_currentProj);
    _stack->setEnabled(0, isEnabled);
    endInsertRows();
}

bool CacheStackModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || (row + count) > _stack->size()) {
        return false;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    //TODO: optimize
    for (int i = 0; i < count; i++) {
        _stack->removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool CacheStackModel::moveRows(const QModelIndex& sourceParent, int srcFirst, int count, const QModelIndex& destinationParent, int destFirst)
{
    auto size = _stack->size();
    if (count == 0 || size <= count) {
        return false;
    }
    if (srcFirst < 0 || destFirst < 0) {
        return false;
    }
    if (srcFirst >= size || destFirst > size) {
        return false;
    }
    if ((srcFirst + count) > size) {
        return false;
    }

    beginMoveRows(sourceParent, srcFirst, srcFirst + count - 1, destinationParent, destFirst);
    _stack->move(srcFirst, count, destFirst);
    endMoveRows();

    return true;
}

bool CacheStackModel::canRemoveAt(int index)
{
    return !_stack->at(index)->isBuiltIn();
}

int CacheStackModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return (int)_stack->size();
}

int CacheStackModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant CacheStackModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        if (_stack->isEnabled(index.row())) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    }

    auto cache = _stack->at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return cache->name();
        case 1:
            return cache->description();
        case 2:
            if (cache->isBuiltIn()) {
                return "Да";
            } else {
                return "Нет";
            }
        case 3:
            if (cache->projection().isA(mccgeo::MercatorProjection::SphericalMercator)) {
                return "Сферический меркатор";
            } else if (cache->projection().isA(mccgeo::MercatorProjection::EllipticalMercator)) {
                return "Эллиптический меркатор";
            } else {
            }
        }
    } else if (role == Qt::BackgroundColorRole) {
        if (index.column() == 0 && cache->isBuiltIn()) {
            auto onlineCache = static_cast<const OnlineCache*>(cache);
            if (onlineCache->fullName() == _currentStaticMap) {
                return QColor(qRgb(251, 128, 114));
            }
        }
        if (index.column() == 2) {
            if (cache->isBuiltIn()) {
                return QColor(qRgb(230, 245, 201));
            } else {
                return QColor(qRgb(253, 205, 172));
            }
        }
        if (index.column() == 3) {
            if (cache->projection().isA(mccgeo::MercatorProjection::SphericalMercator)) {
                return QColor(qRgb(179, 226, 205));
            } else if (cache->projection().isA(mccgeo::MercatorProjection::EllipticalMercator)) {
                return QColor(qRgb(203, 213, 232));
            }
        }
    }

    return QVariant();
}

QVariant CacheStackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Vertical) {
        return section + 1;
    } else {
        switch (section) {
        case 0:
            return "Название";
        case 1:
            return "Описание";
        case 2:
            return "Встроенная";
        case 3:
            return "Проекция";
        }
    }

    return QVariant();
}

Qt::ItemFlags CacheStackModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

void CacheStackModel::updateEnabled()
{
    for (std::size_t i = 0; i < _stack->size(); i++) {
        if (!_stack->at(i)->projection().isA(_currentProj)) {
            _stack->setEnabled(i, false);
            QModelIndex newIndex = createIndex((int)i, 0);
            emit dataChanged(newIndex, newIndex);
        }
    }
}

bool CacheStackModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        _stack->setEnabled(index.row(), value.toBool());
        if (_stack->at(index.row())->projection().isA(mccgeo::MercatorProjection::SphericalMercator)) {
            _currentProj = mccgeo::MercatorProjection::SphericalMercator;
        } else {
            _currentProj = mccgeo::MercatorProjection::EllipticalMercator;
        }
        _stack->setProjection(_currentProj);
        updateEnabled();
        return true;
    }

    return false;
}

void CacheStackModel::setEnabled(bool isEnabled)
{
    _isEnabled = isEnabled;
    emit enabled(isEnabled);
}

OnlineCache* CacheStackModel::onlineMapByName(const std::string& fullName)
{
    auto it = _onlineCachesMap.find(fullName);
    if (it == _onlineCachesMap.end()) {
        return _builtinCache.get();
    }
    return it->second.get();
}

void CacheStackModel::apply()
{
    emit stackChanged();
}

QByteArray CacheStackModel::saveOld() const
{
    bmcl::Buffer state;
    state.writeUint64Le(currentMagic);
    state.writeUint8((uint8_t)_currentProj);
    state.writeUint64Le(_currentStaticMap.size());
    state.write(_currentStaticMap.data(), _currentStaticMap.size());
    for (std::size_t i = 0; i < _stack->size(); i++) {
        const FileCache* ptr = _stack->at(i);
        if (const OnlineCache* onlineCache = dynamic_cast<const OnlineCache*>(ptr)) {
            state.writeUint8(2);
            state.writeUint8(_stack->isEnabled(i));
            const std::string& name = onlineCache->fullName();
            state.writeUint64Le(name.size());
            state.write(name.data(), name.size());
        } else if (const OmcfCache* omcfCache = dynamic_cast<const OmcfCache*>(ptr)) {
            state.writeUint8(1);
            state.writeUint8(_stack->isEnabled(i));
            QByteArray path = omcfCache->path().toUtf8();
            state.writeUint64Le(path.size());
            state.write(path.data(), path.size());
        } else {
            assert(false);
        }
    }
    return QByteArray((char*)state.data(), (int)state.size());
}

void CacheStackModel::restoreOld(const QByteArray& state)
{
    _stack->clear();
    if (state.isEmpty() || state.size() < 3) {
        _stack->setProjection(_currentProj);
        for (const Rc<OnlineCache>& cache : _onlineCaches) {
            _stack->append(cache.get());
        }
    } else {
        readSettings(state);
        _stack->setProjection(_currentProj);
        for (const Rc<OnlineCache>& onlineCache : _onlineCaches) {
            if (!_stack->hasOnlineCache(onlineCache->fullName())) {
                _stack->append(onlineCache.get());
            }
        }
    }

    updateEnabled();
    if (!hasOnlineMap(_currentStaticMap)) {
        _currentStaticMap = _builtinCache->fullName();
    }
}
}

