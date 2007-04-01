#include <QvisSubsetWindow.h>
#include <QvisSubsetListView.h>
#include <QvisSubsetListViewItem.h>
#include <qhbox.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qtimer.h>

#include <ViewerProxy.h>
#include <avtSILNamespace.h>
#include <avtSILRestrictionTraverser.h>

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
// ****************************************************************************

QvisSubsetWindow::QvisSubsetWindow(Subject *subj, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false),
    listViews()
{
    // Set these to uninitialized.
    sil_TopSet = -1;
    sil_NumSets = -1;
    sil_NumCollections = -1;

    // Initialize widgets that we'll delete manually later.
    turnOnGroup = 0;
    turnOffGroup = 0;
    turnReverseGroup = 0;
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
// ****************************************************************************

QvisSubsetWindow::~QvisSubsetWindow()
{
    delete turnOnGroup;
    delete turnOffGroup;
    delete turnReverseGroup;
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
// ****************************************************************************

void
QvisSubsetWindow::CreateWindowContents()
{
    // Create some button groups.
    turnOnGroup = new QButtonGroup(0, "turnOnGroup");
    connect(turnOnGroup, SIGNAL(clicked(int)),
            this, SLOT(turnOn(int)));
    turnOffGroup = new QButtonGroup(0, "turnOffGroup");
    connect(turnOffGroup, SIGNAL(clicked(int)),
            this, SLOT(turnOff(int)));
    turnReverseGroup = new QButtonGroup(0, "turnReverseGroup");
    connect(turnReverseGroup, SIGNAL(clicked(int)),
            this, SLOT(TurnReverse(int)));
 
    scrollView = new QScrollView(central, "scrollView");
    scrollView->setHScrollBarMode(QScrollView::Auto);
    scrollView->setVScrollBarMode(QScrollView::AlwaysOff);
    scrollView->viewport()->setBackgroundMode(PaletteBackground);
    topLayout->addWidget(scrollView);

    // Add a few list views.
    lvSplitter = new QSplitter(scrollView->viewport(), "lvSplitter");
    lvSplitter->setOpaqueResize(false);
    scrollView->addChild(lvSplitter);
    AddListView();
    AddListView();
    AddListView();
    listViews[0].lv->setColumnText(0, "Whole");
    listViews[0].lv->setColumnWidth(0, listViews[0].lv->visibleWidth());
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
// ****************************************************************************

void
QvisSubsetWindow::UpdateWindow(bool)
{
    // Get a reference to the SIL restriction in the viewer proxy.
    avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();

    // If we're looking at what is probably a new SIL restriction, update
    // the contents of the listviews.
    if(restriction->GetNumSets() != sil_NumSets ||
       restriction->GetNumCollections() != sil_NumCollections ||
       restriction->GetTopSet() != sil_TopSet)
    {
        sil_TopSet = restriction->GetTopSet();
        sil_NumSets = restriction->GetNumSets();
        sil_NumCollections = restriction->GetNumCollections();

        listViews[0].lv->blockSignals(true);
        listViews[0].lv->clear();

        // Get the restriction's top set and add it to the panel.
        int topSet = restriction->GetTopSet();
        bool validTopSet = topSet > -1;
        if(validTopSet)
        {
            avtSILSet_p current = restriction->GetSILSet(topSet);
            const std::vector<int> &mapsOut = current->GetMapsOut();

            // Create a listview item with the name of the whole set.
            QString wholeName(current->GetName().c_str());
            avtSILRestrictionTraverser trav(restriction);
            CheckedState s = S2S(trav.UsesSetData(topSet));
            QvisSubsetListViewItem *item = new QvisSubsetListViewItem(
                listViews[0].lv, wholeName, s, topSet);

            // Add all of the collections that come out of the whole.
            for(int j = 0; j < mapsOut.size(); ++j)
            {
                // cIndex is the j'th collection coming from out of the whole.
                int cIndex = mapsOut[j];

                // Create a new item under the whole and set its checked value.
                avtSILCollection_p collection = 
                                         restriction->GetSILCollection(cIndex);
                QString collectionName(collection->GetCategory().c_str());
                QvisSubsetListViewItem *checkItem = new QvisSubsetListViewItem(
                    item, collectionName, NotChecked, cIndex);
                checkItem->setCheckable(false);
            }

            // Make sure that the whole is initially open.
            item->setOpen(true);
        }

        listViews[0].lv->setColumnText(0, "Whole");
        listViews[0].lv->blockSignals(false);

        // Clear all but the first listviews.
        ClearListViewsToTheRight(1);

        // Set the enabled state of the buttons.
        listViews[0].turnOn->setEnabled(validTopSet);
        listViews[0].turnOff->setEnabled(validTopSet);
        listViews[0].turnReverse->setEnabled(validTopSet);
    }
    else
    {
        // The restriction's values matched the values that the window saved
        // about the restriction the last time through this routine. Assume
        // that the restrictions correspond to the same SIL. Since we make
        // this assumption, it means that the checked values of the window
        // only need to be updated.
        UpdateCheckMarks(-1);
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::GetNextListViewIndex
//
// Purpose: 
//   Returns the index of the next listview given a pointer to a listview.
//
// Arguments:
//   lv : The listview that we're looking for in the list.
//
// Returns:    The index of the next listview or -1 if the specified listview
//             is not in the list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:11:31 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 20 15:52:18 PST 2001
//   Modified to support new widget layout.
//
// ****************************************************************************

int
QvisSubsetWindow::GetNextListViewIndex(QListView *lv)
{
    int retval = -1;

    for(int i = 0; i < listViews.size(); ++i)
    {
        if(listViews[i].lv == lv)
        {
            retval = i;
            break;
        }
    }

    // If there is no next list view, then return -1 so we will
    // know to create one.
    if(retval > -1 && retval < listViews.size() - 1)
    {
        ++retval;
    }
    else
        retval = -1;

    return retval;
}

// ****************************************************************************
// Method: QvisSubsetWindow::AddListView
//
// Purpose: 
//   Creates a new listview and adds it to the window.
//
// Arguments:
//   visible : Whether or not the listview is initially visible.
//
// Returns:    The index of the new listview in the list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:12:57 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 17:03:46 PST 2001
//   Set the minimum width of the listview based on the font size.
//
//   Brad Whitlock, Thu Dec 20 15:52:47 PST 2001
//   Changed how we keep track of widgets.
//
//   Brad Whitlock, Fri Feb 8 15:52:11 PST 2002
//   Modified the code to use a splitter.
//
//   Brad Whitlock, Wed Dec 4 16:17:22 PST 2002
//   Renamed some buttons.
//
//   Brad Whitlock, Tue Apr 22 15:30:27 PST 2003
//   I made buttons be disabled by default.
//
// ****************************************************************************

int
QvisSubsetWindow::AddListView(bool visible)
{
    QString tmp;
    tmp.sprintf("L%d", listViews.size());

    // Create a new list view.
    SubsetPanel entry;
    entry.frame = new QFrame(lvSplitter, tmp.latin1());
    lvSplitter->setResizeMode(entry.frame, QSplitter::Stretch);
    entry.layout = new QGridLayout(entry.frame, 3, 2);
    entry.lv = new QvisSubsetListView(entry.frame, tmp.latin1());
    entry.layout->addMultiCellWidget(entry.lv, 0, 0, 0, 1);
    connect(entry.lv, SIGNAL(clicked(QListViewItem *)),
            this, SLOT(listviewClicked(QListViewItem *)));
    connect(entry.lv, SIGNAL(checked(QvisSubsetListViewItem *)),
            this, SLOT(listviewChecked(QvisSubsetListViewItem *)));
    entry.lv->addColumn("");
    entry.lv->setColumnWidthMode(0, QListView::Manual);
    if(!visible)
        entry.frame->hide();

    // Create some buttons.
    tmp.sprintf("turnOn%d", listViews.size());
    entry.turnOn = new QPushButton("All", entry.frame, tmp.latin1());
    entry.turnOn->setEnabled(false);
    entry.layout->addWidget(entry.turnOn, 1, 0);
    turnOnGroup->insert(entry.turnOn);

    tmp.sprintf("turnOff%d", listViews.size());
    entry.turnOff = new QPushButton("None", entry.frame, tmp.latin1());
    entry.turnOff->setEnabled(false);
    entry.layout->addWidget(entry.turnOff, 1, 1);
    turnOffGroup->insert(entry.turnOff);

    tmp.sprintf("turnReverse%d", listViews.size());
    entry.turnReverse = new QPushButton("Reverse", entry.frame, tmp.latin1());
    entry.turnReverse->setEnabled(false);
    entry.layout->addMultiCellWidget(entry.turnReverse, 2, 2, 0, 1);
    turnReverseGroup->insert(entry.turnReverse);

    // Add the entry to the vector.
    listViews.push_back(entry);

    return listViews.size() - 1;
}

// ****************************************************************************
// Method: QvisSubsetWindow::ItemClicked
//
// Purpose: 
//   This method is called when a collection is clicked in a listview. When a
//   collection is clicked, we put the sets in the collection in the listview
//   to the right of the one that was clicked.
//
// Arguments:
//   item        : A pointer to the listview item that was clicked. This
//                 contains the collectionId for the collection that was
//                 clicked.
//   lvNextIndex : The index of the listview that we want to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:13:53 PDT 2001
//
// Modifications:
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Use a reference-counted SIL restriction instead of a reference to one.
//
//   Brad Whitlock, Wed Apr 23 14:16:40 PST 2003
//   I added code to turn the buttons on in the next listview.
//
// ****************************************************************************

void
QvisSubsetWindow::ItemClicked(QvisSubsetListViewItem *item, int lvNextIndex)
{
    // Get a reference to the viewer proxy's SIL restriction.
    avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();

    // Get the collection from the sil.
    avtSILCollection_p collection = restriction->GetSILCollection(item->id());
    if(*collection != NULL)
    {
        QListView *lvNext = listViews[lvNextIndex].lv;

        lvNext->blockSignals(true);
        lvNext->clear();
            
        lvNext->setColumnText(0, collection->GetCategory().c_str());
        const avtSILNamespace *ns = collection->GetSubsets();
        const std::vector<int> &sets = ns->GetAllElements();

        avtSILRestrictionTraverser trav(restriction);
        for(int i = 0; i < sets.size(); ++i)
        {
            avtSILSet_p set = restriction->GetSILSet(sets[i]);

            // Create an item for the set and set its checked value.
            CheckedState s = S2S(trav.UsesSetData(sets[i]));
            QvisSubsetListViewItem *n = new QvisSubsetListViewItem(lvNext,
                QString(set->GetName().c_str()), s, sets[i]);
            n->setCheckable(true);

            // Add all of the collections that come out of the set. Note that
            // they are added as uncheckable items.
            const std::vector<int> &mapsOut = set->GetMapsOut();
            for(int j = 0; j < mapsOut.size(); ++j)
            {
                int cIndex = mapsOut[j];

                avtSILCollection_p c = restriction->GetSILCollection(cIndex);
                QString collectionName(c->GetCategory().c_str());
                QvisSubsetListViewItem *cItem = new QvisSubsetListViewItem(n,
                    collectionName, NotChecked, cIndex);
                cItem->setCheckable(false);
            }
        }

        lvNext->blockSignals(false);

        // Turn on the buttons.
        listViews[lvNextIndex].turnOn->setEnabled(true);
        listViews[lvNextIndex].turnOff->setEnabled(true);
        listViews[lvNextIndex].turnReverse->setEnabled(true);

        // Update all of the column widths
        UpdateColumnWidths();
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::ClearListViewsToTheRight
//
// Purpose: 
//   Empties the contents of all listviews to the right of the specified
//   listview (including the specified listview).
//
// Arguments:
//   index : The index of the listview we want to clear.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:16:20 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 20 16:16:11 PST 2001
//   Modified to support additional buttons.
//
//   Brad Whitlock, Tue Apr 22 15:34:25 PST 2003
//   I made the buttons be disabled.
//
// ****************************************************************************

void
QvisSubsetWindow::ClearListViewsToTheRight(int index)
{
    for(int i = index; i < listViews.size(); ++i)
    {
        listViews[i].lv->blockSignals(true);
        listViews[i].lv->clear();
        listViews[i].lv->setColumnText(0, QString(""));
        listViews[i].lv->blockSignals(false);
        listViews[i].turnOn->setEnabled(false);
        listViews[i].turnOff->setEnabled(false);
        listViews[i].turnReverse->setEnabled(false);
        if(i > 2)
            listViews[i].frame->hide();
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::UpdateColumnWidths
//
// Purpose: 
//   Updates the widths of the listviews' headers.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:17:27 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 20 16:16:11 PST 2001
//   Modified to support additional buttons.
//   
// ****************************************************************************

void
QvisSubsetWindow::UpdateColumnWidths()
{
    for(int i = 0; i < listViews.size(); ++i)
        listViews[i].lv->setColumnWidth(0, listViews[i].lv->visibleWidth());
}

// ****************************************************************************
// Method: QvisSubsetWindow::UpdateCheckMarks
//
// Purpose: 
//   Updates the checkmarks for all visible listViews except the listview with
//   the specified index.
//
// Arguments:
//   lvIndex : The index of the listview that we do *not* want to update.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:18:08 PDT 2001
//
// Modifications:
//   Eric Brugger, Thu Oct 25 11:41:39 PDT 2001
//   I changed a test to use isHidden instead of !isVisible since we want
//   to know if we have explicitly hidden the widget and isVisible doesn't
//   have those exact semantics.  In particular if the window is unmapped,
//   isVisible returns false.
//   
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Account for changing interface of SIL restriction.
//
// ****************************************************************************

void
QvisSubsetWindow::UpdateCheckMarks(int lvIndex)
{
    avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();
    avtSILRestrictionTraverser trav(restriction);

    for(int i = 0; i < listViews.size(); ++i)
    {
        // Skip the specified listview and any that are hidden. Also
        // skip them if they have no collection that they are displaying.
        if((i == lvIndex) || listViews[i].lv->isHidden())
            continue;

        // Iterate through all of the listview items in the listview. If
        // they are top-level they are sets, otherwise they are collections.
        QListViewItemIterator it(listViews[i].lv);
        for ( ; it.current(); ++it)
        {
            QvisSubsetListViewItem *item = (QvisSubsetListViewItem *)it.current();
            if(item->parent() == NULL)
            {
                item->setState(S2S(trav.UsesSetData(item->id())));
            }
        }
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::TurnOnOff
//
// Purpose: 
//   Turns all of the sets in the specified panel on or off.
//
// Arguments:
//   index : The index of the panel.
//   val   : The value that we're setting.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 16:23:33 PST 2001
//
// Modifications:
//   Hank Childs, Thu Feb  7 16:29:33 PST 2002
//   Make use of new SIL methods for mass setting.
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Use a reference-counted SIL restriction instead of a reference to one.
//
// ****************************************************************************

void
QvisSubsetWindow::TurnOnOff(int index, bool val)
{
    avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();
    restriction->SuspendCorrectnessChecking();

    QListViewItemIterator it(listViews[index].lv);
    for ( ; it.current(); ++it)
    {
        QvisSubsetListViewItem *item = (QvisSubsetListViewItem *)it.current();
        if(item->parent() == NULL)
        {
            if(val)
                restriction->TurnOnSet(item->id());
            else
                restriction->TurnOffSet(item->id());
        }
    }

    restriction->EnableCorrectnessChecking();

    // Update all the checkmarks.
    for(int i = 0; i < listViews.size(); ++i)
    {
        // Skip any that are hidden.
        if(listViews[i].lv->isHidden())
            continue;
        UpdateCheckMarks(i);
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::Apply
//
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
        viewer->SetPlotSILRestriction();
    }
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSubsetWindow::listviewClicked
//
// Purpose: 
//   This is a Qt slot function that is called when a listview item is clicked.
//
// Arguments:
//   item : A pointer to the item that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:23:08 PDT 2001
//
// Modifications:
//   Eric Brugger, Thu Oct 25 11:41:39 PDT 2001
//   I changed a test to use isHidden instead of !isVisible since we want
//   to know if we have explicitly hidden the widget and isVisible doesn't
//   have those exact semantics.  In particular if the window is unmapped,
//   isVisible returns false.
//
//   Brad Whitlock, Thu Dec 20 16:16:11 PST 2001
//   Modified to support additional buttons.
//   
// ****************************************************************************

void
QvisSubsetWindow::listviewClicked(QListViewItem *item)
{
    // Make sure that something was clicked. If the mouse did not actually
    // contact a listview item, the item could be NULL.
    if(item == NULL)
        return;

    // Get the index of the next list view.
    int lvNextIndex = GetNextListViewIndex(item->listView());
    if(lvNextIndex == -1)
    {
        lvNextIndex = AddListView();
    }
    // If the next listview is hidden, show it.
    QListView *lvNext = listViews[lvNextIndex].lv;
    if(lvNext->isHidden())
        lvNext->show();

    if(item->parent() != NULL)
    {
        // Cast the item pointer to QvisSubsetListViewItem.
        QvisSubsetListViewItem *item2 = (QvisSubsetListViewItem *)item;

        // Call the code that is used to update the contents of the listview
        // for an item that is not a top-level item.
        ItemClicked(item2, lvNextIndex);

        // Clear all of the listviews to the right...
        ClearListViewsToTheRight(lvNextIndex + 1);

        // Update the checked status for all listviews except the current
        // listview.
        UpdateCheckMarks(lvNextIndex - 1);
    }
    else
    {
        // Clear all of the listviews to the right. This makes sure that the
        // collectionId for the listviews to the right is cleared.
        ClearListViewsToTheRight(lvNextIndex);

        // Update the checked status for all listviews except the current
        // listview.
        UpdateCheckMarks(lvNextIndex - 1);
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::listviewChecked
//
// Purpose: 
//   This is a Qt slot function that is called when listview items are checked.
//
// Arguments:
//   item : A pointer to the item that was checked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:25:35 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:16:51 PST 2002
//   Added support for auto update.
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Use a reference-counted SIL restriction instead of a reference to one.
//
// ****************************************************************************

void
QvisSubsetWindow::listviewChecked(QvisSubsetListViewItem *item)
{
    // Only top-level items can be checked.
    if(item->parent() == NULL)
    {
        // Get whether or not the item is selected. Since the item is
        // a top-level item, it must be a set. Therefore, we want to
        // set the set's selected status in the SIL restriction.
        avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();
        if(item->isOn())
            restriction->TurnOnSet(item->id());
        else
            restriction->TurnOffSet(item->id());

        // Notify the viewer if necessary.
        Apply();
    }
}

// ****************************************************************************
// Method: QvisSubsetWindow::turnOff
//
// Purpose: 
//   Turn off all sets in the panel.
//
// Arguments:
//    index : The index of the panel.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 16:21:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:20:49 PST 2002
//   Added support for auto update.
//
// ****************************************************************************

void
QvisSubsetWindow::turnOff(int index)
{
    TurnOnOff(index, false);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetWindow::turnOn
//
// Purpose: 
//   Turn on all sets in the panel.
//
// Arguments:
//    index : The index of the panel.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 16:21:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:20:49 PST 2002
//   Added support for auto update.
//   
// ****************************************************************************

void
QvisSubsetWindow::turnOn(int index)
{
    TurnOnOff(index, true);
    Apply();
}

// ****************************************************************************
// Method: QvisSubsetWindow::TurnReverse
//
// Purpose: 
//   Turns all of the sets in the specified panel to the reverse of what
//   they are currently.
//
// Arguments:
//   index : The index of the panel.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 16:23:33 PST 2001
//
// Modifications:
//   Hank Childs, Thu Feb  7 16:29:33 PST 2002
//   Make use of new SIL methods for mass setting.
//
//   Brad Whitlock, Mon Mar 4 15:20:49 PST 2002
//   Added support for auto update.
//
//   Hank Childs, Mon Dec  2 13:41:37 PST 2002
//   Use a reference-counted SIL restriction instead of a reference to one.
//
//   Brad Whitlock, Fri Jul 30 14:51:29 PST 2004
//   I made it use a new SIL restriction method to reverse the sets under
//   the set that we're reversing so we don't mess it up if there are sets
//   that are partially on.
//
// ****************************************************************************

void
QvisSubsetWindow::TurnReverse(int index)
{
    avtSILRestriction_p restriction = viewer->GetPlotSILRestriction();
    restriction->SuspendCorrectnessChecking();

    QListViewItemIterator it(listViews[index].lv);
    avtSILRestrictionTraverser trav(restriction);
    for ( ; it.current(); ++it)
    {
        QvisSubsetListViewItem *item = (QvisSubsetListViewItem *)it.current();
        if(item->parent() == NULL)
            restriction->ReverseSet(item->id());
    }

    restriction->EnableCorrectnessChecking();

    // Update all the checkmarks.
    for(int i = 0; i < listViews.size(); ++i)
    {
        // Skip any that are hidden.
        if(listViews[i].lv->isHidden())
            continue;
        UpdateCheckMarks(i);
    }

    Apply();
}

// ****************************************************************************
// Method: QvisSubsetWindow::show
//
// Purpose: 
//   This is a Qt slot function that is called when the window is shown.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:27:24 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:00:56 PST 2001
//   Made window postable.
//
//   Brad Whitlock, Tue Sep 25 16:53:48 PST 2001
//   Added code to resize the window's listviews a little.
//
//   Brad Whitlock, Fri Feb 8 15:42:59 PST 2002
//   Made the call to resize the window a little be on a timer so the window
//   has more time to be mapped before making geometry calculations.
//
// ****************************************************************************

void
QvisSubsetWindow::show()
{
    // Call the parent class's show method.
    QvisPostableWindowObserver::show();

    // Update the window to the appropriate sizes.
    QTimer::singleShot(300, this, SLOT(specialResize()));
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
// Method: QvisSubsetWindow::resizeEvent
//
// Purpose: 
//   This a Qt event handler that is called when the window is resized.
//
// Arguments:
//   e : The resize event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 11:52:31 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:01:12 PST 2001
//   Made window postable.
//
// ****************************************************************************

void
QvisSubsetWindow::resizeEvent(QResizeEvent *e)
{
    // Determine if the window is shrinking.
    bool shrinking = (e->size().width() < e->oldSize().width()) ||
                     (e->size().height() < e->oldSize().height());

    QvisPostableWindowObserver::resizeEvent(e);

    // Make the contents area of the scrollview as large as the viewport.
    if(isCreated)
    {
        // Resize the contents of the scrollview.
        specialResize();

        // If the window is shrinking then also to a delayed resize to
        // take care of any delayed changes of size in the size of the
        // scrollview widget.
        if(shrinking)
        {
            QTimer::singleShot(300, this, SLOT(specialResize()));
        }
    }        
}

// ****************************************************************************
// Method: QvisSubsetWindow::specialResize
//
// Purpose: 
//   This is a Qt slot function that resizes the listviews in the scrollview.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 25 14:22:35 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 8 15:52:11 PST 2002
//   Modified the code to use a splitter.
//   
// ****************************************************************************

void
QvisSubsetWindow::specialResize()
{
    int minWidth = listViews.size()*listViews[0].lv->minimumWidth() +
                       (listViews.size() - 1) * 5;
    int w = minWidth;
    if(scrollView->visibleWidth() > minWidth)
         w = scrollView->visibleWidth();
    int h = scrollView->visibleHeight();
    lvSplitter->resize(w, h);

    // Update all of the column widths
    UpdateColumnWidths();
}

// ****************************************************************************
// Method: QvisSubsetWindow::post
//
// Purpose: 
//   This is a Qt slot function that posts the window to the notepad.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 25 14:23:21 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 8 15:52:11 PST 2002
//   Modified the code to use a splitter.
//
// ****************************************************************************

void
QvisSubsetWindow::post()
{
    QvisPostableWindowObserver::post();

    // Do a delayed resize to let the geometry management catch up before
    // resizing the listviews.
    QTimer::singleShot(300, this, SLOT(specialResize()));
}

// ****************************************************************************
// Method: QvisSubsetWindow::unpost
//
// Purpose: 
//   This is a Qt slot function that unposts the window to the notepad.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 25 14:23:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetWindow::unpost()
{
    QvisPostableWindowObserver::unpost();

    // Do a delayed resize to let the geometry management catch up before
    // resizing the listviews.
    QTimer::singleShot(300, this, SLOT(specialResize()));
}
