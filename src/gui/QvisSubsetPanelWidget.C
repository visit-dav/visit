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

#include <QvisSubsetPanelWidget.h>

#include <QGridLayout>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QMenu>

#include <ViewerProxy.h>
#include <avtSILSet.h>
#include <avtSILNamespace.h>
#include <avtSILRestrictionTraverser.h>

#include <ViewerProxy.h>
#include <QvisSubsetPanelItem.h>



// ****************************************************************************
// Function: S2S
//
// Purpose:
//   Converts SetState to CheckedState.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:07:22 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

static CheckedState
S2S(SetState s)
{
    CheckedState retval = CompletelyChecked;
    if(s == NoneUsed)
        retval = NotChecked;
    else if(s == SomeUsed)
        retval = PartiallyChecked;

    return retval;                
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::QvisSubsetPanelWidget
//
// Purpose: 
//   Constructor for the QvisSubsetPanelWidget class.
//
// Arguments:
//   parent : The parent widget to this object.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// ****************************************************************************

QvisSubsetPanelWidget::QvisSubsetPanelWidget(QWidget *parent, 
                                             ViewerProxy *viewer_proxy) 
:  QWidget(parent), viewerProxy(viewer_proxy), activeChild(NULL),
   numChecked(0), numCheckable(0)
{
    // create contents
    
    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(1,3);
    tree = new QTreeWidget(this);
    
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    
    QTreeWidgetItem *header = new QTreeWidgetItem();
    header->setText(0,"");
    tree->setHeaderItem(header);
    
    connect(tree,SIGNAL(itemSelectionChanged()),
            this,SLOT(onItemSelectionChanged()));
    
    connect(tree,SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this,SLOT(onItemClicked(QTreeWidgetItem*,int)));
    
    layout->addWidget(tree,0,0,1,3);

    
    allSetsLabel = new QLabel(tr("All Sets:"),this);
    allSetsButton = new QPushButton(tr("Reverse"),this);
    allSetsActionButton = new QPushButton("",this);
    
    layout->addWidget(allSetsLabel,1,0);
    layout->addWidget(allSetsButton,1,1);
    layout->addWidget(allSetsActionButton,1,2);
    
    QMenu *allSetsMenu = new QMenu();
    allSetsMenu->addAction(tr("Reverse"),this,SLOT(onAllSetsActionReverse()));
    allSetsMenu->addAction(tr("Turn On"),this,SLOT(onAllSetsActionOn()));
    allSetsMenu->addAction(tr("Turn Off"),this,SLOT(onAllSetsActionOff()));
    allSetsActionButton->setMenu(allSetsMenu);
    
    connect(allSetsButton,SIGNAL(clicked()),
            this,SLOT(onAllSetsButton()));
    
    
    selectedSetsLabel = new QLabel(tr("Selected Sets:"),this);
    selectedSetsButton = new QPushButton(tr("Reverse"),this);
    selectedSetsActionButton = new QPushButton("",this);
    
    selectedSetsLabel->setEnabled(false);
    selectedSetsButton->setEnabled(false);
    selectedSetsActionButton->setEnabled(false);
    
    layout->addWidget(selectedSetsLabel,2,0);
    layout->addWidget(selectedSetsButton,2,1);
    layout->addWidget(selectedSetsActionButton,2,2);

    QMenu *selectedSetsMenu = new QMenu();
    selectedSetsMenu->addAction(tr("Reverse"),this,SLOT(onSelectedSetsActionReverse()));
    selectedSetsMenu->addAction(tr("Turn On"),this,SLOT(onSelectedSetsActionOn()));
    selectedSetsMenu->addAction(tr("Turn Off"),this,SLOT(onSelectedSetsActionOff()));
    selectedSetsActionButton->setMenu(selectedSetsMenu);
    
    connect(selectedSetsButton,SIGNAL(clicked()),
            this,SLOT(onSelectedSetsButton()));
    
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::QvisSubsetPanelWidget
//
// Purpose: 
//   Destructor for the QvisSubsetPanelWidget class.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//
// ****************************************************************************

QvisSubsetPanelWidget::~QvisSubsetPanelWidget()
{
}


// ****************************************************************************
// Method: QvisSubsetPanelWidget::SetTitle
//
// Purpose: 
//   Sets the panels title.
//
// Arguments:
//   title: New panel title.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::SetTitle(const QString &title)
{
    tree->headerItem()->setText(0,title);    
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::ViewSet
//
// Purpose: 
//   Displays a SIL set.
//
// Arguments:
//   id: SIL Collection Id
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::ViewSet(int id)
{
    avtSILRestriction_p restriction = viewerProxy->GetPlotSILRestriction();
    numCheckable = 0;
    numChecked = 0;
    tree->clear();
    tree->setEnabled(true);
    avtSILSet_p current = restriction->GetSILSet(id);
    const std::vector<int> &mapsOut = current->GetMapsOut();

    // Create a listview item with the name of the whole set.
    QString wholeName(current->GetName().c_str());
    avtSILRestrictionTraverser trav(restriction);
    
    CheckedState s = S2S(trav.UsesSetData(id));
    
    QvisSubsetPanelItem *item = new QvisSubsetPanelItem(tree, wholeName, s, id);

    numCheckable++;    
    if(s == CompletelyChecked)
        numChecked++;

    // Add all of the collections that come out of the whole.
    for(int j = 0; j < mapsOut.size(); ++j)
    {
        // cIndex is the j'th collection coming from out of the whole.
        int cIndex = mapsOut[j];

        // Create a new item under the whole and set its checked value.
        avtSILCollection_p collection = restriction->GetSILCollection(cIndex);
        QString collectionName(collection->GetCategory().c_str());
    
        QvisSubsetPanelItem *checkItem = new QvisSubsetPanelItem(item, 
                                                                 collectionName,
                                                                 cIndex);
    }

    item->setExpanded(true);
    EnableButtons(true);
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::ViewCollection
//
// Purpose: 
//   Displays a SIL collection.
//
// Arguments:
//   id: SIL Collection Id
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
//   Hank Childs, Fri Dec 11 11:37:48 PST 2009
//   Adapt to changes in SIL interface.
//
// ****************************************************************************

void
QvisSubsetPanelWidget::ViewCollection(int id)
{
    avtSILRestriction_p restriction = viewerProxy->GetPlotSILRestriction();
    avtSILCollection_p collection = restriction->GetSILCollection(id);
    numCheckable = 0;
    numChecked = 0;
    tree->clear();

    if(*collection != NULL)
    {
        tree->setEnabled(true);
        blockSignals(true);
          
        SetTitle(collection->GetCategory().c_str());
        const avtSILNamespace *ns = collection->GetSubsets();
        int numElems = ns->GetNumberOfElements();

        avtSILRestrictionTraverser trav(restriction);
        for(int i = 0; i < numElems; ++i)
        {
            int setIdx = ns->GetElement(i);
            avtSILSet_p set = restriction->GetSILSet(setIdx);

            // Create an item for the set and set its checked value.
            CheckedState s = S2S(trav.UsesSetData(setIdx));
            QString cname(set->GetName().c_str());
            QvisSubsetPanelItem *item = new QvisSubsetPanelItem(tree,
                                                                cname,
                                                                s,
                                                                setIdx);
            numCheckable++;

            if(s == CompletelyChecked)
                numChecked++;
            
            // Add all of the collections that come out of the set. Note that
            // they are added as uncheckable items.
            const std::vector<int> &mapsOut = set->GetMapsOut();
            for(int j = 0; j < mapsOut.size(); ++j)
            {
                int cIndex = mapsOut[j];

                avtSILCollection_p c = restriction->GetSILCollection(cIndex);
                QString collectionName(c->GetCategory().c_str());
                QvisSubsetPanelItem *cItem = new QvisSubsetPanelItem(item,
                                                                     collectionName,
                                                                     cIndex);
            }
        }
        blockSignals(false);
    }
    EnableButtons(true);
    
    
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::UpdateView
//
// Purpose: 
//   Displays a SIL collection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::UpdateView()
{
    avtSILRestriction_p restriction = viewerProxy->GetPlotSILRestriction();
    avtSILRestrictionTraverser trav(restriction);
    tree->blockSignals(true);
    QTreeWidgetItemIterator itr(tree);
    while(*itr)
    {
        QvisSubsetPanelItem *item = (QvisSubsetPanelItem *)*itr;
        if(item->parent() == NULL)
            item->setState(S2S(trav.UsesSetData(item->id())));
        ++itr;
    }
    tree->blockSignals(false);
    
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::SetSelectedItemState
//
// Purpose: 
//   Used by child panels to update the set state
//
// Arguments:
//   state: Current check state.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 09:46:40 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::SetSelectedItemState(int state)
{
    // get the selected item, and find its parent
    QvisSubsetPanelItem *item =(QvisSubsetPanelItem*) tree->currentItem();
    if(item)
    {
        QvisSubsetPanelItem *parent =(QvisSubsetPanelItem*) item->parent();
        if(parent)
        {
            if(parent->getState() != state)
            {
                // if it is on we must be turning it off, or making it mixed
                if(state == CompletelyChecked)
                    numChecked++;
                else if(parent->getState()  == CompletelyChecked )
                    numChecked--;

                parent->setState((CheckedState)state);
                UpdateParentState();
            }
        }
    }
}



// ****************************************************************************
// Method: QvisSubsetPanelWidget::Clear
//
// Purpose: 
//   Clears the Subset Panel.
//
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::Clear()
{
    tree->clear();
    tree->setEnabled(false);
    tree->headerItem()->setText(0,"");
    EnableButtons(false);

}


// ****************************************************************************
// Method: QvisSubsetPanelWidget::onAllSetsButton
//
// Purpose: 
//   Slot for the all sets button.
//   Changes the SIL status of all sets using the current mode.
//   (Reverse/On/Off)
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onAllSetsButton()
{
    if(allSetsButton->text() == tr("Reverse"))
        Reverse(false);
    else if(allSetsButton->text() == tr("Turn On"))
        TurnOnOff(true,false);
    else if(allSetsButton->text() == tr("Turn Off"))
        TurnOnOff(false,false);
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onAllSetsActionReverse
//
// Purpose: 
//   Sets the all sets button mode action to Reverse.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onAllSetsActionReverse()
{
    allSetsButton->setText(tr("Reverse"));
    onAllSetsButton();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onAllSetsActionOn
//
// Purpose: 
//   Sets the all sets button mode action to On.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onAllSetsActionOn()
{
    allSetsButton->setText(tr("Turn On"));
    onAllSetsButton();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onAllSetsActionOff
//
// Purpose: 
//   Sets the all sets button mode action to Off.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onAllSetsActionOff()
{
    allSetsButton->setText(tr("Turn Off"));
    onAllSetsButton();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onSelectedSetsButton
//
// Purpose: 
//   Slot for the selected sets button.
//   Changes the SIL status of selected sets using the current mode.
//   (Reverse/On/Off)
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onSelectedSetsButton()
{
    if(selectedSetsButton->text() == tr("Reverse"))
        Reverse(true);
    else if(selectedSetsButton->text() == tr("Turn On"))
        TurnOnOff(true,true);
    else if(selectedSetsButton->text() == tr("Turn Off"))
        TurnOnOff(false,true);
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onSelectedSetsActionReverse
//
// Purpose: 
//   Sets the selected sets button mode action to Reverse.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onSelectedSetsActionReverse()
{
    selectedSetsButton->setText(tr("Reverse"));
    onSelectedSetsButton();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onAllSetsActionOn
//
// Purpose: 
//   Sets the all sets button mode action to On.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onSelectedSetsActionOn()
{
    selectedSetsButton->setText(tr("Turn On"));
    onSelectedSetsButton();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onSelectedSetsActionOff
//
// Purpose: 
//   Sets the selected sets button mode action to Off.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::onSelectedSetsActionOff()
{
    selectedSetsButton->setText(tr("Turn Off"));
    onSelectedSetsButton();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onItemSelectionChanged
//
// Purpose: 
//   Handles selection change in the tree view.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void 
QvisSubsetPanelWidget::onItemSelectionChanged() 
{
    // if any root items are selected, make sure
    // child items are not
    
    tree->blockSignals(true);
    QListIterator<QTreeWidgetItem*> itr(tree->selectedItems());
    
    bool sel_root = false;
    while(itr.hasNext() && ! sel_root)
    {
        QTreeWidgetItem *item= itr.next();
        if(item->parent() == NULL)
            sel_root = true;
    }
    
    if(sel_root)
    {
        itr.toFront();
        while(itr.hasNext())
        {
            QTreeWidgetItem *item= itr.next();
            if(item->parent())
                item->setSelected(false);
        }
    }
    
    tree->blockSignals(false);
    
    bool has_sel = false;
    if(tree->selectedItems().count() > 0)
        has_sel = true;
    
    selectedSetsLabel->setEnabled(has_sel);
    selectedSetsButton->setEnabled(has_sel);
    selectedSetsActionButton->setEnabled(has_sel);

}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::onItemClicked
//
// Purpose: 
//   Handles when an item is clicked.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void 
QvisSubsetPanelWidget::onItemClicked(QTreeWidgetItem *item, int col)
{
    activeChild = NULL;    

    QvisSubsetPanelItem *spi = (QvisSubsetPanelItem*)item;
    
    // check for top level item
    if(spi->parent() == NULL)
    {
        // change checked state
        spi->toggleState();
        
        if(spi->isOn())
            numChecked++;
        else
            numChecked--;

        avtSILRestriction_p restriction = viewerProxy->GetPlotSILRestriction();
        if(spi->isOn())
            restriction->TurnOnSet(spi->id());
        else
            restriction->TurnOffSet(spi->id());
    
        // emit change for sub views!
        emit itemSelected(spi->id(),true);
    
        UpdateParentState();
    }
    else
    {
        //spi show subitem!
        activeChild = spi;
        emit itemSelected(spi->id(),false);
    }
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::UpdateSetStatus
//
// Purpose: 
//   Emits a signal specifying if all, none, or a some of the items are 
//   checked.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul 17 09:20:30 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::UpdateParentState()
{
    if(numChecked == 0)
        emit parentStateChanged(NotChecked); 
    else if(numChecked == numCheckable)
        emit parentStateChanged(CompletelyChecked); 
    else 
        emit parentStateChanged(PartiallyChecked); 
}


// ****************************************************************************
// Method: QvisSubsetPanelWidget::EnableButtons
//
// Purpose: 
//   Helper that enables/disables subset panel buttons.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::EnableButtons(bool on)
{
    allSetsLabel->setEnabled(on);
    allSetsButton->setEnabled(on);
    allSetsActionButton->setEnabled(on);
    
    selectedSetsLabel->setEnabled(on);
    selectedSetsButton->setEnabled(on);
    selectedSetsActionButton->setEnabled(on);
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::Reverse
//
// Purpose: 
//   Reverses the SIL state of sets. 
//
// Arguments:
//  only_selected: If all (false) or selected (true) tree items should be 
//  reversed.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::Reverse(bool only_selected)
{
    avtSILRestriction_p restriction = viewerProxy->GetPlotSILRestriction();
    restriction->SuspendCorrectnessChecking();

    bool update = false;
    QTreeWidgetItemIterator itr(tree);
    
    while(*itr)
    {
        QvisSubsetPanelItem *item = (QvisSubsetPanelItem *)*itr;
        if(item->parent() == NULL && (only_selected ? item->isSelected() : true))
        {
            bool prev_on = item->isOn();
            restriction->ReverseSet(item->id());
            item->toggleState();
            if(prev_on)
                numChecked--;
            else
                numChecked++;
            
            // if the child item is active, subpanels will depend on it
            if(activeChild && activeChild->parent() == item)
                update  = true;
        }
        ++itr;
    }

    restriction->EnableCorrectnessChecking();
    UpdateParentState();
    if(update)
        emit stateChanged();
}

// ****************************************************************************
// Method: QvisSubsetPanelWidget::Reverse
//
// Purpose: 
//   Sets the SIL state of sets.
//
// Arguments:
//  val: Turn sets on or off.
//  only_selected: If all (false) or selected (true) tree items should be 
//  set.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelWidget::TurnOnOff(bool on, bool only_selected)
{
    avtSILRestriction_p restriction = viewerProxy->GetPlotSILRestriction();
    restriction->SuspendCorrectnessChecking();

    bool update = false;
    QTreeWidgetItemIterator itr(tree);
    
    while(*itr)
    {
        QvisSubsetPanelItem *item = (QvisSubsetPanelItem *)*itr;
        if(item->parent() == NULL && (only_selected ? item->isSelected() : true))
        {
            bool prev_on = item->isOn();
            if(on)
            {
                if(!prev_on)
                {
                    restriction->TurnOnSet(item->id());
                    item->setState(CompletelyChecked);
                    numChecked++;
                }
            }
            else
            {
                restriction->TurnOffSet(item->id());
                item->setState(NotChecked);
                if(prev_on)
                    numChecked--;
            }
            
            // if the child item is active, subpanels will depend on it
            if(activeChild && activeChild->parent() == item)
                update  = true;
        }
        ++itr;
    }

    restriction->EnableCorrectnessChecking();
    UpdateParentState();
    if(update)
        emit stateChanged();

}

