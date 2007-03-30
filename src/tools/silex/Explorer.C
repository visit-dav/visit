#include "Explorer.h"
#include <SiloView.h>

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qapplication.h>

// ****************************************************************************
//  Constructor: Explorer::Explorer
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
Explorer::Explorer(const QString &file, QWidget *p, const QString &n)
    : QMainWindow(p,n)
{
    view = new SiloView(file,
                        this, "SiloView");
    setCentralWidget(view);
    setCaption("Explorer: "+file);

    QPopupMenu *filemenu = new QPopupMenu( this );
    menuBar()->insertItem(tr("&File"),filemenu);
    filemenu->insertItem( "&Open",  this, SLOT(open()),  CTRL+Key_O );
    filemenu->insertSeparator();
    filemenu->insertItem( "E&xit", this, SLOT(close()),  CTRL+Key_X );
}

// ****************************************************************************
//  Method:  Explorer::open
//
//  Purpose:
//    Open a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
Explorer::open()
{
    QString file = 
        QFileDialog::getOpenFileName(QString(),
                                     "Silo files (*.silo *.root *.pdb);;"
                                     "All files (*)",
                                     NULL, "SiloOpen", "Open file...");
    if (file.isNull())
        return;
    view->Set(file);
}
