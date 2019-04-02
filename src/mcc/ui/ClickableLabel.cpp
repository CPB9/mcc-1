#include "mcc/ui/ClickableLabel.h"

#include <QMouseEvent>

namespace mccui {

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
    , _mainPixmap(nullptr)
    , _hoveredPixmap(nullptr)
    , _isHovered(false)
    , _uuid(bmcl::Uuid::createNil())
{}

ClickableLabel::ClickableLabel(const QString& text, QWidget* parent)
    : ClickableLabel(parent)
{
    setHoverableText(text);
}

ClickableLabel::ClickableLabel(const QPixmap& pixmap, const QPixmap& hoveredPixmap, QWidget* parent)
    : ClickableLabel(parent)
{
    setMainPixmap(pixmap);
    setHoveredPixmap(hoveredPixmap);
}

ClickableLabel::ClickableLabel(const bmcl::Uuid& uuid, const QPixmap& pixmap, const QPixmap& hoveredPixmap, QWidget* parent)
    : ClickableLabel(pixmap, hoveredPixmap, parent)
{
    _uuid = uuid;
}

ClickableLabel::ClickableLabel(const bmcl::Uuid& uuid, QWidget* parent)
    : ClickableLabel(parent)
{
    _uuid = uuid;
}

ClickableLabel::ClickableLabel(const bmcl::Uuid& uuid, const QString& text, QWidget* parent)
    : ClickableLabel(uuid, parent)
{
    setHoverableText(text);
}

ClickableLabel::~ClickableLabel()
{
    removeMainPixmap();
    removeHoveredPixmap();
}

void ClickableLabel::forceHover(bool hovered)
{
    if(hovered)
    {
        if(!_isHovered)
        {
            _isHovered = true;

            if(_hoveredPixmap)
            {
                setPixmap(*_hoveredPixmap);
                update();
            }
            else
            {
                QString txt = text();
                if(!txt.isEmpty())
                {
                    setText("<u>" + txt + "</u>");
                    update();
                }
            }
        }
    }
    else
    {
        if(_isHovered)
        {
            _isHovered = false;

            if(_mainPixmap)
            {
                setPixmap(*_mainPixmap);
                update();
            }
            else
            {
                QString txt = text();
                if(!txt.isEmpty())
                {
                    setText(txt.mid(3, txt.size() - 3 - 4));
                    update();
                }
            }
        }
    }
}

void ClickableLabel::setMainPixmap(const QPixmap& pixmap)
{
    removeMainPixmap();

    if(!pixmap.isNull())
    {
        _mainPixmap = new QPixmap(pixmap);
        setPixmap(pixmap);

        if(!_isHovered)
        {
            setPixmap(*_mainPixmap);
            update();
        }
    }
}

void ClickableLabel::setHoveredPixmap(const QPixmap& pixmap)
{
    removeHoveredPixmap();

    if(!pixmap.isNull())
    {
        _hoveredPixmap = new QPixmap(pixmap);
        setMouseTracking(true);

        if(_isHovered)
        {
            setPixmap(*_hoveredPixmap);
            update();
        }
    }
}

void ClickableLabel::removeMainPixmap()
{
    delete _mainPixmap;
    _mainPixmap = nullptr;

    setPixmap(QPixmap()); // null pixmap
}

void ClickableLabel::removeHoveredPixmap()
{
    delete _hoveredPixmap;
    _hoveredPixmap = nullptr;

    if(!_mainPixmap)
    {
        // if it's setting at hovering
        setPixmap(QPixmap());
    }

    setMouseTracking(false);
}

void ClickableLabel::setHoverableText(const QString& text)
{
    setMouseTracking(true);

    if(_isHovered)
        setText("<u>" + text + "</u>");
    else
        setText(text);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    if(_mainPixmap || !text().isEmpty())
    {
        event->accept();
        emit clicked(event);
        return;
    }

    QLabel::mousePressEvent(event);
}

void ClickableLabel::mouseMoveEvent(QMouseEvent* event)
{
    forceHover(true);

    QLabel::mouseMoveEvent(event);
}

void ClickableLabel::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(_mainPixmap || !text().isEmpty())
    {
        event->accept();
        emit doubleClicked(event);
        return;
    }

    QLabel::mouseDoubleClickEvent(event);
}

void ClickableLabel::leaveEvent(QEvent* event)
{
    forceHover(false);

    QLabel::leaveEvent(event);
}
}
