// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisFilePanelItem.h>
#include <QualifiedFilename.h>

// Some static constants.
const int QvisFilePanelItem::ROOT_NODE = 0;
const int QvisFilePanelItem::HOST_NODE = 1;
const int QvisFilePanelItem::DIRECTORY_NODE = 2;
const int QvisFilePanelItem::FILE_NODE = 3;

// Static member variables.
int QvisFilePanelItem::globalNodeNumber = 0;

// *******************************************************************
// Method: QvisFilePanelItem::QvisFilePanelItem
//
// Purpose: 
//   A constructor for the QvisFilePanelItem class.
//
// Arguments:
//   parent : The tree widget to which this item will be added as a 
//            top level element.
//   str    : The text to display in the item.
//   qf     : The qualified filename associated with the item. This is
//            empty if the item is a host.
//   node   : The node type of the file list item. root,host,file,directory.
//   state  : The file's time state. It's > 0 if the file is a multi-
//            cycle database.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul  1 14:05:53 PDT 2008
//
// Notes: Qt4 port - adapated from Brad Whitlock's QvisListViewFileItem class.
//
// Modifications:
//
// *******************************************************************

QvisFilePanelItem::QvisFilePanelItem(QTreeWidget *parent,
    const QString &str, const QualifiedFilename &qf, 
    int node, int state,bool tsForced) 
: QTreeWidgetItem(parent), file(qf)
{
    setText(0,str);
    nodeType = node;
    timeState = state;
    nodeNumber = globalNodeNumber++;
    timeStateHasBeenForced = tsForced;
}

// *******************************************************************
// Method: QvisFilePanelItem::QvisFilePanelItem
//
// Purpose: 
//   A constructor for the QvisFilePanelItem class.
//
// Arguments:
//   parent : The tree widget item to which this item will be added as a 
//            child element.
//   str    : The text to display in the item.
//   qf     : The qualified filename associated with the item. This is
//            empty if the item is a host.
//   node   : The node type of the file list item. root,host,file,directory.
//   state  : The file's time state. It's > 0 if the file is a multi-
//            cycle database.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul  1 14:05:53 PDT 2008
//
// Notes: Qt4 port - adapated from Brad Whitlock's QvisListViewFileItem class.
//
// Modifications:
//
// *******************************************************************

QvisFilePanelItem::QvisFilePanelItem(QTreeWidgetItem *parent,
    const QString &str, const QualifiedFilename &qf, 
    int node, int state, bool tsForced) 
: QTreeWidgetItem(parent), file(qf)
{
    setText(0,str);
    nodeType = node;
    timeState = state;
    nodeNumber = globalNodeNumber++;
    timeStateHasBeenForced = tsForced;
}

// *******************************************************************
// Method: QvisFilePanelItem::~QvisFilePanelItem
//
// Purpose: 
//   Destructor for the QvisFilePanelItem class.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul  1 14:05:53 PDT 2008
//
// Modifications:
//   
// *******************************************************************

QvisFilePanelItem::~QvisFilePanelItem()
{}

/*
// ****************************************************************************
// Method: QvisFilePanelItem::paintCell
//
// Purpose: 
//   Paints the cell.
//
// Note:       Files that can't be read are grayed out.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 14 15:07:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisListViewFileItem::paintCell(QPainter *p, const QColorGroup &cg,
    int column, int width, int align)
{
    if(file.CanAccess())
        QTreeViewItem::paintCell(p, cg, column, width, align);
    else
    {
        QColorGroup newCg(cg);
        newCg.setColor(QColorGroup::Text, Qt::gray);
        newCg.setColor(QColorGroup::HighlightedText, Qt::gray);
        QTreeViewItem::paintCell(p, newCg, column, width, align);
    }
}

// ****************************************************************************
// Method: QvisFilePanelItem::key
//
// Purpose: 
//   This method returns the key used by the QTreeView container for sorting
//   the items in the QTreeView.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:10:53 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QString
QvisFilePanelItem::key(int, bool) const
{
    QString temp;
    temp.sprintf("%08d", nodeNumber);
    return temp;
}

*/

// ****************************************************************************
// Method: QvisFilePanelItem::resetNodeNumber
//
// Purpose: 
//   This is a static method that resets the number sequence used in the keys
//   to zero.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:11:45 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanelItem::resetNodeNumber()
{
    globalNodeNumber = 0;
}
