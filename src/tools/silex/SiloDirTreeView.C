/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "SiloDirTreeView.h"
#include <SiloFile.h>
#include <qpixmap.h>
#include <qapplication.h>

#include "folder.xpm"

// ****************************************************************************
//  Constructor:  SiloDirTreeView::SiloDirTreeView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloDirTreeView::SiloDirTreeView(SiloFile *s, QWidget *p, const QString &n)
    : QListView(p,n), silo(s)
{
    folder_pixmap = new QPixmap(folder_xpm);
 
    addColumn("Contents");
    //setRootIsDecorated(true);

    total_items = 0;
    QListViewItem *root = AddDir(silo->root, NULL);
    root->setOpen(true);
}

// ****************************************************************************
//  Method:  SiloDirTreeView::AddDir
//
//  Purpose:
//    Add a new directory to the DirTree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
QListViewItem *
SiloDirTreeView::AddDir(SiloDir *d, QListViewItem *parent)
{
    total_items++;

    QListViewItem *item;
    if (!parent)
        item = new SiloDirTreeViewItem(d, this, d->name);
    else
        item = new SiloDirTreeViewItem(d, parent, d->name);

    item->setPixmap(0,*folder_pixmap);
    for (int i=0; i<d->subdir.size(); i++)
        AddDir(d->subdir[i], item);

    return item;
}

// ****************************************************************************
//  Method:  SiloDirTreeView::Set
//
//  Purpose:
//    Reset the view to a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 12 20:52:16 PDT 2004
//    Set the internal silo file to the new file.
//
// ****************************************************************************
void
SiloDirTreeView::Set(SiloFile *s)
{
    silo = s;
    clear();
    total_items = 0;
    QListViewItem *root = AddDir(silo->root, NULL);
    root->setOpen(true);
}

// ****************************************************************************
//  Method:  SiloDirTreeView::OpenRootDir
//
//  Purpose:
//    Select the root directory of the tree.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloDirTreeView::OpenRootDir()
{
    setSelected(firstChild(), true);
}

// ****************************************************************************
//  Method:  SiloDirTreeView::resizeEvent
//
//  Purpose:
//    Make the column header fill the width of the listview.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloDirTreeView::resizeEvent(QResizeEvent *re)
{
    QListView::resizeEvent(re);
    setColumnWidth(0, width() - 4);
}

// ****************************************************************************
//  Method:  SiloDirTreeView::sizeHint
//
//  Purpose:
//    Suggest a good size for the view.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
QSize
SiloDirTreeView::sizeHint() const
{
    QSize size = QListView::sizeHint();
    if (total_items == 0 || firstChild() == 0)
        return size;

    size.setHeight(QMIN(QMAX(size.height(),
                             firstChild()->height() * (total_items+2)),
                        QApplication::desktop()->height() * 7/8));
    if (!size.isValid())
        size.setWidth(200);

    return size;
}

