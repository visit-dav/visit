/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <SpreadsheetTabWidget.h>
#include <qcursor.h>
#include <qtabbar.h>
#include <qstyle.h>

// ****************************************************************************
// Class: HighlightTabBar
//
// Purpose:
//   Subclass of QTabBar that can draw its text label in its parent's 
//   highlight color.
//
// Notes:      The paintLabel guts were taken from QTabBar source in Qt and
//             simplified for this class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:43:28 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class HighlightTabBar : public QTabBar
{
public:
    HighlightTabBar(QWidget *parent, const char *name) : QTabBar(parent, name)
    {
    }

    virtual ~HighlightTabBar()
    {
    }
protected:
    virtual void paintLabel(QPainter* p, const QRect& br,
                            QTab* t, bool has_focus ) const
    {
        QRect r = br;
        bool selected = currentTab() == t->identifier();

        QStyle::SFlags flags = QStyle::Style_Default;

        if (isEnabled() && t->isEnabled())
            flags |= QStyle::Style_Enabled;
        if (has_focus)
            flags |= QStyle::Style_HasFocus;
        if ( selected )
            flags |= QStyle::Style_Selected;

        if(t->rect().contains(mapFromGlobal(QCursor::pos())))
            flags |= QStyle::Style_MouseOver;

        // Change the text color based on whether the tab is selected.
        QColorGroup cg(colorGroup());
        if(selected)
        {
            SpreadsheetTabWidget *tw = (SpreadsheetTabWidget *)parent();
            if(tw != 0)
                cg.setColor(QColorGroup::Foreground, tw->highlightColor());
        }

        style().drawControl( QStyle::CE_TabBarLabel, p, this, r,
             t->isEnabled() ? cg : palette().disabled(),
             flags, QStyleOption(t) );
    }
};

// ****************************************************************************
// Method: SpreadsheetTabWidget::SpreadsheetTabWidget
//
// Purpose:
//   Constructor for the SpreadsheetTabWidget class.
//
// Arguments:
//   parent : The parent widget.
//   name   : The name of the widget.
//
// Note:       We set the tab bar to our own tab bar subclass so it will have
//             the ability to draw the text label in a highlight color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:45:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetTabWidget::SpreadsheetTabWidget(QWidget *parent, const char *name) : 
        QTabWidget(parent, name)
{
    setTabBar(new HighlightTabBar(this, "highlight"));
    highlight = colorGroup().foreground();
}

// ****************************************************************************
// Method: SpreadsheetTabWidget::~SpreadsheetTabWidget
//
// Purpose: 
//   Destructor for the SpreadsheetTabWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:46:24 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetTabWidget::~SpreadsheetTabWidget()
{
}

// ****************************************************************************
// Method: SpreadsheetTabWidget::setHighlightColor
//
// Purpose: 
//   Sets the highlight color for the widget.
//
// Arguments:
//   c : The new highlight color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:46:41 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTabWidget::setHighlightColor(const QColor &c)
{
    if(highlight != c)
    {
        highlight = c;
        tabBar()->update();
    }
}

// ****************************************************************************
// Method: SpreadsheetTabWidget::highlightColor
//
// Purpose: 
//   Returns a reference to the highlight color.
//
// Returns:    The highlight color
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:47:06 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

const QColor &
SpreadsheetTabWidget::highlightColor() const
{
    return highlight;
}
