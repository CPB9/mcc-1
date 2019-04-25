#pragma once

#include <QDialog>

#include <bmcl/Rc.h>
#include <bmcl/Option.h>

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"
#include "mcc/msg/Objects.h"

class QComboBox;
class QLineEdit;

class OpenQmlFileDialog : public QDialog
{
public:
    OpenQmlFileDialog(const mccui::Rc<mccuav::UavController>& uavController, const QString& startPath, QWidget* parent);

    QString path() const;
    QString directory() const;
    bmcl::OptionPtr<mccuav::Uav> uav() const;

private:
    QComboBox* _uavsCombo;
    QLineEdit* _path;

    mccui::Rc<mccuav::UavController> _uavsController;
};