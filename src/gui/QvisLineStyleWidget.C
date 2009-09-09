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

#include <stdio.h>

#include <QvisLineStyleWidget.h>
#include <QComboBox>
#include <QLayout>
#include <QPixmap>
#include <QPixmapCache>

// Some static pixmap data.
const char *QvisLineStyleWidget::style4[] = {
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
" ..  ....  ..  ....  ..  ....  ..  .... ",
" ..  ....  ..  ....  ..  ....  ..  .... ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineStyleWidget::style3[] = {
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
" ..  ..  ..  ..  ..  ..  ..  ..  ..  .. ",
" ..  ..  ..  ..  ..  ..  ..  ..  ..  .. ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineStyleWidget::style2[] = {
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
"...  ....  ....  ....  ....  ....  .... ",
"...  ....  ....  ....  ....  ....  .... ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};

const char *QvisLineStyleWidget::style1[] = {
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

char *QvisLineStyleWidget::augmentedData[23];
char QvisLineStyleWidget::augmentedForeground[15];

// ****************************************************************************
// Method: QvisLineStyleWidget::QvisLineStyleWidget
//
// Purpose: 
//   Constructor for the QvisLineStyleWidget class.
//
// Arguments:
//   style  : The default style to use.
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
//   Brad Whitlock, Tue Jun  3 10:39:54 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Wed Sep  9 16:56:24 PDT 2009
//   I fixed a labelling problem.
//
// ****************************************************************************

QvisLineStyleWidget::QvisLineStyleWidget(int style, QWidget *parent) : 
    QWidget(parent)
{
    // Create some pixmaps and store them in the application global
    // pixmap cache.
    QPixmap style1Pixmap;
    if(!QPixmapCache::find("visit_gui_linestyle1", style1Pixmap))
    {
        AugmentPixmap(style1);
        QPixmap s1p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linestyle1", s1p);
        style1Pixmap = s1p;
    }
    QPixmap style2Pixmap;
    if(!QPixmapCache::find("visit_gui_linestyle2", style2Pixmap))
    {
        AugmentPixmap(style2);
        QPixmap s2p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linestyle2", s2p);
        style2Pixmap = s2p;
    }
    QPixmap style3Pixmap;
    if(!QPixmapCache::find("visit_gui_linestyle3", style3Pixmap))
    {
        AugmentPixmap(style3);
        QPixmap s3p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linestyle3", s3p);
        style3Pixmap = s3p;
    }
    QPixmap style4Pixmap;
    if(!QPixmapCache::find("visit_gui_linestyle4", style4Pixmap))
    {
        AugmentPixmap(style4);
        QPixmap s4p((const char **)augmentedData);
        QPixmapCache::insert("visit_gui_linestyle4", s4p);
        style4Pixmap = s4p;
    }

    // Create the combo box and add the pixmaps to it.
    QHBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setMargin(0);
    lineStyleComboBox = new QComboBox(this);
    lineStyleComboBox->addItem(QIcon(style1Pixmap), tr("solid"));
    lineStyleComboBox->addItem(QIcon(style2Pixmap), tr("dash"));
    lineStyleComboBox->addItem(QIcon(style3Pixmap), tr("dotted"));
    lineStyleComboBox->addItem(QIcon(style4Pixmap), tr("dash/dot"));
    lineStyleComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    lineStyleComboBox->setCurrentIndex(style);
    topLayout->addWidget(lineStyleComboBox);
    connect(lineStyleComboBox, SIGNAL(activated(int)),
            this, SIGNAL(lineStyleChanged(int)));
}

// ****************************************************************************
// Method: QvisLineStyleWidget::~QvisLineStyleWidget
//
// Purpose: 
//   Destructor for the QvisLineStyleWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:19:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisLineStyleWidget::~QvisLineStyleWidget()
{
    // nothing
}

// ****************************************************************************
// Method: QvisLineStyleWidget::setEnabled
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
QvisLineStyleWidget::setEnabled(bool val)
{
    lineStyleComboBox->setEnabled(val);
}

// ****************************************************************************
// Method: QvisLineStyleWidget::SetLineWidth
//
// Purpose: 
//   This method sets the widget's line style. Doing so causes the right
//   pixmap to be displayed.
//
// Arguments:
//   style : The new line style to display. Valid values are 0,1,2,3.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:19:32 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisLineStyleWidget::SetLineStyle(int style)
{
    if(style < 0 || style > 3)
        return;

    lineStyleComboBox->blockSignals(true);
    lineStyleComboBox->setCurrentIndex(style);
    lineStyleComboBox->blockSignals(false);

    // If signals are not blocked, emit the LineStyleChanged signal.
    if(!signalsBlocked())
        emit lineStyleChanged(style);
}

// ****************************************************************************
// Method: QvisLineStyleWidget::GetLineWidth
//
// Purpose: 
//   This method returns the current line style.
//
// Arguments:
//
// Returns:    This method returns the current line style.
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
QvisLineStyleWidget::GetLineStyle() const
{
    return lineStyleComboBox->currentIndex();
}

// ****************************************************************************
// Method: QvisLineStyleWidget::AugmentPixmap
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
//   Brad Whitlock, Tue Jun  3 10:45:50 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisLineStyleWidget::AugmentPixmap(const char *xpm[])
{
    for(int i = 0; i < 23; ++i)
        augmentedData[i] = (char *)xpm[i];

    QColor foreground(palette().color(QPalette::Text));

    // Turn the third element into the foreground color.
    sprintf(augmentedForeground, ". c #%02x%02x%02x", 
            foreground.red(), foreground.green(),
            foreground.blue());
    augmentedData[2] = augmentedForeground;
}
