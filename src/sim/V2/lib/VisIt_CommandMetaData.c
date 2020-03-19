// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_CommandMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CommandMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_CommandMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CommandMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_CommandMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(CommandMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_CommandMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(CommandMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_CommandMetaData_setEnabled(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(CommandMetaData_setEnabled,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_CommandMetaData_getEnabled(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(CommandMetaData_getEnabled,
        int, (visit_handle, int*),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDCMDALLOC                 F77_ID(visitmdcmdalloc_,visitmdcmdalloc,VISITMDCMDALLOC)
#define F_VISITMDCMDFREE                  F77_ID(visitmdcmdfree_,visitmdcmdfree,VISITMDCMDFREE)
#define F_VISITMDCMDSETNAME               F77_ID(visitmdcmdsetname_,visitmdcmdsetname, VISITMDCMDSETNAME)
#define F_VISITMDCMDGETNAME               F77_ID(visitmdcmdgetname_,visitmdcmdgetname, VISITMDCMDGETNAME)
#define F_VISITMDCMDSETENABLED            F77_ID(visitmdcmdsetenabled_,visitmdcmdsetenabled, VISITMDCMDSETENABLED)
#define F_VISITMDCMDGETENABLED            F77_ID(visitmdcmdgetenabled_,visitmdcmdgetenabled, VISITMDCMDGETENABLED)

int
F_VISITMDCMDALLOC(visit_handle *h)
{
    return VisIt_CommandMetaData_alloc(h);
}

int
F_VISITMDCMDFREE(visit_handle *h)
{
    return VisIt_CommandMetaData_free(*h);
}

int
F_VISITMDCMDSETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_CommandMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDCMDGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_CommandMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDCMDSETENABLED(visit_handle *h, int *val)
{
    return VisIt_CommandMetaData_setEnabled(*h, *val);
}

int
F_VISITMDCMDGETENABLED(visit_handle *h, int *val)
{
    return VisIt_CommandMetaData_getEnabled(*h, val);
}

