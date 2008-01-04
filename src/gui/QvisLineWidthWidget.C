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

#include <stdio.h>

#include <QvisLineWidthWidget.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpixmapcache.h>

// Some static pixmap data.
const char *QvisLineWidthWidget::style1[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style2[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style3[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style4[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style5[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};


const char *QvisLineWidthWidget::style6[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style7[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style8[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style9[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineWidthWidget::style10[] = {
"40 18 2 1",
"  c None",
". c black",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"........................................",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

char *QvisLineWidthWidget::augmentedData[21];
char QvisLineWidthWidget::augmentedForeground[15];

// ****************************************************************************
// Method: QvisLineWidthWidget::QvisLineWidthWidget
//
// Purpose: 
//   Constructor for the QvisLineWidthWidget class.
//
// Arguments:
//   width_ : The default width to use.
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:17:45 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 1 16:37:25 PST 2000
//   Changed code so the pixmap is changed before being used.
//
//   Brad Whitlock, Thu Sep 6 15:42:41 PST 2001
//   Changed the combobox from using a background color to a background mode.
//
//   Kathleen Bonnell, Tue Dec  3 16:14:25 PST 2002 
//   Added more pixmaps for larger line widths. 
//
// ****************************************************************************

QvisLineWidthWidget::QvisLineWidthWidget(int width_, QWidget *parent,
                                         const char *name) : 
                                         QWidget(parent, name)
{
    // Create some pixmaps and store them in the application global
    // pixmap cache.
    QPixmap style1Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth1", style1Pixmap))
    {
        AugmentPixmap(style1);
        QPixmap s1p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth1", s1p);
        style1Pixmap = s1p;
    }
    QPixmap style2Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth2", style2Pixmap))
    {
        AugmentPixmap(style2);
        QPixmap s2p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth2", s2p);
        style2Pixmap = s2p;
    }
    QPixmap style3Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth3", style3Pixmap))
    {
        AugmentPixmap(style3);
        QPixmap s3p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth3", s3p);
        style3Pixmap = s3p;
    }
    QPixmap style4Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth4", style4Pixmap))
    {
        AugmentPixmap(style4);
        QPixmap s4p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth4", s4p);
        style4Pixmap = s4p;
    }
    QPixmap style5Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth5", style5Pixmap))
    {
        AugmentPixmap(style5);
        QPixmap s5p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth5", s5p);
        style5Pixmap = s5p;
    }
    QPixmap style6Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth6", style6Pixmap))
    {
        AugmentPixmap(style6);
        QPixmap s6p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth6", s6p);
        style6Pixmap = s6p;
    }
    QPixmap style7Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth7", style7Pixmap))
    {
        AugmentPixmap(style7);
        QPixmap s7p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth7", s7p);
        style7Pixmap = s7p;
    }
    QPixmap style8Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth8", style8Pixmap))
    {
        AugmentPixmap(style8);
        QPixmap s8p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth8", s8p);
        style8Pixmap = s8p;
    }
    QPixmap style9Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth9", style9Pixmap))
    {
        AugmentPixmap(style9);
        QPixmap s9p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth9", s9p);
        style9Pixmap = s9p;
    }
    QPixmap style10Pixmap;
    if(!QPixmapCache::find("visit_gui_linewidth10", style10Pixmap))
    {
        AugmentPixmap(style10);
        QPixmap s10p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linewidth10", s10p);
        style10Pixmap = s10p;
    }

    // Create the combo box and add the pixmaps to it.
    QHBoxLayout *topLayout = new QHBoxLayout(this);
    lineWidthComboBox = new QComboBox(false, this, "lineWidthComboBox");
    lineWidthComboBox->insertItem(style1Pixmap);
    lineWidthComboBox->insertItem(style2Pixmap);
    lineWidthComboBox->insertItem(style3Pixmap);
    lineWidthComboBox->insertItem(style4Pixmap);
    lineWidthComboBox->insertItem(style5Pixmap);
    lineWidthComboBox->insertItem(style6Pixmap);
    lineWidthComboBox->insertItem(style7Pixmap);
    lineWidthComboBox->insertItem(style8Pixmap);
    lineWidthComboBox->insertItem(style9Pixmap);
    lineWidthComboBox->insertItem(style10Pixmap);
    lineWidthComboBox->setBackgroundMode(PaletteBackground);
    lineWidthComboBox->setCurrentItem(width_);
    topLayout->addWidget(lineWidthComboBox);
    connect(lineWidthComboBox, SIGNAL(activated(int)),
            this, SIGNAL(lineWidthChanged(int)));
}

// ****************************************************************************
// Method: QvisLineWidthWidget::~QvisLineWidthWidget
//
// Purpose: 
//   Destructor for the QvisLineWidthWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:19:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisLineWidthWidget::~QvisLineWidthWidget()
{
    // nothing
}

// ****************************************************************************
// Method: QvisLineWidthWidget::setEnabled
//
// Purpose: 
//   This is a Qt slot function that sets the enabled state of the widget.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:40:53 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineWidthWidget::setEnabled(bool val)
{
    lineWidthComboBox->setEnabled(val);
}

// ****************************************************************************
// Method: QvisLineWidthWidget::SetLineWidth
//
// Purpose: 
//   This method sets the widget's line width. Doing so causes the right
//   pixmap to be displayed.
//
// Arguments:
//   width_ : The new line width to display. Valid values are 0 through 9.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:19:32 PST 2000
//
// Modifications:
//   Eric Brugger, Wed Jun 25 16:41:52 PDT 2003
//   Change the error test to allow line widths of 0 through 9.
//   
// ****************************************************************************

void
QvisLineWidthWidget::SetLineWidth(int width_)
{
    if(width_ < 0 || width_ > 9)
        return;

    lineWidthComboBox->blockSignals(true);
    lineWidthComboBox->setCurrentItem(width_);
    lineWidthComboBox->blockSignals(false);

    // If signals are not blocked, emit the LineWidthChanged signal.
    if(!signalsBlocked())
        emit lineWidthChanged(width_);
}

// ****************************************************************************
// Method: QvisLineWidthWidget::GetLineWidth
//
// Purpose: 
//   This method returns the current line width.
//
// Arguments:
//
// Returns:    This method returns the current line width.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:21:07 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisLineWidthWidget::GetLineWidth() const
{
    return lineWidthComboBox->currentItem();
}

// ****************************************************************************
// Method: QvisLineWidthWidget::AugmentPixmap
//
// Purpose: 
//   This method augments pixmap data so that the application's foreground
//   color is used instead of the default of black.
//
// Arguments:
//   xpm : A pointer to xpm pixmap strings.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 1 16:29:56 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineWidthWidget::AugmentPixmap(const char *xpm[])
{
    for(int i = 0; i < 21; ++i)
        augmentedData[i] = (char *)xpm[i];

    // Turn the third element into the foreground color.
    sprintf(augmentedForeground, ". c #%02x%02x%02x", 
            foregroundColor().red(), foregroundColor().green(),
            foregroundColor().blue());
    augmentedData[2] = augmentedForeground;
}
