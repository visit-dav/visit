// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSubsetPanelItem.h>
#include <icons/silchecked.xpm>
#include <icons/silmixed.xpm>
#include <icons/silunchecked.xpm>


// static vars
bool  QvisSubsetPanelItem::iconsReady = false;
QIcon QvisSubsetPanelItem::iconChecked;
QIcon QvisSubsetPanelItem::iconMixed;
QIcon QvisSubsetPanelItem::iconUnchecked;

// ****************************************************************************
// Method: QvisSubsetPanelItem::QvisSubsetPanelItem 
//
// Purpose: 
//   Constructor for the QvisSubsetPanelItem class.
//
// Arguments:
//   parent : A pointer to the item's parent tree. This will make the new
//            item a top-level item.
//   text   : The text to be displayed for the item.
//   s      : The item's checked state.
//   id     : The set id to associate with the item.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//    Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

QvisSubsetPanelItem::QvisSubsetPanelItem(QTreeWidget *parent,
                                         const QString &text,
                                         CheckedState s,
                                         int id)
: QTreeWidgetItem(parent,Qt::ItemIsUserCheckable)
{
    state = Unset;
    checkable = true;
    setState(s);
    setText(0,text);
    itemId = id;
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::QvisSubsetPanelItem
//
// Purpose: 
//   Constructor for the QvisSubsetPanelItem class.
//
// Arguments:
//   parent : A pointer to the item's parent tree item. This will make the
//            new item a low-level item.
//   text   : The text to be displayed for the item.
//   id     : The collection id to associate with the item.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:34:03 PDT 2001
//
// Modifications:
//    Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

QvisSubsetPanelItem::QvisSubsetPanelItem(QTreeWidgetItem *parent,
                                         const QString &text,
                                         int id) 
: QTreeWidgetItem(parent)
{
    state = Unset;
    checkable = false;
    setText(0,text);
    itemId = id;
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::~QvisSubsetPanelItem
//
// Purpose: 
//   Destructor for the QvisSubsetPanelItem class.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetPanelItem::~QvisSubsetPanelItem()
{
    // nothing
}


// ****************************************************************************
// Method: QvisSubsetPanelItem::id
//
// Purpose: 
//   Returns the set or collection id associated with the item.
//
// Returns:    The set or collection id associated with the item.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisSubsetPanelItem::id() const
{
    return itemId;
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::setId
//
// Purpose: 
//   Sets the item's set or collection id.
//
// Arguments:
//   id : The new id.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelItem::setId(int id)
{
    itemId = id;
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::setState
//
// Purpose: 
//   Sets the checked state of the item.
//
// Arguments:
//   s : The new checked state.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisSubsetPanelItem::setState(CheckedState s)
{
    if(!checkable)
        return;
    
    if(state != s)
    {
        // set the check state
        state = s;
        
        if(!iconsReady)
            InitIcons();

        // change the icon
        if(s == CompletelyChecked)
            setIcon(0,iconChecked);
        else if(s == PartiallyChecked)
            setIcon(0,iconMixed);
        else
            setIcon(0,iconUnchecked);
    }
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::getState
//
// Purpose: 
//   Returns the item's checked state.
//
// Returns:    The item's checked state.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

CheckedState
QvisSubsetPanelItem::getState() const
{
    return state;
}


// ****************************************************************************
// Method: QvisSubsetPanelItem::toggleState
//
// Purpose: 
//   Returns the item's checked state.
//
// Returns:    The item's checked state.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelItem::toggleState()
{
    if(state == PartiallyChecked || state == NotChecked)
        setState(CompletelyChecked);
    else
        setState(NotChecked);
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::setCheckable
//
// Purpose: 
//   Sets whether or not the item can be checked.
//
// Arguments:
//   val : Whether or not the item can be checked.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelItem::setCheckable(bool val)
{
    checkable = val;
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::getCheckable
//
// Purpose: 
//   Returns whether or not the item can be checked.
//
// Returns:    Whether or not the item can be checked.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSubsetPanelItem::getCheckable() const
{
    return checkable;
}

// ****************************************************************************
// Method: QvisSubsetPanelItem::getCheckable
//
// Purpose: 
//   Returns whether or not the item can be checked.
//
// Returns:    Whether or not the item can be checked.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetPanelItem::InitIcons() 
{
    QPixmap pixmapChecked(silchecked_xpm);
    QPixmap pixmapMixed(silmixed_xpm);
    QPixmap pixmapUnchecked(silunchecked_xpm);
    
    iconChecked   = QIcon(pixmapChecked);
    iconMixed     = QIcon(pixmapMixed);
    iconUnchecked = QIcon(pixmapUnchecked);
    
    iconsReady    = true;
}


