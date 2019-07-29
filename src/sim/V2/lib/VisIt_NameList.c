// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_NameList_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(NameList_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_NameList_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(NameList_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_NameList_addName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(NameList_addName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_NameList_getNumName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(NameList_getNumName,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_NameList_getName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(NameList_getName,
        int, (visit_handle, int, char **),
        (h, i, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITNAMELISTALLOC              F77_ID(visitnamelistalloc_,visitnamelistalloc,VISITNAMELISTALLOC)
#define F_VISITNAMELISTFREE               F77_ID(visitnamelistfree_,visitnamelistfree,VISITNAMELISTFREE)
#define F_VISITNAMELISTADDNAME            F77_ID(visitnamelistaddname_,visitnamelistaddname, VISITNAMELISTADDNAME)
#define F_VISITNAMELISTGETNUMNAME         F77_ID(visitnamelistgetnumname_,visitnamelistgetnumname, VISITNAMELISTGETNUMNAME)
#define F_VISITNAMELISTGETNAME            F77_ID(visitnamelistgetname_,visitnamelistgetname, VISITNAMELISTGETNAME)

int
F_VISITNAMELISTALLOC(visit_handle *h)
{
    return VisIt_NameList_alloc(h);
}

int
F_VISITNAMELISTFREE(visit_handle *h)
{
    return VisIt_NameList_free(*h);
}

int
F_VISITNAMELISTADDNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_NameList_addName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITNAMELISTGETNUMNAME(visit_handle *h, int *val)
{
    return VisIt_NameList_getNumName(*h, val);
}

int
F_VISITNAMELISTGETNAME(visit_handle *h, int *i, char *val, int *lval)
{
    char *s = NULL;
    int retval;
    retval = VisIt_NameList_getName(*h, *i, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

