#include <QvisListViewFileItem.h>
#include <QualifiedFilename.h>

// Some static constants.
const int QvisListViewFileItem::ROOT_NODE = 0;
const int QvisListViewFileItem::HOST_NODE = 1;
const int QvisListViewFileItem::DIRECTORY_NODE = 2;
const int QvisListViewFileItem::FILE_NODE = 3;

// Static member variables.
int QvisListViewFileItem::globalNodeNumber = 0;

// *******************************************************************
// Method: QvisListViewFileItem::QvisListViewFileItem
//
// Purpose: 
//   This is the constructor for the QvisListViewFileItem class.
//
// Arguments:
//   parent : The listview to which this item will be added as a 
//            top level element.
//   str    : The text to display in the item.
//   qf     : The qualified filename associated with the item. This is
//            empty if the item is a host.
//   node   : The node type of the file list item. root,host,file,directory.
//   state  : The file's time state. It's > 0 if the file is a multi-
//            cycle database.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 12:16:30 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:32:42 PDT 2001
//   Added an argument.
//
// *******************************************************************

QvisListViewFileItem::QvisListViewFileItem(QListView *parent,
    const QString &str, const QualifiedFilename &qf, int node, int state_) :
    QListViewItem(parent, str), file(qf)
{
    nodeType = node;
    timeState = state_;
    nodeNumber = globalNodeNumber++;
}

// *******************************************************************
// Method: QvisListViewFileItem::QvisListViewFileItem
//
// Purpose: 
//   This is the constructor for the QvisListViewFileItem class.
//
// Arguments:
//   parent : The listviewitem to which this item will be added as a 
//            child element.
//   str    : The text to display in the item.
//   qf     : The qualified filename associated with the item. This is
//            empty if the item is a host.
//   node   : The node type of the file list item. root,host,file,directory.
//   state  : The file's time state. It's > 0 if the file is a multi-
//            cycle database.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 12:16:30 PDT 2000
//
// Modifications:
//   
// *******************************************************************

QvisListViewFileItem::QvisListViewFileItem(QListViewItem *parent,
    const QString &str, const QualifiedFilename &qf, int node, int state_) :
    QListViewItem(parent, str), file(qf)
{
    nodeType = node;
    timeState = state_;
    nodeNumber = globalNodeNumber++;
}

// *******************************************************************
// Method: QvisListViewFileItem::~QvisListViewFileItem
//
// Purpose: 
//   This is the destructor for the QvisListViewFileItem class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 12:18:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

QvisListViewFileItem::~QvisListViewFileItem()
{
    // nothing much
}

// ****************************************************************************
// Method: QvisListViewFileItem::paintCell
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
        QListViewItem::paintCell(p, cg, column, width, align);
    else
    {
        QColorGroup newCg(cg);
        newCg.setColor(QColorGroup::Text, Qt::gray);
        newCg.setColor(QColorGroup::HighlightedText, Qt::gray);
        QListViewItem::paintCell(p, newCg, column, width, align);
    }
}

// ****************************************************************************
// Method: QvisListViewFileItem::key
//
// Purpose: 
//   This method returns the key used by the QListView container for sorting
//   the items in the QListView.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:10:53 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QString
QvisListViewFileItem::key(int, bool) const
{
    QString temp;
    temp.sprintf("%08d", nodeNumber);
    return temp;
}

// ****************************************************************************
// Method: QvisListViewFileItem::resetNodeNumber
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
QvisListViewFileItem::resetNodeNumber()
{
    globalNodeNumber = 0;
}
