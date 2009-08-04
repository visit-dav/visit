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

#include <QvisSubsetWindow.h>
#include <QvisSubsetPanelWidget.h>
#include <QvisSubsetPanelItem.h>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QMenu>
#include <QHeaderView>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>

#include <ViewerProxy.h>
#include <avtSILNamespace.h>
#include <avtSILRestrictionTraverser.h>

// ****************************************************************************
// Method: QvisSubsetWindow::QvisSubsetWindow
//
// Purpose: 
//   This is the constructor for the QvisSubsetWindow class.
//
// Arguments:
//   subj : The subject that the window will observe.
//   winCaption : The caption displayed in the window's title.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:07:54 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:01:26 PST 2001
//   Made window postable.
//
//   Brad Whitlock, Fri Feb 8 14:54:47 PST 2002
//   Added initialization of pointers that we'll delete later.
//
//   Brad Whitlock, Fri Aug 6 13:56:40 PST 2004
//   Changed to support multiple set highlighting.
//
//   Brad Whitlock, Wed Apr  9 11:05:11 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisSubsetWindow::QvisSubsetWindow(Subject *subj, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false)
{
    // Set these to uninitialized.
    sil_TopSet = -1;
    sil_NumSets = -1;
    sil_NumCollections = -1;
}

// ****************************************************************************
// Method: QvisSubsetWindow::~QvisSubsetWindow
//
// Purpose: 
//   This is the destructor for the QvisSubsetWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:08:43 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 8 14:54:36 PST 2002
//   Added deletion of parentless widgets.
//
//   Brad Whitlock, Fri Aug 6 13:56:19 PST 2004
//   Removed the buttongroups.
//
// ****************************************************************************

QvisSubsetWindow::~QvisSubsetWindow()
{
}

// ****************************************************************************
// Method: QvisSubsetWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the subset window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:09:03 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:05:09 PST 2001
//   Removed the apply and dismiss buttons since they are now created by the
//   new base class.
//
//   Brad Whitlock, Thu Dec 20 15:49:52 PST 2001
//   Modified to account for a change in the listview storage.
//
//   Brad Whitlock, Fri Feb 8 15:52:11 PST 2002
//   Modified the code to use a splitter.
//
//   Brad Whitlock, Fri Aug 6 13:58:12 PST 2004
//   I removed some buttongroups.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Fri Jul 18 09:03:03 PDT 2008
//   Refactored for Qt4.
//
// ****************************************************************************

void
QvisSubsetWindow::CreateWindowContents()
{
    scrollView = new QScrollArea(central);
    scrollView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollView->setWidgetResizable(true);
    topLayout->addWidget(scrollView);
    
    
    // Add a few list views.
    panelSplitter = new QSplitter(central);
    scrollView->setWidget(panelSplitter);
    
    AddPanel();
    AddPanel();
    AddPanel();
    
    panels[0]->SetTitle("Whole");

}

// ****************************************************************************
// Method: QvisSubsetWindow::UpdateWindow
//
// Purpose: 
//   This method updates the widgets in the window with new state information
//   from the viewer proxy's SIL restriction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:09:32 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 20 15:50:32 PST 2001
//   Modified to support new widget layout.
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Use a reference counted SIL restriction instead of a reference to one.
//
//   Brad Whitlock, Wed Apr 23 14:07:51 PST 2003
//   I added code to set the enabled state of the buttons.
//
//   Cyrus Harrison, Fri Jul 18 09:03:03 PDT 2008
//   Refactored for Qt4.
//
// ****************************************************************************

