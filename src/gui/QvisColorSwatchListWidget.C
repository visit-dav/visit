// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisColorSwatchListWidget.h>
#include <QItemDelegate>
#include <QPainter>

// ****************************************************************************
// Class: TextColorSwatchDelegate
//
// Purpose:
//   This class helps the list widget draw a square of color to the left 
//   of the name.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 12:14:06 PDT 2008
//
// Modifications:
//
// ****************************************************************************

class TextColorSwatchDelegate : public QItemDelegate
{
public:
    TextColorSwatchDelegate(QObject *parent) : QItemDelegate(parent), currentColor()
    {
    }

    virtual ~TextColorSwatchDelegate()
    {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, 
        const QModelIndex &index) const
    {
        QColor *c = const_cast<QColor *>(&currentColor);
        *c = QColor(index.data(Qt::UserRole).toString());
        QItemDelegate::paint(painter, option, index);
    }
protected:
    virtual void drawDisplay(QPainter *painter, 
         const QStyleOptionViewItem &option, 
         const QRect &rect, const QString &text) const
    {
        const int hPadding = 4;
        const int vPadding = 2;
        QRect colorRect(rect.x() + hPadding, rect.y()+vPadding, 20, rect.height()-2*vPadding);
        QRect newTextRect(rect);
        newTextRect.setX(rect.x() + colorRect.width() + 2*hPadding);

        QBrush brush(currentColor);
        painter->fillRect(colorRect, brush);

        QItemDelegate::drawDisplay(painter, option, newTextRect, text);
    }

protected:
    QColor currentColor;
};

// ****************************************************************************
// Method: QvisColorSwatchListWidget::QvisColorSwatchListWidget
//
// Purpose: 
//   Constructor for the QvisColorSwatchListWidget class.
//
// Arguments:
//   parent : The widget's parent.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:28:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisColorSwatchListWidget::QvisColorSwatchListWidget(QWidget *parent) : 
   QListWidget(parent)
{
    setItemDelegate(new TextColorSwatchDelegate(this));
    setSelectionMode(ExtendedSelection);
}

// ****************************************************************************
// Method: QvisColorSwatchListWidget::~QvisColorSwatchListWidget
//
// Purpose: 
//   Destructor for the QvisColorSwatchListWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:32:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisColorSwatchListWidget::~QvisColorSwatchListWidget()
{
}

// ****************************************************************************
// Method: QvisColorSwatchListWidget::addItem
//
// Purpose: 
//   Adds a new item that has text and some color.
//
// Arguments:
//   name  : The text to display for the new item.
//   color : The color swatch to display for the new item.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:32:44 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSwatchListWidget::addItem(const QString &name, const QColor &color)
{
    QListWidgetItem *item = new QListWidgetItem(this);
    item->setText(name);
    item->setData(Qt::UserRole, QVariant(color));
}

// ****************************************************************************
// Method: QvisColorSwatchListWidget::setText
//
// Purpose: 
//   Sets the text for the i'th item.
//
// Arguments:
//   i : The item index.
//   s : The new text value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:33:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSwatchListWidget::setText(int i, const QString &s)
{
    if(item(i) != 0)
        item(i)->setText(s);
}

// ****************************************************************************
// Method: QvisColorSwatchListWidget::text
//
// Purpose: 
//   Returns the text for the i'th item.
//
// Arguments:
//   i : The item index.
//
// Returns:    The item's text.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:34:10 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
QvisColorSwatchListWidget::text(int i) const
{
    QString retval;
    if(item(i) != 0)
        retval = item(i)->text();
    return retval;
}

// ****************************************************************************
// Method: QvisColorSwatchListWidget::color
//
// Purpose: 
//   Get the color for the i'th item.
//
// Arguments:
//  i : The item index.
//
// Returns:    The color.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:34:47 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QColor
QvisColorSwatchListWidget::color(int i) const
{
    QColor retval;
    if(item(i) != 0)
        retval = QColor(item(i)->data(Qt::UserRole).toString());
    return retval;
}

// ****************************************************************************
// Method: QvisColorSwatchListWidget::setColor
//
// Purpose: 
//   Sets the i'th item's color
//
// Arguments:
//   i : The item index.
//   c : The new item color.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:41:49 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorSwatchListWidget::setColor(int i, const QColor &c)
{
    if(item(i) != 0)
        item(i)->setData(Qt::UserRole, QVariant(c));
}
