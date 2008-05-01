/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QvisColorManagerWidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qvbox.h>

#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>

// ****************************************************************************
// Method: QvisColorManagerWidget::QvisColorManagerWidget
//
// Purpose: 
//   This is the constructor for the QvisColorManagerWidget class.
//
// Arguments:
//   parent : A pointer to the widget's parent.
//   name   : The name of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:29:46 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 21 17:56:02 PST 2003
//   I set the background pixmap of the viewport if we have a background pixmap.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

QvisColorManagerWidget::QvisColorManagerWidget(QWidget *parent,
    const char *name) : QWidget(parent, name), colorEntries()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    // Create the scrollview.
    scrollView = new QScrollView(this, "scrollView");
    scrollView->setVScrollBarMode(QScrollView::Auto);
    scrollView->setHScrollBarMode(QScrollView::Auto);
    QPixmap *pix = colorGroup().brush(QColorGroup::Background).pixmap();
    if(pix)
        scrollView->viewport()->setBackgroundPixmap(*pix);
    else
        scrollView->viewport()->setBackgroundColor(colorGroup().background());
    topLayout->addWidget(scrollView);

    // Create the QGrid widget that will manage the layout of the buttons, etc.
    grid = new QGrid(3, QGrid::Horizontal, scrollView->viewport(), "grid");
    grid->setSpacing(10);
    scrollView->addChild(grid);

    // Add some labels.
    nameLabel = new QLabel(tr("Material"), grid, "nameLabel");
    nameLabel->setAlignment(AlignHCenter | AlignVCenter);
    colorLabel = new QLabel(tr("Color"), grid, "colorLabel");
    colorLabel->setAlignment(AlignHCenter | AlignVCenter);
    opacityLabel = new QLabel(tr("Opacity"), grid, "opacityLabel");
    opacityLabel->setAlignment(AlignHCenter | AlignVCenter);

#if 0
    // Temporary default values.
    addEntry("Aluminum", QColor(255, 0, 0), 255);
    addEntry("Nickel",   QColor(255, 255, 0), 255);
    addEntry("Uranium",  QColor(0, 255, 0), 255);
    addEntry("Neon",     QColor(0, 255, 255), 255);
    addEntry("Silver",   QColor(0, 0, 255), 255);
#endif
}

// ****************************************************************************
// Method: QvisColorManagerWidget::~QvisColorManagerWidget
//
// Purpose: 
//   This is the destructor for the QvisColorManagerWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:30:37 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisColorManagerWidget::~QvisColorManagerWidget()
{
    ColorEntryVector::iterator pos;

    // Delete all of the ColorEntry structs.
    for(pos = colorEntries.begin(); pos != colorEntries.end(); ++pos)
        delete *pos;
}

