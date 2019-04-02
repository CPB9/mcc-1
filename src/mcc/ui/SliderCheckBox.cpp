#include "mcc/ui/SliderCheckBox.h"

#include <QMouseEvent>
#include <QPainter>
#include <QVariant>
#include <QDebug>

namespace mccui {

SliderCheckBox::SliderCheckBox(QWidget* parent)
    : SliderCheckBox(false, parent)
{}

SliderCheckBox::SliderCheckBox(bool smallSize, QWidget* parent)
    : SliderCheckBox(bmcl::Uuid::createNil(), smallSize, parent)
{}

SliderCheckBox::SliderCheckBox(bmcl::Uuid uuid, bool smallSize, QWidget* parent)
    : QCheckBox(parent)
    , _uuid(uuid)
    , _smallSize(smallSize)
{
    init();

    if(_smallSize)
        setMinimumSize(32, 14);
    else
        setMinimumSize(42, 18);
}

SliderCheckBox::~SliderCheckBox()
{}

void SliderCheckBox::mousePressEvent(QMouseEvent* e)
{
    if (checkState() == Qt::Unchecked)
        emit sliderStateChanged(_uuid, true);
    else
        emit sliderStateChanged(_uuid, false);

    e->accept();
}

//TODO: выпилить статики

void SliderCheckBox::paintEvent(QPaintEvent *)
{
    resize(sizeHint());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int objWidth = width() - 2;
    int objHeight = height() - 2;

    QPixmap* pixmap(nullptr);

    switch (checkState())
    {
    case Qt::Checked:
    {
        if(_smallSize)
        {
            static QPixmap green_mini(":/slider/green_mini.png");
            pixmap = &green_mini;
        }
        else
        {
            static QPixmap green(":/slider/green.png");
            pixmap = &green;
        }

        break;
    }
    case Qt::PartiallyChecked:
    {
        if(_smallSize)
        {
            static QPixmap gray_mini(":/slider/gray_mini.png");
            pixmap = &gray_mini;
        }
        else
        {
            static QPixmap gray(":/slider/gray.png");
            pixmap = &gray;
        }

        break;
    }
    case Qt::Unchecked:
    {
        if(_smallSize)
        {
            static QPixmap black_mini(":/slider/black_mini.png");
            pixmap = &black_mini;
        }
        else
        {
            static QPixmap black(":/slider/black.png");
            pixmap = &black;
        }

        break;
    }
    }

    if(pixmap != nullptr)
        painter.drawPixmap(1, 1, objWidth, objHeight, *pixmap);
}

QSize SliderCheckBox::sizeHint() const
{
    return minimumSize();
}

void SliderCheckBox::setSmallSize(bool smallSize)
{
    if(smallSize == _smallSize)
        return;

    _smallSize = smallSize;
    update();
}

void SliderCheckBox::init()
{
    setTristate(true);

    setStyleSheet("background-color: rgb(0, 0, 0);\n" \
                  "color: rgb(255, 255, 255);\n");
}

OnOffSliderCheckBox::OnOffSliderCheckBox(QWidget* parent)
    : SliderCheckBox(false, parent)
{}

OnOffSliderCheckBox::OnOffSliderCheckBox(bool smallSize, QWidget* parent)
    : SliderCheckBox(smallSize, parent)
{}

OnOffSliderCheckBox::OnOffSliderCheckBox(bmcl::Uuid uuid, bool smallSize, QWidget* parent)
    : SliderCheckBox(uuid, smallSize, parent)
{}

OnOffSliderCheckBox::~OnOffSliderCheckBox()
{}

void OnOffSliderCheckBox::mousePressEvent(QMouseEvent* e)
{
    if(checkState() == Qt::PartiallyChecked)
    {
        int halfWidth = sizeHint().width() / 2;

        if(e->x() < halfWidth)
        {
            // Off
            emit sliderStateChanged(uuid(), false);
        }
        else
        {
            // On
            emit sliderStateChanged(uuid(), true);
        }

        e->accept();
    }
    else
    {
        SliderCheckBox::mousePressEvent(e);
    }
}
}
