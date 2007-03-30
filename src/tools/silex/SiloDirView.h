#ifndef SILODIRVIEW_H
#define SILODIRVIEW_H

#include <qlistview.h>
class SiloDir;

// ****************************************************************************
//  Class:  SiloDirViewItem
//
//  Purpose:
//    An item for a SiloDirView.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloDirViewItem : public QListViewItem
{
  public:
    SiloDir *dir;
    QString  name;

    SiloDirViewItem(SiloDir *d, QListView *lv, const QString &n)
        : QListViewItem(lv,n), dir(d), name(n) { };
    SiloDirViewItem(SiloDir *d, QListViewItem *li, const QString &n) 
        : QListViewItem(li,n), dir(d), name(n) { };
};

// ****************************************************************************
//  Class:  SiloDirView
//
//  Purpose:
//    ListView-style container for viewing the TOC of a Silo directory.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloDirView : public QListView
{
    Q_OBJECT
  public:
    SiloDirView(QWidget *p, const QString &n);

    void Set(SiloDir *d);

    virtual QSize sizeHint() const;

  public slots:
    void ChangeDir(QListViewItem *i);

  protected:
    virtual void resizeEvent(QResizeEvent *re);

  private:
    int total_items;
    QPixmap *mesh_pixmap;
    QPixmap *mat_pixmap;
    QPixmap *var_pixmap;
    QPixmap *obj_pixmap;
    QPixmap *spec_pixmap;
    QPixmap *curve_pixmap;
    QPixmap *array_pixmap;
    QPixmap *silovar_pixmap;
};

#endif
