#include "mcc/vis/ProfileDataViewer.h"
#include "mcc/vis/Profile.h"

#include <QTableView>
#include <QAbstractTableModel>
#include <QHBoxLayout>

#include <bmcl/Math.h>

#include <cmath>

namespace mccvis {

class ProfDataModel : public QAbstractTableModel {
public:
    void setProfile(bmcl::OptionPtr<const Profile> profile)
    {
        beginResetModel();
        _prof.reset(profile.data());
        endResetModel();
    }

    int rowCount(const QModelIndex & parent) const override
    {
        (void)parent;
        if (_prof.isNull()) {
            return 0;
        }
        return _prof->data().size();
    }

    int columnCount(const QModelIndex & parent) const override
    {
        (void)parent;
        return 5;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }
        if (orientation == Qt::Vertical) {
            return section + 1;
        }
        switch (section) {
        case 0:
            return "D(км)";
        case 1:
            return "H(м)";
        case 2:
            return "Hц(м)";
        case 3:
            return "dH(м)";
        }
        return QVariant();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (_prof.isNull()) {
            return QVariant();
        }

        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        int i = index.row();
        if (i >= _prof->data().size()) {
            return QVariant();
        }

        const Profile::Data& d = _prof->data()[i];
        switch (index.column()) {
        case 0:
            return d.profile.x() / 1000;
        case 1:
            return d.profile.y();
        case 2:
            return d.target.y();
        case 3:
            return d.dy;
        }
        return QVariant();
    }

private:
    Rc<const Profile> _prof;
};

ProfileDataViewer::ProfileDataViewer()
    : _profile(nullptr)
{
    _profModel = new ProfDataModel;

    _profView = new QTableView;
    _profView->setModel(_profModel);
    _profView->setSelectionBehavior(QTableView::SelectRows);
    _profView->setSelectionMode(QTableView::SingleSelection);
    _profView->setAlternatingRowColors(true);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(_profView);
    setLayout(mainLayout);
}

ProfileDataViewer::~ProfileDataViewer()
{
}


void ProfileDataViewer::setProfile(bmcl::OptionPtr<const Profile> profile)
{
    _profile.reset(profile.data());
    _profModel->setProfile(profile.data());
}

}
