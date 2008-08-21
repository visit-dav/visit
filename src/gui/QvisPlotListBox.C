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

#include <qmessagebox.h>

#include <QvisPlotListBox.h>
#include <QvisPlotListBoxItem.h>
#include <PlotList.h>
#include <Plot.h>
#include <QvisPlotListBoxItem.h>
#include <GUIBase.h>
#include <QvisPlotManagerWidget.h>


// ****************************************************************************
// Method: QvisPlotListBox::QvisPlotListBox
//
// Purpose: 
//   This is the constructor for the QvisPlotListBox class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//   f      : window flags.
//
// Notes:
//   This is a QListBox widget with a new version of the doubleClicked
//   signal that can also send the point that was doubled-clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 09:57:03 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 16:07:17 PST 2003
//   I connected this widget's clicked signal to its new itemClicked slot.
//
//   Ellen Tarwater, Mon, June25, 2007
//   Added in the context menu set-up.
//
// ****************************************************************************

QvisPlotListBox::QvisPlotListBox(QWidget *parent, const char *name, WFlags f) :
    QListBox(parent, name, f)
{
    contextMenuCreateActions();
}

// ****************************************************************************
// Method: QvisPlotListBox::~QvisPlotListBox
//
// Purpose: 
//   This is the destructor for the QvisPlotListBox class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 09:58:53 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPlotListBox::~QvisPlotListBox()
{
    // nothing
}

// ****************************************************************************
// Method: QvisPlotListBox::viewportMousePressEvent
//
// Purpose: 
//   This event handler is called when the user releases the mouse in the
//   viewport area of the list box. We use the position of the mouse click
//   to determine whether any special areas of the listbox item were clicked.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 9 12:52:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBox::viewportMousePressEvent(QMouseEvent *e)
{
    QPoint p(viewportToContents(e->pos()));
    clickHandler(p, e->button() == Qt::RightButton, false);
    QListBox::viewportMousePressEvent(e);
}

// ****************************************************************************
// Method: QvisPlotListBox::viewportMouseDoubleClickEvent
//
// Purpose: 
//   This is an internal method of QListBox that was re-implemented
//   for this class.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 09:59:58 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBox::viewportMouseDoubleClickEvent(QMouseEvent *e)
{
    QPoint p(viewportToContents(e->pos()));
    clickHandler(p, e->button() == Qt::RightButton, true);
    QListBox::viewportMouseDoubleClickEvent(e);
}

// ****************************************************************************
// Method: QvisPlotListBox::clickHandler
//
// Purpose: 
//   Handles clicks for us.
//
// Arguments:
//   clickLocation : The location of the click.
//   doubleClicked : Whether or not we're handling a double click.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 16:16:32 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Nov 20 14:34:44 PST 2007
//   Fixed plot expansion.
//
//   Sean Ahern, Thu Aug 21 14:32:54 EDT 2008
//   Fixed deleting operators on non-selected plots.
//
// ****************************************************************************

void
QvisPlotListBox::clickHandler(const QPoint &clickLocation, bool rightClick,
    bool doubleClicked)
{
    QPoint itemClickLocation(clickLocation);
    int y = 0;
    int heightSum = 0;
    int action = -1, opId = -1;
    bool bs = signalsBlocked();
    bool emitted = true;

    // Walk through all of the items, checking if we've clicked in each one.
    for (size_t i = 0; i < count(); ++i)
    {
        QListBoxItem *current = item(i);
        QvisPlotListBoxItem *item2 = (QvisPlotListBoxItem *)current;
        int h = current->height(this);

        if (clickLocation.y() >= y && clickLocation.y() < (y + h))
        {
            // We've clicked in the item.

            // If the item is not selected, select it.
            blockSignals(false);
            setSelected(current, true);
            blockSignals(bs);

            // Reduce the y location of the click location to be local to the
            // item.
            itemClickLocation.setY(clickLocation.y() - heightSum);

            // Handle the click.
            if (action == -1)
            {
                action = item2->clicked(itemClickLocation, doubleClicked, opId);
            }
        }
        else
        {
            blockSignals(false);
            setSelected(current, false);
            blockSignals(bs);
        }
        heightSum += h;
        y += h;
    }

    switch(action)
    {
    case 0: // expand clicked
        triggerUpdate(true);
        break;
    case 1: // subset clicked
         emit activateSubsetWindow();
        break;
    case 2: // plot clicked
         emit activatePlotWindow(opId);
        break;
    case 3: // operator clicked
         emit activateOperatorWindow(opId);
        break;
    case 4: // promote clicked
         emit promoteOperator(opId);
        break;
    case 5: // demote clicked
        emit demoteOperator(opId);
        break;
    case 6: // delete clicked
         emit removeOperator(opId);
        break;
    default:
        if (rightClick)
        {
            emit selectionChanged();
        }
        else
            emitted = false;
        break;
    }
}

