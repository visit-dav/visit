#ifndef QVIS_HELP_LISTVIEW_ITEM_H
#define QVIS_HELP_LISTVIEW_ITEM_H
#include <gui_exports.h>
#include <qlistview.h>

// ****************************************************************************
// Class: QvisHelpListViewItem
//
// Purpose:
//   This class is a special listview that keeps a document name.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 15 15:44:25 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisHelpListViewItem : public QListViewItem
{
public:
    QvisHelpListViewItem(QListView *parent, QListViewItem *after);
    QvisHelpListViewItem(QListViewItem *parent, QListViewItem *after);
    virtual ~QvisHelpListViewItem();
    const QString &document() const;
    void setDocument(const QString &d);

private:
    QString doc;
};

#endif
