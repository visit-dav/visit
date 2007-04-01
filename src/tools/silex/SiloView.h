#ifndef SILOVIEW_H
#define SILOVIEW_H

#include <qlistview.h>
#include <qsplitter.h>
#include <qmainwindow.h>
class SiloFile;

class SiloDirTreeView;
class SiloDirView;

// ****************************************************************************
//  Class:  SiloView
//
//  Purpose:
//    A combination of a directory tree and a TOC viewer, which can also
//    launch windows to view objects, arrays, and variables.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloView : public QSplitter
{
    Q_OBJECT
  public:
    SiloView(const QString &file, QWidget *p, const QString &n);
    virtual ~SiloView();
    void Set(const QString &file);

  public slots:
   void ShowObject(const QString &name);
   void ShowVariable(const QString &name);
   void ShowUnknown(const QString &name);
   void ShowItem(QListViewItem *i);
   void SetDir(QListViewItem *i);

  private:
    SiloFile *silo;
    SiloDirTreeView *dirTreeView;
    SiloDirView     *dirView;
};

#endif
