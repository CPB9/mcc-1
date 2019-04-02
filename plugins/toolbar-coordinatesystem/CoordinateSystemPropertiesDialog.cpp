#include "CoordinateSystemPropertiesDialog.h"

#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/CoordinateSystem.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ide/toolbar/MainToolBar.h"

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

CoordinateSystemPropertiesDialog::CoordinateSystemPropertiesDialog(mccui::CoordinateSystemController* csController,
                                                                   mccuav::GlobalActions* actions,
                                                                   QWidget* parent)
    : mccui::Dialog(parent)
    , _csController(csController)
    , _actions(actions)
    , _systemList(new QListWidget)
    , _formatList(new QListWidget)
    , _converterButton(new QPushButton("Открыть конвертер координат"))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    setStyleSheet(QString(
        "QWidget\n"
        "{\n"
        "	color: #909090;\n" // fafafa
        "	background-color: #%1;\n"
        "}\n\n"
    ).arg(mccide::MainToolBar::mainBackgroundColor().rgb(), 6, 16, QLatin1Char('0')));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* listLayout = new QHBoxLayout;

    _systemList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _formatList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    for (std::size_t i = 0; i < csController->systems().size(); i++) {
        const auto& desc = csController->systems()[i];
        _systemList->addItem(desc.fullName());
    }
    listLayout->addWidget(_systemList);

    _formatList->addItem("Г");
    _formatList->addItem("ГМ");
    _formatList->addItem("ГМС");
    listLayout->addWidget(_formatList);

    int systemW = _systemList->sizeHintForColumn(0);
    int systemH = _systemList->sizeHintForRow(0) * _systemList->count();

    int formatW = _formatList->sizeHintForColumn(0);
    int formatH = _formatList->sizeHintForRow(0) * _formatList->count();

    _systemList->setFixedSize(systemW + 2 + _systemList->frameWidth(),
                              std::max(systemH, formatH) + 2 + _systemList->frameWidth());
    _formatList->setFixedSize(formatW + 2 + _formatList->frameWidth(),
                              std::max(systemH, formatH) + 2 + _formatList->frameWidth());
    _systemList->updateGeometry();
    _formatList->updateGeometry();

    mainLayout->addLayout(listLayout);
    mainLayout->addWidget(_converterButton);

    connect(_systemList, &QListWidget::currentRowChanged, this,
            [this](int row)
    {
        if(row != _csController->currentSystemIndex()) {
            _csController->selectSystem(row);
        }
    });

    connect(_formatList, &QListWidget::currentRowChanged, this,
            [this](int row)
    {
        mccui::AngularFormat format = static_cast<mccui::AngularFormat>(row);
        if(format != _csController->angularFormat()) {
            _csController->setAngularFormat(format);
        }
    });
    connect(_converterButton, &QPushButton::clicked, this,
            [this]()
    {
        _actions->showCoordinateConverterDialog();
    });

    adjustSize();
}

CoordinateSystemPropertiesDialog::~CoordinateSystemPropertiesDialog()
{}

void CoordinateSystemPropertiesDialog::updateSystemAndFormat()
{
    _systemList->setCurrentRow(static_cast<int>(_csController->currentSystemIndex()));
    _formatList->setCurrentRow(static_cast<int>(_csController->angularFormat()));

    _formatList->setHidden(!_csController->format().isAngular());
}
