/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "SiloObjectView.h"
#include <SiloFile.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <visitstream.h>

#include <cstdlib>

// ----------------------------------------------------------------------------
//                            Object View Window
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor:  SiloObjectViewWindow::SiloObjectViewWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloObjectViewWindow::SiloObjectViewWindow(SiloFile *s, const QString &n, QWidget *p)
    : QMainWindow(p, n), silo(s), name(n)
{
    setCaption(QString("Object: ")+name);

    SiloObjectView *ov = new SiloObjectView(silo,name,this);
    setCentralWidget(ov);
    connect(ov,   SIGNAL(doubleClicked(QListViewItem*)),
            this, SLOT(ShowItem(QListViewItem*)));
}

// ****************************************************************************
//  Method:  SiloObjectViewWindow::ShowItem
//
//  Purpose:
//    A slot used to signal a "show var" event.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloObjectViewWindow::ShowItem(QListViewItem *i)
{
    if (i->text(1) == "var")
        emit showRequested(i->text(2));
}

// ----------------------------------------------------------------------------
//                               Object View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor: SiloObjectView::SiloObjectView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon May 17 12:37:32 PDT 2004
//    Added a couple calls to free memory and prevent really big leaks.
//
//    Mark C. Miller, Wed Apr 20 17:08:36 PDT 2005
//    Added code to deal with hdf5 formatted strings in pdbname
//
//    Jeremy Meredith, Wed May 11 12:42:12 PDT 2005
//    Show an error message if we get an invalid object.
//
// ****************************************************************************
SiloObjectView::SiloObjectView(SiloFile *s, const QString &n, QWidget *p)
    : QListView(p, n), silo(s), name(n)
{
    setSorting(-1);
    setAllColumnsShowFocus(true);
    addColumn("Component");
    addColumn("Type");
    addColumn("Value");

    DBobject *object = silo->GetObject(name);
    if (!object)
    {
        QMessageBox::warning(this, "silex", "Could not read this object.\n"
           "The file may have been written using an incomplete driver.", "OK");
        return;
    }

    QListViewItem *lastItem = NULL;
    for (int i=0; i<object->ncomponents; i++)
    {
        QString compname = object->comp_names[i];
        QString pdbname  = object->pdb_names[i];
        void *comp = silo->GetComponent(name, compname);
        if (!comp)
        {
            cerr << "ERROR: DBGetComponent failed for object '"<<name<<"', component '"<<compname<<"'"<<endl;
            continue;
        }
        int   type = silo->GetComponentType(name, compname);
        QString typestr = "";
        char  value[256] = "";
        int ival = -1;
        switch (type)
        {
          case DB_INT:
            typestr = "int";
            sprintf(value, "%d", *((int*)comp));
            ival = *((int*)comp);
            break;
          case DB_SHORT:
            typestr = "short";
            sprintf(value, "%d", *((short*)comp));
            ival = *((short*)comp);
            break;
          case DB_LONG:
            typestr = "long";
            sprintf(value, "%ld", *((long*)comp));
            ival = *((long*)comp);
            break;
          case DB_FLOAT:
            typestr = "float";
            sprintf(value, "%g", *((float*)comp));
            break;
          case DB_DOUBLE:
            typestr = "double";
            sprintf(value, "%g", *((double*)comp));
            break;
          case DB_CHAR:
            typestr = "char";
            sprintf(value, "%s", ((char*)comp));
            break;
          case DB_NOTYPE:
            typestr = "notype";
            sprintf(value, "NOTYPE");
            break;
          default:
            typestr = "var";
            std::string valStr = pdbname.latin1();
            if (pdbname.find("'<s>") == 0)
            {
                int len = pdbname.length();
                valStr = std::string(pdbname.latin1(),4,len-5);
            }
            sprintf(value, "%s", valStr.c_str());
            break;
        }

        // No such call as "DBFreeComponent".  Maybe there should be one!
        free(comp);
        comp = NULL;

        if (type==DB_INT || type==DB_SHORT || type==DB_LONG)
        {
            if (compname == "coordtype")
            {
                if (ival == DB_COLLINEAR)    strcat(value, " (DB_COLLINEAR)");
                if (ival == DB_NONCOLLINEAR) strcat(value, " (DB_NONCOLLINEAR)");
            }
            if (compname == "centering")
            {
                if (ival == DB_NOTCENT)      strcat(value, " (DB_NOTCENT)");
                if (ival == DB_NODECENT)     strcat(value, " (DB_NODECENT)");
                if (ival == DB_ZONECENT)     strcat(value, " (DB_ZONECENT)");
                if (ival == DB_FACECENT)     strcat(value, " (DB_FACECENT)");
            }
            if (compname == "major_order")
            {
                if (ival == DB_ROWMAJOR)     strcat(value, " (DB_ROWMAJOR)");
                if (ival == DB_COLMAJOR)     strcat(value, " (DB_COLMAJOR)");
            }
            if (compname == "coord_sys")
            {
                if (ival == DB_CARTESIAN)    strcat(value, " (DB_CARTESIAN)");
                if (ival == DB_CYLINDRICAL)  strcat(value, " (DB_CYLINDRICAL)");
                if (ival == DB_SPHERICAL)    strcat(value, " (DB_SPHERICAL)");
                if (ival == DB_NUMERICAL)    strcat(value, " (DB_NUMERICAL)");
                if (ival == DB_OTHER)        strcat(value, " (DB_OTHER)");
            }
            if (compname == "planar")
            {
                if (ival == DB_AREA)         strcat(value, " (DB_AREA)");
                if (ival == DB_VOLUME)       strcat(value, " (DB_VOLUME)");
            }
            if (compname == "facetype")
            {
                if (ival == DB_RECTILINEAR)  strcat(value, " (DB_RECTILINEAR)");
                if (ival == DB_CURVILINEAR)  strcat(value, " (DB_CURVILINEAR)");
            }
            if (compname == "datatype")
            {
                if (ival == DB_INT)          strcat(value, " (DB_INT)");
                if (ival == DB_SHORT)        strcat(value, " (DB_SHORT)");
                if (ival == DB_LONG)         strcat(value, " (DB_LONG)");
                if (ival == DB_FLOAT)        strcat(value, " (DB_FLOAT)");
                if (ival == DB_DOUBLE)       strcat(value, " (DB_DOUBLE)");
                if (ival == DB_CHAR)         strcat(value, " (DB_CHAR)");
                if (ival == DB_NOTYPE)       strcat(value, " (DB_NOTYPE)");
            }
        }
        if (lastItem)
            lastItem = new QListViewItem(this,
                                         lastItem,
                                         object->comp_names[i],
                                         typestr,
                                         value);
        else
            lastItem = new QListViewItem(this,
                                         object->comp_names[i],
                                         typestr,
                                         value);
    }

    total_items = object->ncomponents;
    DBFreeObject(object);
}

// ****************************************************************************
//  Method:  SiloObjectView::sizeHint
//
//  Purpose:
//    Suggest a good size for the view.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
QSize
SiloObjectView::sizeHint() const
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


