/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <qwidgetstack.h>
#include <qtabbar.h>
#include <QvisNotepadArea.h>
#include <QvisPostableWindow.h>

#include <qscrollview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qwidget.h>

// ****************************************************************************
// Method: QvisNotepadArea::QvisNotepadArea
//
// Purpose: 
//   Constructor for the QvisNotepadArea class. It hooks the tab bar
//   to the widget stack so when the tab bar is clicked, the corresp-
//   onding tab is raised in the widget stack.
//
// Arguments:
//    parent : The parent of this widget.
//    name   : The name of this widget.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:45:52 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 15:19:10 PST 2001
//   Renamed an initializer.
//
// ****************************************************************************

QvisNotepadArea::QvisNotepadArea(QWidget *parent, const char *name) :
    QVBox(parent, name), postedLookup()
{
    QWidget *central = new QWidget( this, "central" );
    QVBoxLayout *topLayout = new QVBoxLayout(central);

    tabs = new QTabWidget(central, "tabWidget");
    topLayout->addWidget(tabs);

    // Since no window is posted, add an empty tab.
    empty = new QWidget(tabs);
    tabs->addTab(empty, "    ");
    numPosted = 0;
}

// ****************************************************************************
// Method: QvisNotepadArea::~QvisNotepadArea
//
// Purpose: 
//   Destructor for the QvisNotepadArea class.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:47:40 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisNotepadArea::~QvisNotepadArea()
{
}

// ****************************************************************************
// Method: QvisNotepadArea::showPage
//
// Purpose: 
//   Shows the page containing the posted window.
//
// Arguments:
//   pw : A pointer to the posted window.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 15:48:25 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Sep 25 15:20:09 PST 2001
//   I changed the code to reflect that all posted windows have parent widgets
//   that are not their top-level window when they are posted in this notepad.
//
// ****************************************************************************

void
QvisNotepadArea::showPage(QvisPostableWindow *pw)
{
    if(pw == NULL)
        return;

    // If the window pointer is a key in the map, show the page that contains
    // the window's posted parent widget.
    PostedInfoLookup::ConstIterator pos;
    if((pos = postedLookup.find(pw)) != postedLookup.end())
        tabs->showPage(pos.data().parent);
}

// ****************************************************************************
// Method: QvisNotepadArea::postWindow
//
// Purpose: 
//   This method causes the widget to post or unpost a QvisPostableWindow
//   widget to/from itself.
//
// Arguments:
//
// Returns:    
//
// Note:       
//   If the window being posted is too large to fit on the tab, a 
//   scrollview is added as the parent of the window before it is
//   posted to the tab.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 18:47:59 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Dec 11 14:48:33 PST 2000
//   I added code to set the background color of the scrollview's
//   viewport.
//
//   Brad Whitlock, Mon Feb 12 14:05:57 PST 2001
//   I added a little code to make the notepad more likely to use the vertical
//   scroll bar if the window is close to being the height of the notepad.
//
//   Brad Whitlock, Tue Sep 25 15:22:26 PST 2001
//   I changed the code to reflect the fact that all windows now have new
//   parent widgets when they are posted into the notepad.
//
// ****************************************************************************

void
QvisNotepadArea::postWindow(QvisPostableWindow *pw)
{
    if(pw == NULL)
        return;

    if(pw->posted())
    {
        // See if the window is marked as having been put into
        // a scrollview.
        PostedInfoLookup::Iterator pos = postedLookup.find(pw);
        if(pos != postedLookup.end())
        {
            // If the parent is a scrollview, restore the minimum size that
            // the widget had before it is was posted.
            if(pos.data().parentIsScrollView)
            {
                int w = pos.data().minWidth;
                int h = pos.data().minHeight;
                pw->GetCentralWidget()->setMinimumSize(w, h);
            }

            // Reparent the window's central widget back to the window
            // before we delete its posted parent. That keeps the window's
            // central widget from getting deleted. 
            pw->GetCentralWidget()->reparent(pw, 0, QPoint(0,0), true);

            // Delete the dead parent widget and remove its tab from the
            // notepad. Make explicit casts since the Qt destructors being
            // did not appear to be virtual.
            tabs->removePage(pos.data().parent);
            if(pos.data().parentIsScrollView)
                delete ((QScrollView *)pos.data().parent);
            else
                delete ((QVBox *)pos.data().parent);
            postedLookup.remove(pos);
        }

        // If all the tabs were removed, add the empty tab.
        if(--numPosted == 0)
        {
            empty = new QWidget( tabs );
            tabs->addTab(empty, "    ");
        }
    }
    else
    {
        // Remove and destroy the empty tab if there are no
        // windows posted.
        if(numPosted == 0)
        {
            tabs->removePage(empty);
            delete empty;
            empty = 0;
        }

        // See if we need to post the window into a scrollview.
        bool need_hscroll = (pw->minimumSize().width() >= tabs->width());
        bool need_vscroll = ((pw->minimumSize().height() + 20) >= tabs->height());

        // Sanity check. This can happen when windows are saved so that they
        // are supposed to post on startup. If this case happens, just use a
        // scrollview.
        if(pw->minimumSize().width() == 0 && pw->minimumSize().height() == 0)
        {
            need_hscroll = false;
            need_vscroll = true;
        }

        if(need_hscroll || need_vscroll)
        {
            // The widget is too big to fit without a scrolled window.
            // Create a scrollview.
            QScrollView *scroll = new QScrollView(tabs);
            scroll->setHScrollBarMode(QScrollView::Auto);
            scroll->setVScrollBarMode(QScrollView::Auto);
            scroll->viewport()->setBackgroundMode(PaletteBackground);

            // Record that the widget was stuck into a scrollview.
            PostedInfo info;
            info.parent = scroll;
            info.parentIsScrollView = true;
            info.minWidth = pw->GetCentralWidget()->minimumWidth();
            info.minHeight = pw->GetCentralWidget()->minimumHeight();
            postedLookup.insert(pw, info);

            // Add the window's central widget to the scrollview.
            scroll->addChild(pw->GetCentralWidget());

            // Add the scrollview to the tab. Show the page so the extents
            // of the scrollview's viewport get set and we have a valid size
            // to use for stretching the widget.
            tabs->addTab(scroll, pw->GetShortCaption());
            tabs->showPage(scroll);

            // Stretch the widget in the appropriate direction.
            if(!need_hscroll)
                pw->GetCentralWidget()->setMinimumWidth(scroll->visibleWidth());
            else if(!need_vscroll)
                pw->GetCentralWidget()->setMinimumHeight(scroll->visibleHeight());
        }
        else
        {
            // Create a VBox to put into the window. Reparent the window's
            // central widget to the VBox so it gets better geometry
            // management. This ensures that it looks correct if it is
            // posted before it is ever shown.
            QVBox *intermediate = new QVBox(tabs, "intermediate");
            pw->GetCentralWidget()->reparent(intermediate, 0, QPoint(0,0), false);

            // Record that the widget was stuck into a scrollview.
            PostedInfo info;
            info.parent = intermediate;
            info.parentIsScrollView = false;
            info.minWidth = 0;
            info.minHeight = 0;
            postedLookup.insert(pw, info);

            // No scrollview was needed, post the window.
            tabs->addTab(intermediate, pw->GetShortCaption());
            tabs->showPage(intermediate);
        }

        // Update the count of posted windows.
        ++numPosted;
    }
}

