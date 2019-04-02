#include "UavModeWidget.h"

#include "mcc/ui/TextUtils.h"

#include <QFontMetrics>
#include <QLabel>
#include <QVBoxLayout>

constexpr int maxTextSize = 20;
constexpr int margin = 3;

UavModeWidget::UavModeWidget(QWidget* separator, QWidget *parent)
    : AbstractUavWidget(separator, parent)
    , _modeLabel(new QLabel())
    , _submodeLabel(new QLabel())
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, margin, 0);
    layout->setSpacing(3);

    layout->addWidget(_modeLabel);
    layout->addWidget(_submodeLabel);

    _modeLabel->setStyleSheet("background-color: transparent;");
    _submodeLabel->setStyleSheet("background-color: transparent;");

    QFont font = _modeLabel->font();
    font.setBold(true);
    _modeLabel->setFont(font);

    QString testLine(maxTextSize, 'x');
    QFontMetrics fm(font);
    int minW = fm.boundingRect(testLine).width();
    _modeLabel->setFixedWidth(minW);
    _submodeLabel->setFixedWidth(minW);
    setMinimumWidth(minW + margin);
}

UavModeWidget::~UavModeWidget()
{}

void UavModeWidget::setMode(const QString& mode)
{
    if(mode == _modeText)
        return;

    _modeText = mode;

    QString shortName;
    QFontMetrics fm(font());
    for(int i = maxTextSize; i > 1; --i)
    {
        shortName = mccui::shortTextLine(mode, i);
        if(fm.boundingRect(shortName).width() <= _modeLabel->width())
            break;
    }

    _modeLabel->setText(shortName);
    if(_modeText.size() > shortName.size())
        _modeLabel->setToolTip(_modeText);
    else
        _modeLabel->setToolTip(QString());
}

void UavModeWidget::setSubmode(const QString& submode)
{
    if(submode == _submodeText)
        return;

    _submodeText = submode;

    QString shortName;
    QFontMetrics fm(font());
    for(int i = maxTextSize; i > 1; --i)
    {
        shortName = mccui::shortTextLine(submode, i);
        if(fm.boundingRect(shortName).width() <= _submodeLabel->width())
            break;
    }

    _submodeLabel->setText(shortName);
    if(_submodeText.size() > shortName.size())
        _submodeLabel->setToolTip(_submodeText);
    else
        _submodeLabel->setToolTip(QString());
}

bool UavModeWidget::mayToShow() const
{
    return !_modeLabel->text().isEmpty() || !_submodeLabel->text().isEmpty();
}
