#include <QvisPlotListBox.h>
#include <QvisPlotListBoxItem.h>
#include <PlotList.h>
#include <Plot.h>
#include <QvisPlotListBoxItem.h>

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
// ****************************************************************************

QvisPlotListBox::QvisPlotListBox(QWidget *parent, const char *name, WFlags f) :
    QListBox(parent, name, f)
{
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
    clickHandler(p, false);
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
    clickHandler(p, true);
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
//   
// ****************************************************************************

void
QvisPlotListBox::clickHandler(const QPoint &clickLocation, bool doubleClicked)
{
    QPoint itemClickLocation(clickLocation);
    int y = 0;
    int heightSum = 0;
    for(int i = 0; i < count(); ++i)
    {
        QListBoxItem *current = item(i);
        int h = current->height(this);

        if(clickLocation.y() >= y && clickLocation.y() < (y + h))
        {
            // If the item is not selected, select it.
            bool bs = signalsBlocked();
            if(!bs)
                blockSignals(true);
            setSelected(current, true);
            if(!bs)
                blockSignals(false);

            // Reduce the y location of the click location
            itemClickLocation.setY(clickLocation.y() - heightSum);

            // Handle the click.
            itemClicked(current, itemClickLocation, doubleClicked);
            return;
        }

        heightSum += h;
        y += h;
    }
}

// ****************************************************************************
// Method: QvisPlotListBox::itemClicked
//
// Purpose: 
//   This method is called when an item is clicked. We use it to emit certain
//   signals depending on which areas of the item were clicked.
//
// Arguments:
//   item          : The item that was clicked.
//   point         : The point where the item was clicked.
//   doubleClicked : Whether or not we're handling a click or a double click.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 8 16:20:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotListBox::itemClicked(QListBoxItem *item, const QPoint &point,
    bool doubleClicked)
{
    if(item == 0)
        return;

    QvisPlotListBoxItem *item2 = (QvisPlotListBoxItem *)item;
    int id = 0;

    switch(item2->clicked(point, doubleClicked, id))
    {
    case 0: // expand clicked
        triggerUpdate(true);
        break;
    case 1: // subset clicked
        emit activateSubsetWindow();
        break;
    case 2: // plot clicked
        emit activatePlotWindow(id);
        break;
    case 3: // operator clicked
        emit activateOperatorWindow(id);
        break;
    case 4: // promote clicked
        emit promoteOperator(id);
        break;
    case 5: // demote clicked
        emit demoteOperator(id);
        break;
    case 6: // delete clicked
        emit removeOperator(id);
        break;
    default:
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
// ****************************************************************************

bool
QvisPlotListBox::NeedsToBeRegenerated(const PlotList *pl) const
{
    bool retval = true;

    if(pl->GetNumPlots() == count())
    {
        for(int i = 0; i < pl->GetNumPlots(); ++i)
        {
            QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
            const Plot &newPlot = pl->operator[](i);
            const Plot &currentPlot = lbi->GetPlot();

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
//   
// ****************************************************************************

bool
QvisPlotListBox::NeedToUpdateSelection(const PlotList *pl) const
{
    bool retval = true;

    if(pl->GetNumPlots() == count())
    {
        for(int i = 0; i < pl->GetNumPlots(); ++i)
        {
            QvisPlotListBoxItem *lbi = (QvisPlotListBoxItem *)item(i);
            const Plot &newPlot = pl->operator[](i);
            const Plot &currentPlot = lbi->GetPlot();

            bool nu = newPlot.GetActiveFlag() != currentPlot.GetActiveFlag();

            if(nu) return true;
        }
        return false;
    }

    return retval;
}
