#ifndef QVIS_SUBSET_LISTVIEW_ITEM_H
#define QVIS_SUBSET_LISTVIEW_ITEM_H
#include <gui_exports.h>
#include <qlistview.h>

typedef enum {NotChecked, PartiallyChecked, CompletelyChecked} CheckedState;

// ****************************************************************************
// Class: QvisSubsetListViewItem
//
// Purpose: 
//   This class is a listview item that has a tri-state checkbox and contains
//   an integer that represents set or collection id's.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:46:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisSubsetListViewItem : public QCheckListItem
{
public:
    QvisSubsetListViewItem(QListView * parent, const QString &text,
                           CheckedState s = NotChecked,
                           int id_ = 0);
    QvisSubsetListViewItem(QListViewItem *parent, const QString &text,
                           CheckedState s = NotChecked,
                           int id_ = 0);

    virtual ~QvisSubsetListViewItem();
    virtual QString key(int column, bool ascending) const;
    virtual void activate();
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int align);
    virtual void paintFocus(QPainter *p, const QColorGroup &cg,
                            const QRect &r);

    void         setCheckable(bool val);
    bool         getCheckable() const;
    void         setState(CheckedState s);
    CheckedState getState() const;
    int          id() const;
    void         setId(int id_);
    bool         isOn() const { return state != NotChecked; }

private:
    CheckedState state;
    int          itemId;
    bool         checkable;
};

#endif
