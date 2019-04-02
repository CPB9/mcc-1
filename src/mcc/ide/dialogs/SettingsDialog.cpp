#include "mcc/ide/dialogs/SettingsDialog.h"

#include "mcc/ide/models/SettingsTreeModel.h"
#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/Settings.h"
#include "mcc/res/Resource.h"

#include <bmcl/OptionPtr.h>

#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QDebug>
#include <QStringRef>
#include <QHeaderView>
#include <QLabel>

#include <functional>
#include <memory>

namespace mccide {

SettingsDialog::SettingsDialog(mccui::Settings* settings, QWidget* parent /*= 0*/)
    : mccui::Dialog(parent)
    , _settings(settings)
{
    setWindowTitle("Настройки");
    setWindowIcon(mccres::loadIcon(mccres::ResourceKind::SettingsIcon));

    resize(1000, 600);
    _pageContainer = new QWidget(this);
    _containerLayout = new QVBoxLayout;
    _pageLabel = new QLabel("");
    QFont labelFont = _pageLabel->font();
    labelFont.setPointSizeF(labelFont.pointSizeF() * 1.5);
    _pageLabel->setFont(labelFont);

    _containerLayout->addWidget(_pageLabel);
    _pageContainer->setLayout(_containerLayout);

    _settingsModel = new SettingsTreeModel(this);
    _settingsTreeView = new QTreeView;
    _settingsTreeView->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    _settingsTreeView->setModel(_settingsModel);
    _settingsTreeView->setHeaderHidden(true);
    _settingsTreeView->setSelectionBehavior(QTreeView::SelectRows);
    _settingsTreeView->setSelectionMode(QTreeView::SingleSelection);
    _settingsTreeView->setAlternatingRowColors(true);

    _buttonBox = new QDialogButtonBox(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    _buttonBox->button(QDialogButtonBox::Cancel)->setIcon(mccres::loadIcon(mccres::ResourceKind::CancelButtonIcon));
    _buttonBox->button(QDialogButtonBox::Ok)->setIcon(mccres::loadIcon(mccres::ResourceKind::OkButtonIcon));

    auto hlayout = new QHBoxLayout;
    hlayout->addWidget(_settingsTreeView);
    hlayout->addWidget(_pageContainer);

    auto vlayout = new QVBoxLayout;
    vlayout->addLayout(hlayout);
    vlayout->addWidget(_buttonBox);

    setLayout(vlayout);

    setModal(true);

    connect(this, &SettingsDialog::accepted, this, &SettingsDialog::apply);
    connect(_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

    connect(_buttonBox, &QDialogButtonBox::clicked, this,
        [this](QAbstractButton* button)
        {
            QAbstractButton* cancelButton = _buttonBox->button(QDialogButtonBox::Cancel);
            QAbstractButton* restoreDefaultsButton = _buttonBox->button(QDialogButtonBox::RestoreDefaults);
            if (button == restoreDefaultsButton) {
                restoreDefaults();
            } else if (button == cancelButton) {
                restoreOld();
            }
        }
    );

    connect(_settingsTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& prev) {
         Q_UNUSED(prev)
         bmcl::OptionPtr<mccui::SettingsPage> page = _settingsModel->pageFromIndex(current);
         if (page.isNone()) {
             return;
         }
         if (_currentSettingsWidget.isSome()) {
             _currentSettingsWidget->hide();
             _containerLayout->replaceWidget(_currentSettingsWidget.unwrap(), page.unwrap());
         } else {
             _containerLayout->addWidget(page.unwrap());
         }
         _currentSettingsWidget = page;
         _pageLabel->setText(page->pageTitle());
         page->show();
    });
}

SettingsDialog::~SettingsDialog() {}

bool SettingsDialog::addPage(mccui::SettingsPage* page)
{
    page->hide();
    page->setParent(this);
    bool isAdded = _settingsModel->addChildPage(page);
    //_settingsTreeView->setMinimumWidth(_settingsTreeView->header()->sectionSize(0));
    if (isAdded) {
        _settingsTreeView->expandAll();
        _settingsTreeView->resizeColumnToContents(0);
        _pages.push_back(page);
    }
    return isAdded;
}

void SettingsDialog::foreachPage(void (mccui::SettingsPage::*functor)())
{
    for (const auto& page : _pages) {
        std::bind(functor, page)();
    }
}

void SettingsDialog::apply()
{
    foreachPage(&mccui::SettingsPage::apply);
}

void SettingsDialog::load()
{
    foreachPage(&mccui::SettingsPage::load);
}

void SettingsDialog::saveOld()
{
    foreachPage(&mccui::SettingsPage::saveOld);
}

void SettingsDialog::restoreOld()
{
    foreachPage(&mccui::SettingsPage::restoreOld);
}

void SettingsDialog::showEvent(QShowEvent* event)
{
    mccui::Dialog::showEvent(event);

    saveOld();
    _settingsTreeView->selectionModel()->setCurrentIndex(_settingsModel->firstSelectableIndex(), QItemSelectionModel::ClearAndSelect);
}

void SettingsDialog::restoreDefaults()
{
    _settings->restoreDefaults();
    load();
    apply();
}
}
