/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include "SiloDirView.h"
#include <SiloDirTreeView.h>
#include <SiloFile.h>
#include <qpixmap.h>
#include <qapplication.h>

#include "mesh.xpm"
#include "mat.xpm"
#include "var.xpm"
#include "object.xpm"
#include "species.xpm"
#include "curve.xpm"
#include "array.xpm"
#include "silovar.xpm"


// ****************************************************************************
//  Constructor:  SiloDirView::SiloDirView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloDirView::SiloDirView(QWidget *p, const QString &n)
    : QListView(p,n)
{
    total_items = 0;

    mesh_pixmap   = new QPixmap(mesh_xpm);
    mat_pixmap    = new QPixmap(mat_xpm);
    var_pixmap    = new QPixmap(var_xpm);
    obj_pixmap    = new QPixmap(object_xpm);
    spec_pixmap   = new QPixmap(species_xpm);
    curve_pixmap  = new QPixmap(curve_xpm);
    array_pixmap  = new QPixmap(array_xpm);
    silovar_pixmap= new QPixmap(silovar_xpm);

    addColumn("Objects");
    setRootIsDecorated(true);
}

// ****************************************************************************
//  Method:  SiloDirView::Set
//
//  Purpose:
//    View the TOC of a new directory.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Jul 12 16:57:37 PDT 2002
//    Split the total_items calculation into multiple lines.  g++-2.96
//    was choking on it for some odd reason.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for new silo objects; defvars, csgmesh/vars
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added multimesh-adjacency object support 
// ****************************************************************************
void
SiloDirView::Set(SiloDir *d)
{
    clear();

    total_items  = d->array.size()           + (d->array.size()           ? 1:0);
    total_items += d->obj.size()             + (d->obj.size()             ? 1:0);
    total_items += d->var.size()             + (d->var.size()             ? 1:0);
    total_items += d->defvars.size()         + (d->defvars.size()         ? 1:0);
    total_items += d->matspecies.size()      + (d->matspecies.size()      ? 1:0);
    total_items += d->mat.size()             + (d->mat.size()             ? 1:0);
    total_items += d->ptvar.size()           + (d->ptvar.size()           ? 1:0);
    total_items += d->ptmesh.size()          + (d->ptmesh.size()          ? 1:0);
    total_items += d->ucdvar.size()          + (d->ucdvar.size()          ? 1:0);
    total_items += d->ucdmesh.size()         + (d->ucdmesh.size()         ? 1:0);
    total_items += d->qvar.size()            + (d->qvar.size()            ? 1:0);
    total_items += d->qmesh.size()           + (d->qmesh.size()           ? 1:0);
    total_items += d->multimatspecies.size() + (d->multimatspecies.size() ? 1:0);
    total_items += d->multimat.size()        + (d->multimat.size()        ? 1:0);
    total_items += d->multivar.size()        + (d->multivar.size()        ? 1:0);
    total_items += d->multimesh.size()       + (d->multimesh.size()       ? 1:0);
    total_items += d->multimeshadj.size()    + (d->multimeshadj.size()    ? 1:0);
    total_items += d->curve.size()           + (d->curve.size()           ? 1:0);
    total_items += d->csgvar.size()          + (d->csgvar.size()          ? 1:0);
    total_items += d->csgmesh.size()         + (d->csgmesh.size()         ? 1:0);

    bool expandVars = true;
    if (d->curve.size())
    {
        SiloDirViewItem *curve = new SiloDirViewItem(NULL,this, "Curves");
        curve->setPixmap(0, *curve_pixmap);
        for (int i=0; i<d->curve.size(); i++)
            new SiloDirViewItem(d,curve, d->curve[i]);
        curve->setOpen(true);
    }

    if (d->multimesh.size())
    {
        SiloDirViewItem *multimesh = new SiloDirViewItem(NULL,this, "MultiMeshes");
        multimesh->setPixmap(0, *mesh_pixmap);
        for (int i=0; i<d->multimesh.size(); i++)
            new SiloDirViewItem(d,multimesh, d->multimesh[i]);
        multimesh->setOpen(true);
        expandVars = false;
    }

    if (d->multivar.size())
    {
        SiloDirViewItem *multivar = new SiloDirViewItem(NULL,this, "MultiVars");
        multivar->setPixmap(0, *var_pixmap);
        for (int i=0; i<d->multivar.size(); i++)
            new SiloDirViewItem(d,multivar, d->multivar[i]);
        multivar->setOpen(true);
        expandVars = false;
    }

    if (d->multimeshadj.size())
    {
        SiloDirViewItem *multimeshadj = new SiloDirViewItem(NULL,this, "MultiMesheadjs");
        multimeshadj->setPixmap(0, *mesh_pixmap);
        for (int i=0; i<d->multimeshadj.size(); i++)
            new SiloDirViewItem(d,multimeshadj, d->multimeshadj[i]);
        multimeshadj->setOpen(true);
        expandVars = false;
    }


    if (d->multimat.size())
    {
        SiloDirViewItem *multimat = new SiloDirViewItem(NULL,this, "MultiMats");
        multimat->setPixmap(0, *mat_pixmap);
        for (int i=0; i<d->multimat.size(); i++)
            new SiloDirViewItem(d,multimat, d->multimat[i]);
        multimat->setOpen(true);
        expandVars = false;
    }

    if (d->multimatspecies.size())
    {
        SiloDirViewItem *multimatspecies = new SiloDirViewItem(NULL,this, "MultiSpecies");
        multimatspecies->setPixmap(0, *spec_pixmap);
        for (int i=0; i<d->multimatspecies.size(); i++)
            new SiloDirViewItem(d,multimatspecies, d->multimatspecies[i]);
        multimatspecies->setOpen(true);
        expandVars = false;
    }

    if (d->qmesh.size())
    {
        SiloDirViewItem *qmesh = new SiloDirViewItem(NULL,this, "QuadMeshes");
        qmesh->setPixmap(0, *mesh_pixmap);
        for (int i=0; i<d->qmesh.size(); i++)
            new SiloDirViewItem(d,qmesh, d->qmesh[i]);
        qmesh->setOpen(true);
        expandVars = false;
    }

    if (d->qvar.size())
    {
        SiloDirViewItem *qvar = new SiloDirViewItem(NULL,this, "QuadVars");
        qvar->setPixmap(0, *var_pixmap);
        for (int i=0; i<d->qvar.size(); i++)
            new SiloDirViewItem(d,qvar, d->qvar[i]);
        qvar->setOpen(true);
        expandVars = false;
    }

    if (d->ucdmesh.size())
    {
        SiloDirViewItem *ucdmesh = new SiloDirViewItem(NULL,this, "UCDMeshes");
        ucdmesh->setPixmap(0, *mesh_pixmap);
        for (int i=0; i<d->ucdmesh.size(); i++)
            new SiloDirViewItem(d,ucdmesh, d->ucdmesh[i]);
        ucdmesh->setOpen(true);
        expandVars = false;
    }

    if (d->ucdvar.size())
    {
        SiloDirViewItem *ucdvar = new SiloDirViewItem(NULL,this, "UCDVars");
        ucdvar->setPixmap(0, *var_pixmap);
        for (int i=0; i<d->ucdvar.size(); i++)
            new SiloDirViewItem(d,ucdvar, d->ucdvar[i]);
        ucdvar->setOpen(true);
        expandVars = false;
    }

    if (d->ptmesh.size())
    {
        SiloDirViewItem *ptmesh = new SiloDirViewItem(NULL,this, "PointMeshes");
        ptmesh->setPixmap(0, *mesh_pixmap);
        for (int i=0; i<d->ptmesh.size(); i++)
            new SiloDirViewItem(d,ptmesh, d->ptmesh[i]);
        ptmesh->setOpen(true);
        expandVars = false;
    }

    if (d->ptvar.size())
    {
        SiloDirViewItem *ptvar = new SiloDirViewItem(NULL,this, "PointVars");
        ptvar->setPixmap(0, *var_pixmap);
        for (int i=0; i<d->ptvar.size(); i++)
            new SiloDirViewItem(d,ptvar, d->ptvar[i]);
        ptvar->setOpen(true);
        expandVars = false;
    }

    if (d->csgmesh.size())
    {
        SiloDirViewItem *csgmesh = new SiloDirViewItem(NULL,this, "CSGMeshes");
        csgmesh->setPixmap(0, *mesh_pixmap);
        for (int i=0; i<d->csgmesh.size(); i++)
            new SiloDirViewItem(d,csgmesh, d->csgmesh[i]);
        csgmesh->setOpen(true);
        expandVars = false;
    }

    if (d->csgvar.size())
    {
        SiloDirViewItem *csgvar = new SiloDirViewItem(NULL,this, "CSGVars");
        csgvar->setPixmap(0, *var_pixmap);
        for (int i=0; i<d->csgvar.size(); i++)
            new SiloDirViewItem(d,csgvar, d->csgvar[i]);
        csgvar->setOpen(true);
        expandVars = false;
    }

    if (d->mat.size())
    {
        SiloDirViewItem *mat = new SiloDirViewItem(NULL,this, "Materials");
        mat->setPixmap(0, *mat_pixmap);
        for (int i=0; i<d->mat.size(); i++)
            new SiloDirViewItem(d,mat, d->mat[i]);
        mat->setOpen(true);
        expandVars = false;
    }

    if (d->matspecies.size())
    {
        SiloDirViewItem *matspecies = new SiloDirViewItem(NULL,this, "Species");
        matspecies->setPixmap(0, *spec_pixmap);
        for (int i=0; i<d->matspecies.size(); i++)
            new SiloDirViewItem(d,matspecies, d->matspecies[i]);
        matspecies->setOpen(true);
        expandVars = false;
    }

    if (d->obj.size())
    {
        SiloDirViewItem *obj = new SiloDirViewItem(NULL,this, "Objects");
        obj->setPixmap(0, *obj_pixmap);
        for (int i=0; i<d->obj.size(); i++)
            new SiloDirViewItem(d,obj, d->obj[i]);
        obj->setOpen(true);
    }

    if (d->array.size())
    {
        SiloDirViewItem *array = new SiloDirViewItem(NULL,this, "Arrays");
        array->setPixmap(0, *array_pixmap);
        for (int i=0; i<d->array.size(); i++)
            new SiloDirViewItem(d,array, d->array[i]);
        array->setOpen(true);
    }

    if (d->var.size())
    {
        SiloDirViewItem *var = new SiloDirViewItem(NULL,this, "Vars");
        var->setPixmap(0, *silovar_pixmap);
        for (int i=0; i<d->var.size(); i++)
            new SiloDirViewItem(d,var, d->var[i]);
        if (expandVars)
            var->setOpen(true);
    }
}

// ****************************************************************************
//  Method:  SiloDirView::ChangeDir
//
//  Purpose:
//    Wrapper for "Set" which is suitable for a QListViewItem callback.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloDirView::ChangeDir(QListViewItem *i)
{
    SiloDirTreeViewItem *item = (SiloDirTreeViewItem*)i;
    setColumnText(0, QString("Contents of ") + item->dir->path);
    Set(item->dir);
}

// ****************************************************************************
//  Method:  SiloDirView::resizeEvent
//
//  Purpose:
//    Make the column header fill the width of the listview.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloDirView::resizeEvent(QResizeEvent *re)
{
    QListView::resizeEvent(re);
    setColumnWidth(0, width() - 4);
}

// ****************************************************************************
//  Method:  SiloDirView::sizeHint
//
//  Purpose:
//    Suggest a good size for the view.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
QSize
SiloDirView::sizeHint() const
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
