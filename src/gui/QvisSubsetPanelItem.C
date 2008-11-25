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


