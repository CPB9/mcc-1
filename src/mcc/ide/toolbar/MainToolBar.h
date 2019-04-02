#pragma once

#include "mcc/Config.h"
#include "mcc/ui/Fwd.h"

#include <QWidget>

#include <map>

class QHBoxLayout;
class QMenu;
class QFrame;

namespace mccide {

class MCC_IDE_DECLSPEC MainToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit MainToolBar(QWidget *parent = nullptr);
    ~MainToolBar() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    static QSize  blockMinimumSize() {return QSize(48, 48);}
    static QColor mainBackgroundColor() {return QColor(64, 64, 64, 255);}
    static QColor hoveredBackgroundColor() {return QColor(32, 32, 32, 255);}

    QMenu* mainMenu() const {return _mainMenu;}

public slots:
    void addUserWidget(QWidget* widget, bool left = true);
    void addStretch(int stretch = 1);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QFrame* addLine();

    QHBoxLayout*                _mainLayout;
    QMenu*                      _mainMenu;
    mccui::ClickableLabel*      _menuLabel;

    std::map<QObject*, QFrame*> _lines;

    Q_DISABLE_COPY(MainToolBar)
};
}
