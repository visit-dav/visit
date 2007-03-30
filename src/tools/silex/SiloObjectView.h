#ifndef SILOOBJECTVIEW_H
#define SILOOBJECTVIEW_H

#include <qlistview.h>
#include <qmainwindow.h>
class SiloFile;

// ****************************************************************************
//  Class:  SiloObjectViewWindow
//
//  Purpose:
//    Window which views an object.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloObjectViewWindow : public QMainWindow
{
    Q_OBJECT
  public:
    SiloObjectViewWindow(SiloFile *s, const QString &n, QWidget *p);
  signals:
    void showRequested(const QString&);
  public slots:
    void ShowItem(QListViewItem *i);
  private:
    SiloFile *silo;
    QString   name;
};

// ****************************************************************************
//  Class:  SiloObjectView
//
//  Purpose:
//    ListView-style container for viewing an object.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloObjectView : public QListView
{
    Q_OBJECT
  public:
    SiloObjectView(SiloFile *s, const QString &n, QWidget *p);
    virtual QSize sizeHint() const;
  private:
    int total_items;
    SiloFile *silo;
    QString   name;
};

#endif
