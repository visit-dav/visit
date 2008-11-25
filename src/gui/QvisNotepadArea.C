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

#include <QStackedWidget>
#include <QTabBar>
#include <QvisNotepadArea.h>
#include <QvisPostableWindow.h>

#include <QScrollArea>
#include <QPushButton>
#include <QLayout>
#include <QTabWidget>
#include <QWidget>

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
//   Brad Whitlock, Fri Jun  6 09:36:46 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisNotepadArea::QvisNotepadArea(QWidget *parent) : QWidget(parent), 
    postedLookup()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    tabs = new QTabWidget(this);
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
//   Brad Whitlock, Fri Jun  6 09:48:10 PDT 2008
//   Qt 4.
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
        tabs->setCurrentIndex(tabs->indexOf(pos.value().parent));
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
//   Brad Whitlock, Fri Jun  6 09:40:11 PDT 2008
//   Qt 4.
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
            if(pos.value().parentIsScrollView)
            {
                int w = pos.value().minWidth;
                int h = pos.value().minHeight;
                pw->GetCentralWidget()->setMinimumSize(w, h);
            }

            // Reparent the window's central widget back to the window
            // before we delete its posted parent. That keeps the window's
            // central widget from getting deleted.
            if(pos.value().parentIsScrollView)
                 ((QScrollArea *)pos.value().parent)->takeWidget();
            else if(pos.value().parentLayout != 0)
                 pos.value().parentLayout->removeWidget(pw->GetCentralWidget());
            pw->GetCentralWidget()->setParent(pw);
            pw->GetCentralWidget()->show();

            // Delete the dead parent widget and remove its tab from the
            // notepad. Make explicit casts since the Qt destructors being
            // did not appear to be virtual.
            tabs->removeTab(tabs->indexOf(pos.value().parent));
            if(pos.value().parentIsScrollView)
                delete ((QScrollArea *)pos.value().parent);
            else
                delete pos.value().parent;
            postedLookup.erase(pos);
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
            tabs->removeTab(tabs->indexOf(empty));
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
            QScrollArea *scroll = new QScrollArea(tabs);
            scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

            // Record that the widget was stuck into a scrollview.
            PostedInfo info;
            info.parent = scroll;
            info.parentLayout = 0;
            info.parentIsScrollView = true;
            info.minWidth = pw->GetCentralWidget()->minimumWidth();
            info.minHeight = pw->GetCentralWidget()->minimumHeight();
            postedLookup.insert(pw, info);

            // Add the window's central widget to the scrollview.
            scroll->setWidget(pw->GetCentralWidget());

            // Add the scrollview to the tab. Show the page so the extents
            // of the scrollview's viewport get set and we have a valid size
            // to use for stretching the widget.
            tabs->addTab(scroll, pw->GetShortCaption());
            tabs->setCurrentIndex(tabs->indexOf(scroll));
        }
        else
        {
            // Create a QWidget to put into the window. Reparent the window's
            // central widget to the QWidget so it gets better geometry
            // management. This ensures that it looks correct if it is
            // posted before it is ever shown.
            QWidget *intermediate = new QWidget(tabs);
            QVBoxLayout *intermediateLayout = new QVBoxLayout(intermediate);
            intermediateLayout->setMargin(0);
            pw->GetCentralWidget()->setParent(intermediate);
            intermediateLayout->addWidget(pw->GetCentralWidget());

            // Record that the widget was stuck into a scrollview.
            PostedInfo info;
            info.parent = intermediate;
            info.parentLayout = intermediateLayout;
            info.parentIsScrollView = false;
            info.minWidth = 0;
            info.minHeight = 0;
            postedLookup.insert(pw, info);

            // No scrollview was needed, post the window.
            tabs->addTab(intermediate, pw->GetShortCaption());
            tabs->setCurrentIndex(tabs->indexOf(intermediate));
        }

        // Update the count of posted windows.
        ++numPosted;
    }
}

