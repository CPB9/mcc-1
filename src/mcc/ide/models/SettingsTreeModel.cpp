#include "mcc/ide/models/SettingsTreeModel.h"

#include "mcc/ui/SettingsPage.h"

#include <QDebug>

namespace mccide {

SettingsTreeItem::SettingsTreeItem(const QStringRef& name, bmcl::OptionPtr<mccui::SettingsPage> page, bmcl::OptionPtr<SettingsTreeItem> parent)
    : _name(name.toString())
    , _page(page)
    , _parent(parent)
{}

bool SettingsTreeItem::addChildPage(const QStringRef& path, mccui::SettingsPage* page)
{
    int idx = path.indexOf('/');
    if (idx < 0) {
        auto it = std::find_if(_children.begin(), _children.end(), [&](const std::unique_ptr<SettingsTreeItem>& child) {
            return child->name() == path;
        });

        if (it != _children.end()) {
            bmcl::OptionPtr<mccui::SettingsPage> childPage = it->get()->page();
            if (childPage.isSome()) {
                qWarning() << "page path conflict: " << page->pagePath();
                return false;
            }
            it->get()->setPage(page);
        } else {
            _children.emplace_back(new SettingsTreeItem(path, page, this));
        }
        return true;
    }

    QStringRef left = path.left(idx);
    QStringRef right = path.mid(idx + 1);

    if (left.size() == 0 || right.size() == 0) { //invalid name starting with '/' or ending with '/'
        qWarning() << "invalid settings page path: " << page->pagePath();
        return false;
    }

    auto it = std::find_if(_children.begin(), _children.end(), [&](const std::unique_ptr<SettingsTreeItem>& child) {
        return child->name() == left;
    });
    if (it == _children.end()) {
        _children.emplace_back(new SettingsTreeItem(left, bmcl::None, this));
        return _children.back()->addChildPage(right, page);
    }
    return it->get()->addChildPage(right, page);
}

bmcl::OptionPtr<SettingsTreeItem> SettingsTreeItem::childItem(int row)
{
    if (row >= _children.size() || row < 0) {
        return bmcl::None;
    }
    return _children[row].get();
}

int SettingsTreeItem::row() const
{
    if (_parent.isNone()) {
        return -1;
    }
    auto begin = _parent->_children.begin();
    auto end = _parent->_children.end();
    auto it = std::find_if(begin, end, [this](const std::unique_ptr<SettingsTreeItem>& item) {
        return item.get() == this;
    });
    if (it == end) {
        return -1;
    }
    return std::distance(begin, it);
}

int SettingsTreeItem::childCount() const
{
    return _children.size();
}

QIcon SettingsTreeItem::icon() const
{
    if (_page.isSome()) {
        return _page->pageIcon();
    }
    return QIcon();
}

bmcl::OptionPtr<const SettingsTreeItem> SettingsTreeItem::findFirstPage() const
{
    if (_page.isSome()) {
        return this;
    }
    for (const auto& child : _children) {
        auto rv = child->findFirstPage();
        if (rv.isSome()) {
            return rv;
        }
    }
    return bmcl::None;
}

SettingsTreeModel::SettingsTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    QString rootName("_root_");

    _rootItem.reset(new SettingsTreeItem(&rootName));
}

SettingsTreeModel::~SettingsTreeModel() {}

bool SettingsTreeModel::addChildPage(mccui::SettingsPage* page)
{
    QString path = page->pagePath();
    return _rootItem->addChildPage(&path, page);
}

QModelIndex SettingsTreeModel::firstSelectableIndex() const
{
    bmcl::OptionPtr<const SettingsTreeItem> item = _rootItem->findFirstPage();
    if (item.isNone()) {
        return QModelIndex();
    }
    return createIndex(item->row(), 0, const_cast<SettingsTreeItem*>(item.unwrap()));
}

QVariant SettingsTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    SettingsTreeItem* item = static_cast<SettingsTreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        return item->name();
    } else if (role == Qt::DecorationRole) {
        return item->icon();
    }
    return QVariant();
}

bmcl::OptionPtr<mccui::SettingsPage> SettingsTreeModel::pageFromIndex(const QModelIndex& index)
{
    if (!index.isValid()) {
        return bmcl::None;
    }

    return static_cast<SettingsTreeItem*>(index.internalPointer())->page();
}

Qt::ItemFlags SettingsTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    SettingsTreeItem* item = static_cast<SettingsTreeItem*>(index.internalPointer());
    auto flags =  QAbstractItemModel::flags(index);
    if (item->page().isNone()) {
        flags &= ~Qt::ItemIsEnabled;
    }
    return flags;
}

QModelIndex SettingsTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    SettingsTreeItem* parentItem;

    if (!parent.isValid()) {
        parentItem = _rootItem.get();
    } else {
        parentItem = static_cast<SettingsTreeItem*>(parent.internalPointer());
    }

    bmcl::OptionPtr<SettingsTreeItem> childItem = parentItem->childItem(row);
    if (childItem.isSome()) {
        return createIndex(row, column, childItem.unwrap());
    } else {
        return QModelIndex();
    }
}

QModelIndex SettingsTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    SettingsTreeItem* childItem = static_cast<SettingsTreeItem*>(index.internalPointer());
    bmcl::OptionPtr<SettingsTreeItem> parentItem = childItem->parentItem();

    if (parentItem.unwrap() == _rootItem.get()) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem.unwrap());
}

int SettingsTreeModel::rowCount(const QModelIndex& parent) const
{
    SettingsTreeItem* parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = _rootItem.get();
    } else {
        parentItem = static_cast<SettingsTreeItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int SettingsTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}


}
