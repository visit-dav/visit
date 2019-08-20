// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_MessageMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_MessageMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_MessageMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MessageMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMESSAGEMETADATAALLOC       F77_ID(visitmessagemetadataalloc_,visitmessagemetadataalloc,VISITMESSAGEMETADATAALLOC)
#define F_VISITMESSAGEMETADATAFREE        F77_ID(visitmessagemetadatafree_,visitmessagemetadatafree,VISITMESSAGEMETADATAFREE)
#define F_VISITMESSAGEMETADATASETNAME     F77_ID(visitmessagemetadatasetname_,visitmessagemetadatasetname, VISITMESSAGEMETADATASETNAME)
#define F_VISITMESSAGEMETADATAGETNAME     F77_ID(visitmessagemetadatagetname_,visitmessagemetadatagetname, VISITMESSAGEMETADATAGETNAME)

int
F_VISITMESSAGEMETADATAALLOC(visit_handle *h)
{
    return VisIt_MessageMetaData_alloc(h);
}

int
F_VISITMESSAGEMETADATAFREE(visit_handle *h)
{
    return VisIt_MessageMetaData_free(*h);
}

int
F_VISITMESSAGEMETADATASETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MessageMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMESSAGEMETADATAGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MessageMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

