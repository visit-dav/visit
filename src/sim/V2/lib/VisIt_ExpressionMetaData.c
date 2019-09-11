// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_ExpressionMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_ExpressionMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_ExpressionMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_ExpressionMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_ExpressionMetaData_setDefinition(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_setDefinition,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_ExpressionMetaData_getDefinition(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_getDefinition,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_ExpressionMetaData_setType(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_setType,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_ExpressionMetaData_getType(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(ExpressionMetaData_getType,
        int, (visit_handle, int*),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDEXPRALLOC                F77_ID(visitmdexpralloc_,visitmdexpralloc,VISITMDEXPRALLOC)
#define F_VISITMDEXPRFREE                 F77_ID(visitmdexprfree_,visitmdexprfree,VISITMDEXPRFREE)
#define F_VISITMDEXPRSETNAME              F77_ID(visitmdexprsetname_,visitmdexprsetname, VISITMDEXPRSETNAME)
#define F_VISITMDEXPRGETNAME              F77_ID(visitmdexprgetname_,visitmdexprgetname, VISITMDEXPRGETNAME)
#define F_VISITMDEXPRSETDEFINITION        F77_ID(visitmdexprsetdefinition_,visitmdexprsetdefinition, VISITMDEXPRSETDEFINITION)
#define F_VISITMDEXPRGETDEFINITION        F77_ID(visitmdexprgetdefinition_,visitmdexprgetdefinition, VISITMDEXPRGETDEFINITION)
#define F_VISITMDEXPRSETTYPE              F77_ID(visitmdexprsettype_,visitmdexprsettype, VISITMDEXPRSETTYPE)
#define F_VISITMDEXPRGETTYPE              F77_ID(visitmdexprgettype_,visitmdexprgettype, VISITMDEXPRGETTYPE)

int
F_VISITMDEXPRALLOC(visit_handle *h)
{
    return VisIt_ExpressionMetaData_alloc(h);
}

int
F_VISITMDEXPRFREE(visit_handle *h)
{
    return VisIt_ExpressionMetaData_free(*h);
}

int
F_VISITMDEXPRSETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_ExpressionMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDEXPRGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_ExpressionMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDEXPRSETDEFINITION(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_ExpressionMetaData_setDefinition(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDEXPRGETDEFINITION(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_ExpressionMetaData_getDefinition(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDEXPRSETTYPE(visit_handle *h, int *val)
{
    return VisIt_ExpressionMetaData_setType(*h, *val);
}

int
F_VISITMDEXPRGETTYPE(visit_handle *h, int *val)
{
    return VisIt_ExpressionMetaData_getType(*h, val);
}

