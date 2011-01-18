/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <QComboBox>
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

#include <SILRestrictionAttributes.h>
#include <Plot.h>
#include <PlotList.h>
#include <SelectionList.h>
#include <SelectionProperties.h>

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

QvisSubsetWindow::QvisSubsetWindow(const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
        QvisPostableWindowSimpleObserver::ApplyButton, false)
{
    silrAtts = 0;
    selectionList = 0;
    plotList = 0;

    // Set these to uninitialized.
    sil_TopSet = -1;
    sil_NumSets = -1;
    sil_NumCollections = -1;
    sil_dirty = true;

    sel_selectionName = QString();
    sel_dirty = true;
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
//   Brad Whitlock, Tue Aug 10 13:42:24 PDT 2010
//   Added code to detach subjects.
//
// ****************************************************************************

QvisSubsetWindow::~QvisSubsetWindow()
{
    if(silrAtts != 0)
        silrAtts->Detach(this);
    if(selectionList != 0)
        selectionList->Detach(this);
    if(plotList != 0)
        plotList->Detach(this);
}

void
QvisSubsetWindow::ConnectSILRestrictionAttributes(SILRestrictionAttributes *s)
{
    silrAtts = s;
    silrAtts->Attach(this);
}

void
QvisSubsetWindow::ConnectSelectionList(SelectionList *s)
{
    selectionList = s;
    selectionList->Attach(this);
}

void
QvisSubsetWindow::ConnectPlotList(PlotList *s)
{
    plotList = s;
    plotList->Attach(this);
}

void
QvisSubsetWindow::SubjectRemoved(Subject *s)
{
    if(silrAtts == s)
        silrAtts = 0;

    if(selectionList == s)
        selectionList = 0;

    if(plotList == s)
        plotList = 0;
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
    //
    // Subset controls
    //
    scrollView = new QScrollArea(central);
    scrollView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollView->setWidgetResizable(true);
    topLayout->addWidget(scrollView);

    panelSplitter = new QSplitter(central);
    scrollView->setWidget(panelSplitter);
    AddPanel();
    AddPanel();
    AddPanel();
    
    panels[0]->SetTitle("Whole");

    //
    // Selection controls
    //
    QWidget *selParent = new QWidget(central);
    topLayout->addWidget(selParent);
    QHBoxLayout *sLayout = new QHBoxLayout(selParent);
    sLayout->setMargin(0);
    selectionLabel = new QLabel(tr("Applied selection"), selParent);
    selections = new QComboBox(selParent);
    connect(selections, SIGNAL(activated(const QString &)),
            this, SLOT(selectionChanged(const QString &)));
    sLayout->addWidget(selectionLabel);
    sLayout->addWidget(selections, 10);
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
//   Brad Whitlock, Tue Aug 10 11:40:31 PDT 2010
//   I rewrote the code and moved some to UpdateSILControls.
//
// ****************************************************************************

void
QvisSubsetWindow::UpdateWindow(bool doAll)
{
    if(silrAtts == 0 || selectionList == 0 || plotList == 0)
        return;

    if(SelectedSubject() == silrAtts || doAll)
        UpdateSILControls();

    if(SelectedSubject() == selectionList ||
       SelectedSubject() == plotList || doAll)
        UpdateSelectionControls();
}

// ****************************************************************************
// Method: QvisSubsetWindow::UpdateSelectionControls
//
// Purpose: 
//   Update the selection controls.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 10 13:40:57 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetWindow::UpdateSelectionControls()
{
    // Get the name of the first selected plot
    int selIndex = plotList->FirstSelectedIndex();

    // Let's populate the available selections based on the selection list.
    selections->blockSignals(true);
    selections->clear();
    selections->addItem(tr("None"));
    sel_selectionName = QString();
    int selectedSelection = 0, index = 1;
    for(int i = 0; i < selectionList->GetNumSelections(); ++i)
    {
        const SelectionProperties &sel = selectionList->GetSelections(i);
        bool addOkay = true;
        if(selIndex >= 0)
        {
            const Plot &plot = plotList->GetPlots(selIndex);
            // Get the active plot's selection name.
            if(plot.GetSelection() == sel.GetName())
            {
                sel_selectionName = QString(plot.GetSelection().c_str());
                selectedSelection = index;
            }

            // We can add the selection name if it is not created by the active plot.
            addOkay = (plot.GetPlotName() != sel.GetOriginatingPlot());
        }
            
        if(addOkay)
        {
            selections->addItem(selectionList->GetSelections(i).GetName().c_str());
            index++;
        }
    }
    selections->setCurrentIndex(selectedSelection);
    selections->blockSignals(false);

    bool seEnabled = selections->count() > 1 && plotList->GetNumPlots() > 0;
    selectionLabel->setEnabled(seEnabled);
    selections->setEnabled(seEnabled);

    // Indicate that the user has not changed the selection controls.
    sel_dirty = false;
}

// ****************************************************************************
// Method: QvisSubsetWindow::UpdateSILControls
//
// Purpose: 
//   Update the SIL part of the window.
//
// Arguments:
//
// Returns:    
//
// Note:       I moved this from UpdateWindow.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 10 11:40:10 PDT 2010
//
// Modifications:
//   Brad Whitlock, Tue Aug 10 13:58:01 PDT 2010
//   I set the sil_dirty flag.
//
// ****************************************************************************

void
QvisSubsetWindow::UpdateSILControls()
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

    // Indicate that the user has not changed the SIL restriction controls.
    sil_dirty = false;
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
//   Brad Whitlock, Tue Aug 10 14:03:30 PDT 2010
//   I added code to set the named selection.
//
// ****************************************************************************

void
QvisSubsetWindow::Apply(bool ignore)
{
    if(sil_dirty && (AutoUpdate() || ignore))
    {
        // Indicate that we do not want the window to update when we apply
        // the values.
        SetUpdate(false);
        GetViewerProxy()->SetPlotSILRestriction();
    }

    if(sel_dirty && (AutoUpdate() || ignore))
    {
        // Indicate that we do not want the window to update when we apply
        // the values.
        GetViewerMethods()->ApplyNamedSelection(sel_selectionName.toStdString());
    }
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
//   Brad Whitlock, Tue Aug 10 14:00:03 PDT 2010
//   I added sil_dirty.
//
// ****************************************************************************

void
QvisSubsetWindow::onPanelItemSelected(int id, bool parent)
{
    int index = FindPanelIndex(sender());

    if(index >= 0)
    {
        // The user has changed the SIL controls.
        sil_dirty = true;

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
//   Brad Whitlock, Tue Aug 10 13:59:46 PDT 2010
//   I added sil_dirty.
//
//   Brad Whitlock, Tue Oct 12 11:48:48 PDT 2010
//   Call Apply() since we're likely getting here because we used one of the
//   en masse change buttons in the panel.
//
// ****************************************************************************

void
QvisSubsetWindow::onPanelStateChanged()
{
    int index = FindPanelIndex(sender());

    if(index == -1)
        return;

    // The user has changed the SIL controls.
    sil_dirty = true;
    UpdatePanels(index,true);

    Apply();
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

// ****************************************************************************
// Method: QvisSubsetWindow::selectionChanged
//
// Purpose: 
//   This is a Qt slot function that gets called when the user selects a new
//   selection name.
//
// Arguments:
//   index : the index of the selection name. 0==no selection.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 10 14:01:27 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetWindow::selectionChanged(const QString &selName)
{
    QString sName;
    if(selName != tr("None"))
        sName = selName;

    sel_dirty = sName != sel_selectionName;
    sel_selectionName = sName;

    Apply();
}
