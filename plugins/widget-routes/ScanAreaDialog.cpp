#include "ScanAreaDialog.h"
#include "ui_ScanAreaDialog.h"
#include "mcc/uav/WaypointTemplateType.h"

ScanAreaDialog::ScanAreaDialog(QWidget* parent)
    : mccui::Dialog(parent)
{
    _ui = new Ui::ScanAreaDialog;
    _ui->setupUi(this);
}

ScanAreaDialog::~ScanAreaDialog()
{
    delete _ui;
}


double ScanAreaDialog::delta() const
{
    return _ui->deltaBox->value();
}

double ScanAreaDialog::speed() const
{
    return _ui->speedBox->value();
}

double ScanAreaDialog::height() const
{
    return _ui->heightBox->value();
}

void ScanAreaDialog::setType(mccuav::WaypointTempalteType type)
{
    _type = type;
    if (type == mccuav::WaypointTempalteType::Scan)
    {
        _ui->deltaBox->setVisible(true);
        _ui->label_7->setVisible(true);
        _ui->scanType->setText("Маршрут \"Змейка\"");
    }
    else
    {
        _ui->deltaBox->setVisible(false);
        _ui->label_7->setVisible(false);
        _ui->scanType->setText("Маршрут \"Прямоугольник\"");
    }

    adjustSize();
}