// ****************************************************************************
// Method: QvisColorManagerWidget::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 15:19:50 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisColorManagerWidget::sizeHint() const
{
    QSize s;

    int h = colorEntries.size() * 30;
    if(h < 170)
       h = 170;
    if(h > 800)
       h = 800;

    s.setHeight(h);
    return s;
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setEnabled
//
// Purpose: 
//   Sets the enabled state of all of the sub-widgets.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 22 12:13:05 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setEnabled(bool val)
{
    // Call the base class's setEnabled method first.
    QWidget::setEnabled(val);

    // Set the enabled state of the top-level widgets.
    scrollView->setEnabled(val);
    grid->setEnabled(val);
    nameLabel->setEnabled(val);
    colorLabel->setEnabled(val);
    opacityLabel->setEnabled(val);

    // Iterate through the color widgets and set their enabled state.
    for(size_t i = 0; i < colorEntries.size(); ++i)
    {
        colorEntries[i]->nameLabel->setEnabled(val);
        colorEntries[i]->colorBox->setEnabled(val);
        colorEntries[i]->opacitySlider->setEnabled(val);
    }
}

// ****************************************************************************
// Method: QvisColorManagerWidget::addEntry
//
// Purpose: 
//   Adds a new ColorEntry to the list and creates widgets for it.
//
// Arguments:
//   name  : The name of the new item.
//   color : The color of the new item.
//   opacity : The opacity of the new item.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:31:06 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 22 12:22:26 PDT 2002
//   I made it so that new widgets can be disabled by default.
//
//   Brad Whitlock, Wed Apr  9 10:36:22 PDT 2008
//   Clean up string coding.
//
// ****************************************************************************

void
QvisColorManagerWidget::addEntry(const QString &name, const QColor &color,
    int opacity)
{
    QString    temp;
    const void *userData = (const void *)colorEntries.size();
    ColorEntry *entry = new ColorEntry;

    // Create the name label.
    temp.sprintf("entry%d.nameLabel", colorEntries.size());
    QString tempName(" ");
    tempName += name;
    entry->nameLabel = new QLabel(tempName, grid, temp.ascii());
    entry->nameLabel->setEnabled(isEnabled());

    // Create the color box.
    temp.sprintf("entry%d.colorBox", colorEntries.size());
    entry->colorBox = new QvisColorButton(grid, temp.ascii(), userData);
    entry->colorBox->setButtonColor(color);
    entry->colorBox->setEnabled(isEnabled());
    connect(entry->colorBox,
            SIGNAL(selectedColor(const QColor &, const void *)),
            this,
            SLOT(selectedColor(const QColor &, const void *)));

    // Create the opacity slider.
    temp.sprintf("entry%d.opacitySlider", colorEntries.size());
    entry->opacitySlider = new QvisOpacitySlider(0, 255, 25, opacity,
        grid, temp.ascii(), userData);
    entry->opacitySlider->setTickInterval(64);
    entry->opacitySlider->setGradientColor(color);
    entry->opacitySlider->setEnabled(isEnabled());
    connect(entry->opacitySlider, SIGNAL(valueChanged(int, const void *)),
            this, SLOT(changedOpacity(int, const void *)));

    // If the widget is visible, we need to show the widgets that we created.
    if(isVisible())
    {
        entry->nameLabel->show();
        entry->colorBox->show();
        entry->opacitySlider->show();
    }

    // Add the new entry to the list of ColorEntries.
    colorEntries.push_back(entry);
}

// ****************************************************************************
// Method: QvisColorManagerWidget::removeLastEntry
//
// Purpose: 
//   Removes the entry at the end of the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:32:22 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::removeLastEntry()
{
    if(colorEntries.size() > 0)
    {
        ColorEntry *entry = colorEntries.back();

        // Disconnect the entry's widgets.
        disconnect(entry->colorBox,
                   SIGNAL(selectedColor(const QColor &, const void *)),
                   this,
                   SLOT(selectedColor(const QColor &, const void *)));
        disconnect(entry->opacitySlider,
                   SIGNAL(valueChanged(int, const void *)),
                   this,
                   SLOT(changedOpacity(int, const void *)));

        // Delete the ColorEntry.
        delete entry->nameLabel;
        delete entry->colorBox;
        delete entry->opacitySlider;
        delete entry;

        // Remove the last element from the vector.
        colorEntries.pop_back();
    }
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setEntry
//
// Purpose: 
//   Sets the attributes of the entry at the specified index.
//
// Arguments:
//   index   : The index of the entry to change.
//   name    : The entry's new name.
//   color   : The entry's new color.
//   opacity : The entry's new opacity.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:33:08 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setEntry(int index, const QString &name,
    const QColor &color, int opacity)
{
    if(index >= 0 && index < colorEntries.size())
    {
        ColorEntry *entry = colorEntries[index];
        QString tempName(" ");
        tempName += name;
        entry->nameLabel->setText(tempName);

        entry->colorBox->blockSignals(true);
        entry->colorBox->setButtonColor(color);
        entry->colorBox->blockSignals(false);

        entry->opacitySlider->blockSignals(true);
        entry->opacitySlider->setValue(opacity);
        entry->opacitySlider->blockSignals(false);
        entry->opacitySlider->setGradientColor(color);
    }
    else if(index > 0)
    {
        // If the index was out of range, but was positive, append a new
        // entry to the entry list.
        addEntry(name, color, opacity);
    }
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setAttributeName
//
// Purpose: 
//   Sets the attribute name of the entry at the specified index.
//
// Arguments:
//   index : The index of the entry to change.
//   name  : The entry's new name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:34:36 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setAttributeName(int index, const QString &name)
{
    if(index >= 0 && index < colorEntries.size())
    {
        QString tempName(" ");
        tempName += name;
        colorEntries[index]->nameLabel->setText(tempName);
    }
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setColor
//
// Purpose: 
//   Sets the color of the specified entry.
//
// Arguments:
//   index : The index of the entry to change.
//   color : The entry's new color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:35:25 PST 2000
//
// Modifications:
//   Brad Whitlock, Sat Feb 17 13:45:49 PST 2001
//   Added code to set the gradient color of the opacity slider.
//
// ****************************************************************************

void
QvisColorManagerWidget::setColor(int index, const QColor &color)
{
    if(index >= 0 && index < colorEntries.size())
    {
        colorEntries[index]->colorBox->setButtonColor(color);
        colorEntries[index]->opacitySlider->setGradientColor(color);
    }    
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setOpacity
//
// Purpose: 
//   Sets the specified entry's opacity.
//
// Arguments:
//   index   : The index of the entry to change.
//   opacity : The entry's new opacity.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:36:12 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setOpacity(int index, int opacity)
{
    if(index >= 0 && index < colorEntries.size())
    {
        colorEntries[index]->opacitySlider->setValue(opacity);
    }
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setNameLabelText
//
// Purpose: 
//   Sets the text of the name column label widget.
//
// Arguments:
//   s : The new text.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 16 17:49:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setNameLabelText(const QString &s)
{
    nameLabel->setText(s);
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setColorLabelText
//
// Purpose: 
//   Sets the text of the color column label widget.
//
// Arguments:
//   s : The new text.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 16 17:49:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setColorLabelText(const QString &s)
{
    colorLabel->setText(s);
}

// ****************************************************************************
// Method: QvisColorManagerWidget::setOpacityLabelText
//
// Purpose: 
//   Sets the text of the opacity column label widget.
//
// Arguments:
//   s : The new text.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 16 17:49:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::setOpacityLabelText(const QString &s)
{
    opacityLabel->setText(s);
}

// ****************************************************************************
// Method: QvisColorManagerWidget::numEntries
//
// Purpose: 
//   Returns the number of entries.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:37:07 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisColorManagerWidget::numEntries() const
{
    return colorEntries.size();
}

// ****************************************************************************
// Method: QvisColorManagerWidget::name
//
// Purpose: 
//   Returns the name of the specified entry.
//
// Arguments:
//   index : The index of the entry whose name we're returning.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:37:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QString
QvisColorManagerWidget::name(int index) const
{
    QString retval;

    if(index >= 0 && index < colorEntries.size())
    {
        retval = colorEntries[index]->nameLabel->text();
    }

    return retval;
}

// ****************************************************************************
// Method: QvisColorManagerWidget::color
//
// Purpose: 
//   Returns the color of the specified entry.
//
// Arguments:
//   index : The index of the entry whose color we're returning.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:37:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QColor
QvisColorManagerWidget::color(int index) const
{
    QColor retval(0,0,0);

    if(index >= 0 && index < colorEntries.size())
    {
        retval = colorEntries[index]->colorBox->buttonColor();
    }

    return retval;
}

// ****************************************************************************
// Method: QvisColorManagerWidget::opacity
//
// Purpose: 
//   Returns the opacity of the specified entry.
//
// Arguments:
//   index : The index of the entry whose opacity we're returning.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:37:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisColorManagerWidget::opacity(const int index) const
{
    int retval = 255;

    if(index >= 0 && index < colorEntries.size())
    {
        retval = colorEntries[index]->opacitySlider->value();
    }

    return retval;
}

// ****************************************************************************
// Method: QvisColorManagerWidget::paletteChange
//
// Purpose: 
//   This method is called when the palette changes and it is intended to
//   update the scrollview's viewport color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 15:27:53 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Aug 22 14:51:14 PST 2003
//   I changed the method so it works better on MacOS X.
//
// ****************************************************************************

void
QvisColorManagerWidget::paletteChange(const QPalette &)
{
    QPixmap *pix = colorGroup().brush(QColorGroup::Background).pixmap();
    if(pix)
        scrollView->viewport()->setBackgroundPixmap(*pix);
    else
        scrollView->viewport()->setBackgroundColor(colorGroup().background());
}

// ****************************************************************************
// Method: QvisColorManagerWidget::selectedColor
//
// Purpose: 
//   This is a Qt slot function that is called when one of the widget's color
//   buttons changes color. It turns the color button's userData into an index
//   and re-emits the signal.
//
// Arguments:
//   color    : The color button's new color.
//   userData : The color button's userData.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 14:19:27 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::selectedColor(const QColor &color,
    const void *userData)
{
    // The userData pointer contains an integer, get it out.
    int index = (int)((long)userData);

    // Change the gradient color of the opacity slider.
    if(index >= 0 && index < colorEntries.size())
        colorEntries[index]->opacitySlider->setGradientColor(color);

    // emit the colorChanged signal.
    emit colorChanged(color, index);
}

// ****************************************************************************
// Method: QvisColorManagerWidget::changedOpacity
//
// Purpose: 
//   This is a Qt slot function that is called when one of the widget's opacity
//   sliders changes opacity. It turns the opacity slider's userData into an
//   index and re-emits the signal.
//
// Arguments:
//   opacity  : The opacity slider's new color.
//   userData : The opacity slider's userData.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 14:19:27 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorManagerWidget::changedOpacity(int opacity, const void *userData)
{
    // The userData pointer contains an integer, get it out.
    int index = (int)((long)userData);

    // emit the opacityChanged signal.
    emit opacityChanged(opacity, index);
}
