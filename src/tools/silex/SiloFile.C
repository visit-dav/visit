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

#include "SiloFile.h"

#include <silo.h>

// ----------------------------------------------------------------------------
//                             Silo Dir
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  SiloDir::SiloDir
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Nov 18 12:02:13 PDT 2002
//    Ported to Windows.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for new silo objects; defvars, csgmesh/vars
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Made it able to compile with different versions of Silo
//
// ****************************************************************************
SiloDir::SiloDir(DBfile *db, const QString &name_, const QString &path_)
{
    name = name_;
    path = path_;

    DBSetDir(db, (char*)path.latin1());
    DBtoc *toc = DBGetToc(db);

    int i;
    for (i=0; i<toc->ncurve; i++)
        curve.push_back(toc->curve_names[i]);
#ifdef DBCSG_QUADRIC_G
    for (i=0; i<toc->ncsgmesh; i++)
        csgmesh.push_back(toc->csgmesh_names[i]);
    for (i=0; i<toc->ncsgvar; i++)
        csgvar.push_back(toc->csgvar_names[i]);
#endif
#ifdef DB_VARTYPE_SCALAR
    for (i=0; i<toc->ndefvars; i++)
        defvars.push_back(toc->defvars_names[i]);
#endif
    for (i=0; i<toc->nmultimesh; i++)
        multimesh.push_back(toc->multimesh_names[i]);
    for (i=0; i<toc->nmultivar; i++)
        multivar.push_back(toc->multivar_names[i]);
#ifdef DBCSG_QUADRIC_G // mmadj came into Silo same time as CSG stuff
    for (i=0; i<toc->nmultimeshadj; i++)
        multimeshadj.push_back(toc->multimeshadj_names[i]);
#endif
    for (i=0; i<toc->nmultimat; i++)
        multimat.push_back(toc->multimat_names[i]);
    for (i=0; i<toc->nmultimatspecies; i++)
        multimatspecies.push_back(toc->multimatspecies_names[i]);
    for (i=0; i<toc->nqmesh; i++)
        qmesh.push_back(toc->qmesh_names[i]);
    for (i=0; i<toc->nqvar; i++)
        qvar.push_back(toc->qvar_names[i]);
    for (i=0; i<toc->nucdmesh; i++)
        ucdmesh.push_back(toc->ucdmesh_names[i]);
    for (i=0; i<toc->nucdvar; i++)
        ucdvar.push_back(toc->ucdvar_names[i]);
    for (i=0; i<toc->nptmesh; i++)
        ptmesh.push_back(toc->ptmesh_names[i]);
    for (i=0; i<toc->nptvar; i++)
        ptvar.push_back(toc->ptvar_names[i]);
    for (i=0; i<toc->nmat; i++)
        mat.push_back(toc->mat_names[i]);
    for (i=0; i<toc->nmatspecies; i++)
        matspecies.push_back(toc->matspecies_names[i]);
    for (i=0; i<toc->nvar; i++)
        var.push_back(toc->var_names[i]);
    for (i=0; i<toc->nobj; i++)
        obj.push_back(toc->obj_names[i]);
    for (i=0; i<toc->narrays; i++)
        array.push_back(toc->array_names[i]);
    for (i=0; i<toc->ndir; i++)
        dir.push_back(toc->dir_names[i]);

    for (i=0; i<dir.size(); i++)
    {
        if (name == "/")
            subdir.push_back(new SiloDir(db, dir[i], path + dir[i]));
        else
            subdir.push_back(new SiloDir(db, dir[i], path + "/" + dir[i]));
    }
}

// ****************************************************************************
//  Destructor:  SiloDir::~SiloDir
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloDir::~SiloDir()
{
    for (int i=0; i<subdir.size(); i++)
    {
        delete subdir[i];
    }
}




// ----------------------------------------------------------------------------
//                              Silo File
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Constructor:  SiloFile::SiloFile
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloFile::SiloFile(const QString &name)
{
    db = DBOpen((char*)name.latin1(), DB_UNKNOWN, DB_READ);
    if (!db)
        throw "Could not open silo file";

    root = new SiloDir(db, "/", "/");
}


// ****************************************************************************
//  Destructor:  SiloFile::~SiloFile
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
SiloFile::~SiloFile()
{
    DBClose(db);
}


// ****************************************************************************
//  Methods:     Silo API Functions
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************

void*
SiloFile::GetVar(const QString &name)
{
    return DBGetVar(db, (char*)name.latin1());
}

int
SiloFile::GetVarType(const QString &name)
{
    return DBGetVarType(db, (char*)name.latin1());
}

int
SiloFile::GetVarLength(const QString &name)
{
    return DBGetVarLength(db, (char*)name.latin1());
}

DBobject*
SiloFile::GetObject(const QString &name)
{
    return DBGetObject(db, (char*)name.latin1());
}

void*
SiloFile::GetComponent(const QString &oname, const QString &cname)
{
    return DBGetComponent(db, (char*)oname.latin1(), (char*)cname.latin1());
}

int
SiloFile::GetComponentType(const QString &oname, const QString &cname)
{
    return DBGetComponentType(db, (char*)oname.latin1(), (char*)cname.latin1());
}

DBObjectType
SiloFile::InqVarType(const QString &name)
{
    return DBInqVarType(db, (char*)name.latin1());
}
