#ifndef QVIS_SUBSET_LISTVIEW_H
#define QVIS_SUBSET_LISTVIEW_H
#include <gui_exports.h>
#include <qlistview.h>

// Forward declaration.
class QvisSubsetListViewItem;

// ****************************************************************************
// Class: QvisSubsetListView
//
// Purpose:
//   This is a subclass of the listview widget that can emit checked signals
//   when its listview items are checked.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:45:36 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisSubsetListView : public QListView
{
    Q_OBJECT
public:
    QvisSubsetListView(QWidget *parent, const char *name = 0);
    virtual ~QvisSubsetListView();
    void emitChecked(QvisSubsetListViewItem *item);
signals:
    void checked(QvisSubsetListViewItem *item);
public slots:
    virtual void resize(int w, int h);
};

#endif
