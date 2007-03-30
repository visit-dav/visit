#include <QvisSubsetListView.h>
#include <QvisSubsetListViewItem.h>

// ****************************************************************************
// Method: QvisSubsetListView::QvisSubsetListView
//
// Purpose: 
//   This is the constructor for the QvisSubsetListView class.
//
// Arguments:
//   parent : The widget's parent widget.
//   name   : The name of this widget instance.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:47:57 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListView::QvisSubsetListView(QWidget *parent, const char *name) :
    QListView(parent,name)
{
    // nothing
}

// ****************************************************************************
// Method: QvisSubsetListView::~QvisSubsetListView
//
// Purpose: 
//   This is the destructor for the QvisSubsetListView class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:48:48 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListView::~QvisSubsetListView()
{
    // nothing
}

// ****************************************************************************
// Method: QvisSubsetListView::resize
//
// Purpose: 
//   This is a Qt slot function that resizes the widget.
//
// Arguments:
//   w : The new widget width.
//   h : The new widget height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:49:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 16:48:49 PST 2001
//   Removed a stray debugging comment.
//
// ****************************************************************************

void
QvisSubsetListView::resize(int w, int h)
{
    // Call the parent's resize method.
    QListView::resize(w, h);

    // resize the width
    if(columns() > 0)
    {

        setColumnWidth(0, visibleWidth());
    }
}

// ****************************************************************************
// Method: QvisSubsetListView::emitChecked
//
// Purpose: 
//   This method tells the listview to emit a checked signal. This is used by
//   QvisSubsetListViewItem to tell the world when the item has been checked.
//
// Arguments:
//   item : A pointer to the item that was checked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:50:28 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetListView::emitChecked(QvisSubsetListViewItem *item)
{
    emit checked(item);
}
