#include "mcc/map/CacheStackView.h"
#include "mcc/map/CacheStackModel.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/TableEditWidget.h"

#include <QFileDialog>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QAction>

namespace mccmap {

CacheStackView::CacheStackView(CacheStackModel* model, mccui::Settings* settings, QWidget* parent)
    : mccui::SettingsPage(parent)
    , _model(model)
    , _settings(settings)
{
    setEnabled(model->isEnabled());

    _tableEdit = new mccui::TableEditWidget;
    _tableEdit->setModel(model);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(_tableEdit);

    setLayout(mainLayout);

    QAction* omcfAction = new QAction("Добавить карту OMCF", this);
    _tableEdit->addAddAction(omcfAction);

    connect(omcfAction, &QAction::triggered, this, [this]() {
        QString stackPathKey = "map/lastStackPath";
        QString last = _settings->read(stackPathKey).toString();
        QString path = QFileDialog::getOpenFileName(this, "Добавить кеш", last, "Кеш карт (*.omcf)");
        if (path.isEmpty()) {
            return;
        }
        _settings->tryWrite(stackPathKey, path);
        _model->addOmcfCache(path);
    });

    connect(_tableEdit, &mccui::TableEditWidget::indexSelected, this, [this](const QModelIndex& index) {
        if (!index.isValid()) {
            _tableEdit->setRemoveButtonEnabled(false);
            return;
        }
        _tableEdit->setRemoveButtonEnabled(_model->canRemoveAt(index.row()));
    });

    connect(_model, &CacheStackModel::enabled, this, &CacheStackView::setEnabled);

    _lastState = _model->saveOld();
}

CacheStackView::~CacheStackView()
{
}

bool CacheStackView::event(QEvent* event)
{
    if (!isEnabled() && (event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        if (mevent->button() == Qt::LeftButton) {
            event->accept();
            emit clicked();
            return true;
        }
    }
    return QWidget::event(event);
}

void CacheStackView::load()
{
}

void CacheStackView::apply()
{
    _model->apply();
}

void CacheStackView::saveOld()
{
    _lastState = _model->saveOld();
}

void CacheStackView::restoreOld()
{
    _model->restoreOld(_lastState);
}

QString CacheStackView::pagePath() const
{
    return "Карты/Стек карт";
}

QString CacheStackView::pageTitle() const
{
    return "Стек карт";
}

QIcon CacheStackView::pageIcon() const
{
    return QIcon(":/resources/stack_icon.png");
}
}