// ****************************************************************************
// Method: QvisPlotListBox::isExpanded
//
// Purpose: 
//   Returns whether or not the id'th item is expanded.
//
// Arguments:
//   id : The index of the item that we want to check.
//
// Returns:    True if the item is expanded; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 13:35:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPlotListBox::isExpanded(int id) const
{
   bool retval = false;

   if(id >= 0 && id < count())
   {
       QvisPlotListBoxItem *obj = (QvisPlotListBoxItem *)item(id);
       retval = obj ? obj->isExpanded() : false;
   }

   return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::activeOperatorIndex
//
// Purpose: 
//   Returns the id'th item's active operator.
//
// Arguments:
//   id : The index of the item that we want to check.
//
// Returns:    The active operator for the id'th item.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 13:35:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
QvisPlotListBox::activeOperatorIndex(int id) const
{
   int retval = -1;

   if(id >= 0 && id < count())
   {
       QvisPlotListBoxItem *obj = (QvisPlotListBoxItem *)item(id);
       retval = obj ? obj->activeOperatorIndex() : -1;
   }

   return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::NeedsToBeRegenerated
//
// Purpose: 
//   Determines whether or not the plot listbox needs to be updated.
//
// Arguments:
//   pl : The plot list object that we observe.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 16 13:18:27 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Dec 5 16:20:07 PST 2003
//   I changed the method so having a different Active flag is not cause
//   to regenerate the items. I also renamed the method.
//
//   Brad Whitlock, Wed Jul 28 17:39:36 PST 2004
//   I made it check the file prefixes.
//
// ****************************************************************************

bool
QvisPlotListBox::NeedsToBeRegenerated(const PlotList *pl,
    const stringVector &prefixes) const
{
    bool retval = true;

    if(pl->GetNumPlots() == count() && prefixes.size() == count())
    {
        for(int i = 0; i < pl->GetNumPlots(); ++i)
        {
            QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
            const Plot &newPlot = pl->operator[](i);
            const Plot &currentPlot = lbi->GetPlot();

            // See if the prefixes are different.
            if(prefixes[i] != std::string(lbi->GetPrefix().latin1()))
                 return true;

            // See if the plots are different
            bool nu = newPlot.GetStateType() != currentPlot.GetStateType() ||
                   newPlot.GetPlotType() != currentPlot.GetPlotType() ||
                   newPlot.GetHiddenFlag() != currentPlot.GetHiddenFlag() ||
                   newPlot.GetExpandedFlag() != currentPlot.GetExpandedFlag() ||
                   newPlot.GetActiveOperator() != currentPlot.GetActiveOperator() ||
                   newPlot.GetPlotVar() != currentPlot.GetPlotVar() ||
                   newPlot.GetDatabaseName() != currentPlot.GetDatabaseName() ||
                   newPlot.GetOperators() != currentPlot.GetOperators();

            if(nu) return true;
        }
        return false;
    }

    return retval;
}

// ****************************************************************************
// Method: QvisPlotListBox::NeedToUpdateSelection
//
// Purpose: 
//   Determines if the selection needs to be updated.
//
// Arguments:
//   pl : The new plot list to compare to the one that we have.
//
// Returns:    True if the selection needs to be updated; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 5 16:20:58 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Feb 23 14:17:32 PST 2007
//   Changed code so it checks all plots and updates any active flags that
//   are different in the plot objects. This ensures that the list box will
//   stay consistent between updates that cause its items to be regenerated.
//
// ****************************************************************************

bool
QvisPlotListBox::NeedToUpdateSelection(const PlotList *pl) const
{
    bool retval = true;

    if(pl->GetNumPlots() == count())
    {
        bool anyDiffer = false;
        for(int i = 0; i < pl->GetNumPlots(); ++i)
        {
            QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
            const Plot &newPlot = pl->operator[](i);
            Plot &currentPlot = lbi->GetPlot();

            if(newPlot.GetActiveFlag() != currentPlot.GetActiveFlag())
            {
                currentPlot.SetActiveFlag(newPlot.GetActiveFlag());
                anyDiffer = true;
            }
        }
        return anyDiffer;
    }

    return retval;
}


// ****************************************************************************
// Method: QvisPlotListBox::contextMenuCreateActions
//
// Purpose: 
//   Creates the actions required for the context menu & then builds the menu
//
// Arguments:
//
// Programmer: Ellen Tarwater
// Creation:   Mon June 11 2007
//
// Modifications:
//   Kathleen Bonnell, Wed Nov 28 09:36:22 PST 2007
//   Fix compile errror on Windows -- make maxNwins const.
//
//   Ellen Tarwater, Thurs, Dec 27, 2007
//   Commenting out CopyToWin option... incomplete.
//   
// ****************************************************************************
void
QvisPlotListBox::contextMenuCreateActions()
{
// this should be updated so that nothing is enabled unless a plot is selected!

     //  - need to get the max # of wins for copyToWin...
     // const int maxNwins = 10;
     // int selectWindow[maxNwins];
     
     hideShowAct = new QAction(tr("&Hide/Show"), 0, this);
     hideShowAct->setStatusTip(tr("Hide or Show this plot"));
     hideShowAct->setToggleAction(true);
     connect( hideShowAct, SIGNAL(toggled(bool)), this, SIGNAL(hideThisPlot()));
     
     deleteAct = new QAction(tr("Delete"), 0, this);
     deleteAct->setStatusTip(tr("Delete this plot"));
     connect( deleteAct, SIGNAL(activated()), this, SIGNAL(deleteThisPlot()));
          
     drawAct = new QAction(tr("Draw"), 0, this);
     drawAct->setStatusTip(tr("Draw this plot"));
     connect( drawAct, SIGNAL(activated()), this, SIGNAL(drawThisPlot()));    

     clearAct = new QAction(tr("Clear"), 0, this);
     clearAct->setStatusTip(tr("Clear this plot"));
     connect( clearAct, SIGNAL(activated()), this, SIGNAL(clearThisPlot()));    

     redrawAct = new QAction(tr("Redraw"), 0, this);
     redrawAct->setStatusTip(tr("Redraw this plot"));
     connect( redrawAct, SIGNAL(activated()), this, SIGNAL(redrawThisPlot()));  
     
     copyAct = new QAction(tr("Copy"), 0, this);
     copyAct->setStatusTip(tr("Copy this plot"));
     connect( copyAct, SIGNAL(activated()), this, SIGNAL(copyThisPlot()));  
       
     // build the menu
     plotContextMenu = new QPopupMenu(this);
     hideItem = hideShowAct->addTo( plotContextMenu );
     deleteItem = deleteAct->addTo( plotContextMenu );
     drawAct->addTo( plotContextMenu );
     plotContextMenu->insertSeparator();
     clearAct->addTo( plotContextMenu );
     redrawAct->addTo( plotContextMenu );  
     plotContextMenu->insertSeparator();
     copyAct->addTo( plotContextMenu );
    
    
// copy to window incomplete!!!!! Commented out below...
     copyToWinAct = new QAction(tr("Copy To Window"), 0, this);
     copyToWinAct->setStatusTip(tr("Copy this plot to different window"));
     connect( copyToWinAct, SIGNAL(activated()), this, SIGNAL(copyToWinThisPlot()));    
     win1Act = new QAction(tr("Window 1"), 0, this);
     win2Act = new QAction(tr("Window 2"), 0, this);
     win1Act->setStatusTip(tr("Copy this plot to different window"));
     win2Act->setStatusTip(tr("Copy this plot to different window"));
     connect( win1Act, SIGNAL(activated()), this, SIGNAL(copyToWinThisPlot()));    
     connect( win2Act, SIGNAL(activated()), this, SIGNAL(copyToWinThisPlot()));    

    // adding a popup menu to the copyToWin option:
    // how many windows are open? hard-coding to 3 while debugging...
//    int nWindows = 3;    
//    for (int i = 0; i < nWindows; ++i)
//    {
//        selectWindow[i] = -1;
//    }
    
//    copyWinSubMenu = new QPopupMenu(this);
//    win1Act->addTo(copyWinSubMenu);
//    win2Act->addTo(copyWinSubMenu);
//    plotContextMenu->insertItem(tr("Copy To Window"), copyWinSubMenu);

     plotContextMenu->insertSeparator();
    
     // Now, for the "Disconnect From TimeSlider" option:
     disconnectAct = new QAction(tr("Disconnect From TimeSlider"), 0, this);
     disconnectAct->setStatusTip(tr("Disconnect this plot from time slider"));
     disconnectAct->setToggleAction(true);
     connect( disconnectAct, SIGNAL(toggled(bool)), this, SIGNAL(disconnectThisPlot()));
     disconnectAct->addTo( plotContextMenu );
    
    
}

// ****************************************************************************
// Method: QvisPlotListBox::contextMenuEvent
//
// Purpose: 
//   This is an internal method of QWidget that was re-implemented
//   for this class - to pop up context menu...
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Ellen Tarwater
// Creation:   Mon June 11 2007
//
// Modifications:
//    Ellen Tarwater December 28, 2007
//    disabling menu items if no plots active
//   
// ****************************************************************************

void
QvisPlotListBox::contextMenuEvent(QContextMenuEvent *e)
{
    
    // setEnabled(false) if no active plots (plots are 'active' without being
    // highlighted/selected, non-intuitive...??)
    
    bool anyActive = false;
    for(size_t i = 0; i < count(); ++i)
    {
        QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
        Plot &currentPlot = lbi->GetPlot();

        if(currentPlot.GetActiveFlag())
        {
            anyActive = true;
        }
    }
    if( !anyActive ) 
    {
        hideShowAct->setEnabled(false);
        deleteAct->setEnabled(false);
        drawAct->setEnabled(false);
        clearAct->setEnabled(false);
        copyAct->setEnabled(false);
        redrawAct->setEnabled(false);
        disconnectAct->setEnabled(false);
    }
    else
    {
        hideShowAct->setEnabled(true);
        deleteAct->setEnabled(true);
        drawAct->setEnabled(true);
        clearAct->setEnabled(true);
        copyAct->setEnabled(true);
        redrawAct->setEnabled(true);
        disconnectAct->setEnabled(true);
    }
    
    plotContextMenu->exec( e->globalPos() );
    
    QListBox::viewportContextMenuEvent(e);
}


