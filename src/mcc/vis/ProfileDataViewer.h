#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"

#include <bmcl/OptionPtr.h>

#include <QWidget>

class QTableView;

namespace mccvis {

class ProfDataModel;
class Profile;

class MCC_VIS_DECLSPEC ProfileDataViewer : public QWidget {
    Q_OBJECT
public:
    ProfileDataViewer();
    ~ProfileDataViewer();

    void setProfile(bmcl::OptionPtr<const Profile> profile);

private:
    QTableView* _profView;
    ProfDataModel* _profModel;
    Rc<const Profile> _profile;
};

}