void
QvisSubsetWindow::UpdateWindow(bool)
{
    // Get a reference to the SIL restriction in the viewer proxy.
    avtSILRestriction_p restriction = GetViewerProxy()->GetPlotSILRestriction();

    // If we're looking at what is probably a new SIL restriction, update
    // the contents of the listviews.
    if(restriction->GetNumSets() != sil_NumSets ||
       restriction->GetNumCollections() != sil_NumCollections ||
       restriction->GetTopSet() != sil_TopSet)
    {
        sil_TopSet = restriction->GetTopSet();
        sil_NumSets = restriction->GetNumSets();
        sil_NumCollections = restriction->GetNumCollections();

        bool validTopSet = sil_TopSet > -1;
        
        if(validTopSet)
            panels[0]->ViewSet(sil_TopSet);
        else
            panels[0]->Clear();
        panels[0]->SetTitle(tr("Whole"));
        
        // Clear all but the first panel.
        ClearPanelsToTheRight(1);

    }
    else
    {
        // The restriction's values matched the values that the window saved
        // about the restriction the last time through this routine. Assume
        // that the restrictions correspond to the same SIL. Since we make
        // this assumption, it means that the checked values of the window
        // only need to be updated.
        UpdatePanels();
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::AddPanel
//
// Purpose: 
//   Creates a new subset panel widget and adds it to the subset window.
//
// Arguments:
//   visible : Whether or not the listview is initially visible.
//
// Returns:    The index of the new listview in the list.
//
// Programmer: Cyrus Harrison
// Creation:   Wed Jul 16 14:36:45 PDT 2008
//
// Modifications:
//
// ****************************************************************************
int
QvisSubsetWindow::AddPanel(bool visible)
{
    QvisSubsetPanelWidget *panel = new QvisSubsetPanelWidget(panelSplitter,
                                                             GetViewerProxy());
    connect(panel,SIGNAL(itemSelected(int,bool)),
            this,SLOT(onPanelItemSelected(int,bool)));
    connect(panel,SIGNAL(stateChanged()),
            this,SLOT(onPanelStateChanged()));    
                
    int npanels = panels.count();
    if( npanels > 0)
    {
        connect(panel,SIGNAL(parentStateChanged(int)),
                panels[npanels-1],SLOT(SetSelectedItemState(int)));
    }
    
    
    
    panelSplitter->addWidget(panel);
    panels.append(panel);
    return panels.count() - 1;
}


// ****************************************************************************
// Method: QvisSubsetWindow::ClearPanelsToTheRight
//
// Purpose: 
//   Clears all panels with index greater than or equal to specified index.
//   Hides any emty panels beyond index 2.
//
// Arguments:
//   index : The start index of the panels we want to clear.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 08:54:37 PDT 2008
//
// Notes: Refactored for Qt4 from Brad Whitlock's ClearListViewsToTheRight().
//
// Modifications:
//
// ****************************************************************************

void
QvisSubsetWindow::ClearPanelsToTheRight(int index)
{
    for(int i=index;i<panels.count();i++)
    {
        panels[i]->Clear();
        if(i > 2)
            panels[i]->hide();
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::UpdatePanels
//
// Purpose: 
//   Updates the checkmarks for all visible panels except the panel with
//   the specified index. Also allows you to update panels after a specific
//   index.
//
// Arguments:
//   index: The index of the panel that we do *not* want to update.
//   panels_after: Option to only update the panels after the given index.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 08:57:49 PDT 2008
//
// Notes: Refactored for Qt4 from Brad Whitlock's UpdateCheckMarks().
//
// Modifications:
//
// ****************************************************************************

void
QvisSubsetWindow::UpdatePanels(int index, bool panels_after)
{
    avtSILRestriction_p restriction = GetViewerProxy()->GetPlotSILRestriction();
    
    int npanels = panels.count();
    
    int start = 0;
    if(panels_after)
        start = index+1;
    
    for(int i = start ; i < npanels; ++i)
    {
        if(i == index || panels[i]->isHidden() )
            continue;

        panels[i]->UpdateView();
    }

}


// ****************************************************************************
// Method: QvisSubsetWindow::Apply
//g
// Purpose: 
//   Sends the SIL restriction to the viewer.
//
// Arguments:
//   ignore : If ignore is true then the SIL restriction is send to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 4 15:15:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Indicate that we do not want the window to update when we apply
        // the values.
        SetUpdate(false);
        GetViewerProxy()->SetPlotSILRestriction();
    }
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSubsetWindow::onPanelItemSelected
//
// Purpose: 
//   This is a Qt slot function that is called when a panel item is selected.
//
// Arguments:
//   id: The SIL id
//
// Programmer: Cyrus Harrison
// Creation:   Wed Jul 16 16:07:22 PDT 2008
//
// Modifications:
//
// ****************************************************************************
void
QvisSubsetWindow::onPanelItemSelected(int id, bool parent)
{
    int index = FindPanelIndex(sender());

    if(index >= 0)
    {
        if(parent)
        {
            ClearPanelsToTheRight(index+1);
            Apply();
            return;
        }
        
        if(index + 1 == panels.count() )
            AddPanel();

        avtSILRestriction_p restriction = GetViewerProxy()->GetPlotSILRestriction();
        panels[index+1]->ViewCollection(id);
        if(panels[index+1]->isHidden())
            panels[index+1]->show();
        ClearPanelsToTheRight(index+2);
    }
        
}

// ****************************************************************************
// Method: QvisSubsetWindow::onPanelItemSelected
//
// Purpose: 
//   This is a Qt slot function that is called when a panel item is selected.
//
// Arguments:
//   id: The SIL id
//
// Programmer: Cyrus Harrison
// Creation:   Wed Jul 16 16:07:22 PDT 2008
//
// Modifications:
//
// ****************************************************************************
void
QvisSubsetWindow::onPanelStateChanged()
{
    int index = FindPanelIndex(sender());

    if(index == -1)
        return;
    
    UpdatePanels(index,true);
        
}

// ****************************************************************************
// Method: QvisSubsetWindow::FindPanelIndex
//
// Purpose: 
//   Finds the index of a panel.
//
// Arguments:
//   obj: The object 
//
// Programmer: Cyrus Harrison
// Creation:   Wed Jul 16 16:07:22 PDT 2008
//
// Modifications:
//
// ****************************************************************************
int
QvisSubsetWindow::FindPanelIndex(QObject *obj)
{
    if(!obj)
        return -1;
        
    int npanels = panels.count();
    
    for(int i=0;i<npanels;i++)
        if(panels[i] == obj)
            return i;

    return -1;
}


// ****************************************************************************
// Method: QvisSubsetWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's "Apply"
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:27:55 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:14:54 PST 2002
//   Moved the body to the Apply method.
//
// ****************************************************************************

void
QvisSubsetWindow::apply()
{
    Apply(true);
}


