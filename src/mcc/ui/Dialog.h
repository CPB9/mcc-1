#pragma once
#include "mcc/Config.h"
#include <QDialog>

namespace mccui
{

class MCC_UI_DECLSPEC Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);

private:
    int exec() final {return QDialog::Rejected;}
    void open() final {show();}
};

}
