#include "mcc/uav/Fwd.h"
#include "mcc/Config.h"

#include <bmcl/OptionPtr.h>

#include <QAbstractItemModel>
#include <QIcon>

namespace mccui {
class SettingsPage;
}

namespace mccide {

class SettingsTreeItem {
public:
    SettingsTreeItem(const QStringRef& name,
                     bmcl::OptionPtr<mccui::SettingsPage> page = bmcl::None,
                     bmcl::OptionPtr<SettingsTreeItem> parent = bmcl::None);

    bool addChildPage(const QStringRef& path, mccui::SettingsPage* page);

    bmcl::OptionPtr<SettingsTreeItem> childItem(int row);
    bmcl::OptionPtr<SettingsTreeItem> parentItem() { return _parent; }
    bmcl::OptionPtr<mccui::SettingsPage> page() { return _page; }

    void setPage(mccui::SettingsPage* page) { _page = page; }

    int row() const;
    int childCount() const;
    const QString& name() const { return _name; }

    QIcon icon() const;
    bmcl::OptionPtr<const SettingsTreeItem> findFirstPage() const;

private:
    QString                                         _name;
    bmcl::OptionPtr<mccui::SettingsPage>            _page;
    bmcl::OptionPtr<SettingsTreeItem>               _parent;

    std::vector<std::unique_ptr<SettingsTreeItem>>  _children;
};

class MCC_IDE_DECLSPEC SettingsTreeModel : public QAbstractItemModel
{
public:
    explicit SettingsTreeModel(QObject* parent = nullptr);
    ~SettingsTreeModel() override;

    bool addChildPage(mccui::SettingsPage* page);
    QModelIndex firstSelectableIndex() const;

    QVariant data(const QModelIndex& index, int role) const override;
    bmcl::OptionPtr<mccui::SettingsPage> pageFromIndex(const QModelIndex& index);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    //QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    std::unique_ptr<SettingsTreeItem>               _rootItem;

    Q_DISABLE_COPY(SettingsTreeModel)
};
}
