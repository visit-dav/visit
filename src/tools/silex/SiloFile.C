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
    for (i=0; i<toc->nmultimesh; i++)
        multimesh.push_back(toc->multimesh_names[i]);
    for (i=0; i<toc->nmultivar; i++)
        multivar.push_back(toc->multivar_names[i]);
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
