#ifndef SILODIRTREEVIEW_H
#define SILODIRTREEVIEW_H

#include <qlistview.h>
class SiloFile;
class SiloDir;

// ****************************************************************************
//  Class:  SiloDirTreeViewItem
//
//  Purpose:
//    An item for a SiloDirTreeView.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloDirTreeViewItem : public QListViewItem
{
  public:
    SiloDir *dir;

    SiloDirTreeViewItem(SiloDir *d, QListView *lv, const QString &n)
        : QListViewItem(lv,n), dir(d) { };
    SiloDirTreeViewItem(SiloDir *d, QListViewItem *li, const QString &n) 
        : QListViewItem(li,n), dir(d) { };
};

// ****************************************************************************
//  Class:  SiloDirTreeView
//
//  Purpose:
//    ListView-style container for viewing a Silo directory tree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloDirTreeView : public QListView
{
    Q_OBJECT
  public:
    SiloDirTreeView(SiloFile *s, QWidget *p, const QString &n);
    void Set(SiloFile *s);
    void OpenRootDir();

    virtual QSize sizeHint() const;

  protected:
    virtual void resizeEvent(QResizeEvent *re);

  private:
    int total_items;
    SiloFile *silo;
    QPixmap *folder_pixmap;

    QListViewItem *AddDir(SiloDir *d, QListViewItem *parent);
};

#endif
