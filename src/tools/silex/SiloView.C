#include "SiloView.h"
#include <SiloFile.h>
#include <SiloDirTreeView.h>
#include <SiloDirView.h>
#include <SiloValueView.h>
#include <SiloArrayView.h>
#include <SiloObjectView.h>
#include <qheader.h>
#include <qmessagebox.h>

// ----------------------------------------------------------------------------
//                            Silo View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor: SiloView::SiloView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Sep 24 11:19:27 PDT 2003
//    I added a call to set the root directory in the directory-contents
//    panel.  This was unnecessary in Qt versions before 3.x.
//
// ****************************************************************************
SiloView::SiloView(const QString &file, QWidget *p, const QString &n)
    : QSplitter(p, n)
{
    silo        = new SiloFile(file);
    dirTreeView = new SiloDirTreeView(silo, this, "DirTreeView");
    dirView     = new SiloDirView(this, "DirView");

    connect(dirTreeView, SIGNAL(currentChanged(QListViewItem*)),
            this,        SLOT(SetDir(QListViewItem*)));
    connect(dirView,     SIGNAL(doubleClicked(QListViewItem*)),
            this,        SLOT(ShowItem(QListViewItem*)));

    dirTreeView->OpenRootDir();
    dirView->Set(silo->root);

    dirTreeView->header()->setResizeEnabled(false);
    dirView->header()->setResizeEnabled(false);
}

// ****************************************************************************
//  Destructor:  SiloView::~SiloView
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 17, 2004
//
// ****************************************************************************
SiloView::~SiloView()
{
    delete silo;
}

// ****************************************************************************
//  Method:  SiloView::Set
//
//  Purpose:
//    Replace with a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 12 20:53:28 PDT 2004
//    Explicitly set the dirView directory.  Newer Qt versions don't seem to 
//    do it automatically by setting the first child (root dir) as selcted
//    inside SiloDirTreeView.
//
// ****************************************************************************
void
SiloView::Set(const QString &file)
{
    delete silo;
    silo = new SiloFile(file);
    dirTreeView->Set(silo);
    dirTreeView->OpenRootDir();
    dirView->ChangeDir(dirTreeView->firstChild());
}

// ****************************************************************************
//  Method:  SiloView::SetDir
//
//  Purpose:
//    Tell the current SiloDirView directories.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloView::SetDir(QListViewItem *i)
{
    dirView->ChangeDir(i);
}

// ****************************************************************************
//  Method:  SiloView::ShowVariable
//
//  Purpose:
//    Pop up a new window to show a variable or simple array.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloView::ShowVariable(const QString &name)
{
    int len = silo->GetVarLength(name);
    if (len == 1)
    {
        SiloValueViewWindow *vv = new SiloValueViewWindow(silo, name, NULL);
        vv->show();
    }
    else
    {
        SiloArrayViewWindow *av = new SiloArrayViewWindow(silo, name, NULL);
        av->show();
        av->resize(av->sizeHint());
    }
}

// ****************************************************************************
//  Method:  SiloView::ShowObject
//
//  Purpose:
//    Pop up a new window to show a compound object.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloView::ShowObject(const QString &name)
{
    SiloObjectViewWindow *ov = new SiloObjectViewWindow(silo, name, NULL);
    connect(ov,   SIGNAL(showRequested(const QString&)),
            this, SLOT(ShowUnknown(const QString&)));
    ov->show();
}

// ****************************************************************************
//  Method:  SiloView::ShowUnknown
//
//  Purpose:
//    Pop up a new window to show an item based on its type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed May 11 12:42:12 PDT 2005
//    Show an error message if we get an invalid object.
//
// ****************************************************************************
void
SiloView::ShowUnknown(const QString &name)
{
    DBObjectType type = silo->InqVarType(name);

    if (type == DB_INVALID_OBJECT)
    {
        QMessageBox::warning(this, "silex",
           "Could not determine the type of this object.\n"
           "The file may have been written using an incomplete driver.", "OK");
        return;
    }


    bool isObject = (type != DB_VARIABLE);

    if (isObject)
        ShowObject(name);
    else
        ShowVariable(name);
}

// ****************************************************************************
//  Method:  SiloView::ShowItem
//
//  Purpose:
//    Wrapper for ShowUnknown appropriate for a QListViewItem callback.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloView::ShowItem(QListViewItem *i)
{
    SiloDirViewItem *item = (SiloDirViewItem*)i;
    if (!item->dir)
        return;

    QString name = item->dir->path;
    if (name != "/")
        name += "/";
    name += item->name;

    ShowUnknown(name);
}
