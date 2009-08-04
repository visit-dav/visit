/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
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
