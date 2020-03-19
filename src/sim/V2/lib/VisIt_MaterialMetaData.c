// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_MaterialMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_MaterialMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_MaterialMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MaterialMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MaterialMetaData_setMeshName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_setMeshName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MaterialMetaData_getMeshName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getMeshName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MaterialMetaData_addMaterialName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_addMaterialName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MaterialMetaData_getNumMaterialNames(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getNumMaterialNames,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_MaterialMetaData_getMaterialName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getMaterialName,
        int, (visit_handle, int, char **),
        (h, i, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDMATALLOC                 F77_ID(visitmdmatalloc_,visitmdmatalloc,VISITMDMATALLOC)
#define F_VISITMDMATFREE                  F77_ID(visitmdmatfree_,visitmdmatfree,VISITMDMATFREE)
#define F_VISITMDMATSETNAME               F77_ID(visitmdmatsetname_,visitmdmatsetname, VISITMDMATSETNAME)
#define F_VISITMDMATGETNAME               F77_ID(visitmdmatgetname_,visitmdmatgetname, VISITMDMATGETNAME)
#define F_VISITMDMATSETMESHNAME           F77_ID(visitmdmatsetmeshname_,visitmdmatsetmeshname, VISITMDMATSETMESHNAME)
#define F_VISITMDMATGETMESHNAME           F77_ID(visitmdmatgetmeshname_,visitmdmatgetmeshname, VISITMDMATGETMESHNAME)
#define F_VISITMDMATADDMATERIALNAME       F77_ID(visitmdmataddmaterialname_,visitmdmataddmaterialname, VISITMDMATADDMATERIALNAME)
#define F_VISITMDMATGETNUMMATERIALNAMES    F77_ID(visitmdmatgetnummaterialnames_,visitmdmatgetnummaterialnames, VISITMDMATGETNUMMATERIALNAMES)
#define F_VISITMDMATGETMATERIALNAME       F77_ID(visitmdmatgetmaterialname_,visitmdmatgetmaterialname, VISITMDMATGETMATERIALNAME)

int
F_VISITMDMATALLOC(visit_handle *h)
{
    return VisIt_MaterialMetaData_alloc(h);
}

int
F_VISITMDMATFREE(visit_handle *h)
{
    return VisIt_MaterialMetaData_free(*h);
}

int
F_VISITMDMATSETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MaterialMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMATGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MaterialMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMATSETMESHNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MaterialMetaData_setMeshName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMATGETMESHNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MaterialMetaData_getMeshName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMATADDMATERIALNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MaterialMetaData_addMaterialName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMATGETNUMMATERIALNAMES(visit_handle *h, int *val)
{
    return VisIt_MaterialMetaData_getNumMaterialNames(*h, val);
}

int
F_VISITMDMATGETMATERIALNAME(visit_handle *h, int *i, char *val, int *lval)
{
    char *s = NULL;
    int retval;
    retval = VisIt_MaterialMetaData_getMaterialName(*h, *i, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

