// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_VariableMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_VariableMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_VariableMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_VariableMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_VariableMetaData_setMeshName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setMeshName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_VariableMetaData_getMeshName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getMeshName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_VariableMetaData_setUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setUnits,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_VariableMetaData_getUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getUnits,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_VariableMetaData_setCentering(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setCentering,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_VariableMetaData_getCentering(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getCentering,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_VariableMetaData_setType(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setType,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_VariableMetaData_getType(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getType,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_VariableMetaData_setTreatAsASCII(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setTreatAsASCII,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_VariableMetaData_getTreatAsASCII(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getTreatAsASCII,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_VariableMetaData_setHideFromGUI(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setHideFromGUI,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_VariableMetaData_getHideFromGUI(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getHideFromGUI,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_VariableMetaData_setNumComponents(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setNumComponents,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_VariableMetaData_getNumComponents(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getNumComponents,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_VariableMetaData_addMaterialName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_addMaterialName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_VariableMetaData_getNumMaterialNames(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getNumMaterialNames,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_VariableMetaData_getMaterialName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getMaterialName,
        int, (visit_handle, int, char **),
        (h, i, val));
}

int
VisIt_VariableMetaData_setEnumerationType(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setEnumerationType,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_VariableMetaData_getEnumerationType(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getEnumerationType,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_VariableMetaData_addEnumNameValue(visit_handle h, const char *name, double val, int * index)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_addEnumNameValue,
                          int, (visit_handle, const char *, double, int *),
                          (h, name, val, index));
}

int
VisIt_VariableMetaData_addEnumNameRange(visit_handle h, const char *name, double minVal, double maxVal, int * index)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_addEnumNameRange,
                          int, (visit_handle, const char *, double, double, int *),
                          (h, name, minVal, maxVal, index));
}

int
VisIt_VariableMetaData_getNumEnumNames(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getNumEnumNames,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_VariableMetaData_getEnumName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getEnumName,
        int, (visit_handle, int, char **),
        (h, i, val));
}

int
VisIt_VariableMetaData_getEnumNameRange(visit_handle h, int i, double * minVal, double * maxVal)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getEnumNameRange,
                          int, (visit_handle, int, double *, double *),
                          (h, i, minVal, maxVal));
}

int
VisIt_VariableMetaData_addEnumGraphEdge(visit_handle h, int head, int tail, const char * name)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_addEnumGraphEdge,
                          int, (visit_handle, int, int, const char *),
                          (h, head, tail, name));
}

int
VisIt_VariableMetaData_getNumEnumGraphEdges(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getNumEnumGraphEdges,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_VariableMetaData_getEnumGraphEdge(visit_handle h, int i, int * head, int * tail, char **edgeName)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getEnumGraphEdge,
                          int, (visit_handle, int, int *, int *, char**),
                          (h, i, head, tail, edgeName));
}

int
VisIt_VariableMetaData_setEnumAlwaysExcludeValue(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setEnumAlwaysExcludeValue,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_VariableMetaData_setEnumAlwaysExcludeRange(visit_handle h, double minVal, double maxVal)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setEnumAlwaysExcludeRange,
                          int, (visit_handle, double, double),
                          (h, minVal, maxVal));
}

int
VisIt_VariableMetaData_getEnumAlwaysExcludeRange(visit_handle h, double *minVal, double *maxVal)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getEnumAlwaysExcludeRange,
                          int, (visit_handle, double *, double *),
                          (h, minVal, maxVal));
}

int
VisIt_VariableMetaData_setEnumAlwaysIncludeValue(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setEnumAlwaysIncludeValue,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_VariableMetaData_setEnumAlwaysIncludeRange(visit_handle h, double minVal, double maxVal)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_setEnumAlwaysIncludeRange,
                          int, (visit_handle, double, double),
                          (h, minVal, maxVal));
}

int
VisIt_VariableMetaData_getEnumAlwaysIncludeRange(visit_handle h, double *minVal, double *maxVal)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getEnumAlwaysIncludeRange,
                          int, (visit_handle, double *, double *),
                          (h, minVal, maxVal));
}



/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDVARALLOC                 F77_ID(visitmdvaralloc_,visitmdvaralloc,VISITMDVARALLOC)
#define F_VISITMDVARFREE                  F77_ID(visitmdvarfree_,visitmdvarfree,VISITMDVARFREE)
#define F_VISITMDVARSETNAME               F77_ID(visitmdvarsetname_,visitmdvarsetname, VISITMDVARSETNAME)
#define F_VISITMDVARGETNAME               F77_ID(visitmdvargetname_,visitmdvargetname, VISITMDVARGETNAME)
#define F_VISITMDVARSETMESHNAME           F77_ID(visitmdvarsetmeshname_,visitmdvarsetmeshname, VISITMDVARSETMESHNAME)
#define F_VISITMDVARGETMESHNAME           F77_ID(visitmdvargetmeshname_,visitmdvargetmeshname, VISITMDVARGETMESHNAME)
#define F_VISITMDVARSETUNITS              F77_ID(visitmdvarsetunits_,visitmdvarsetunits, VISITMDVARSETUNITS)
#define F_VISITMDVARGETUNITS              F77_ID(visitmdvargetunits_,visitmdvargetunits, VISITMDVARGETUNITS)
#define F_VISITMDVARSETCENTERING          F77_ID(visitmdvarsetcentering_,visitmdvarsetcentering, VISITMDVARSETCENTERING)
#define F_VISITMDVARGETCENTERING          F77_ID(visitmdvargetcentering_,visitmdvargetcentering, VISITMDVARGETCENTERING)
#define F_VISITMDVARSETTYPE               F77_ID(visitmdvarsettype_,visitmdvarsettype, VISITMDVARSETTYPE)
#define F_VISITMDVARGETTYPE               F77_ID(visitmdvargettype_,visitmdvargettype, VISITMDVARGETTYPE)
#define F_VISITMDVARSETTREATASASCII       F77_ID(visitmdvarsettreatasascii_,visitmdvarsettreatasascii, VISITMDVARSETTREATASASCII)
#define F_VISITMDVARGETTREATASASCII       F77_ID(visitmdvargettreatasascii_,visitmdvargettreatasascii, VISITMDVARGETTREATASASCII)
#define F_VISITMDVARSETHIDEFROMGUI        F77_ID(visitmdvarsethidefromgui_,visitmdvarsethidefromgui, VISITMDVARSETHIDEFROMGUI)
#define F_VISITMDVARGETHIDEFROMGUI        F77_ID(visitmdvargethidefromgui_,visitmdvargethidefromgui, VISITMDVARGETHIDEFROMGUI)
#define F_VISITMDVARSETNUMCOMPONENTS      F77_ID(visitmdvarsetnumcomponents_,visitmdvarsetnumcomponents, VISITMDVARSETNUMCOMPONENTS)
#define F_VISITMDVARGETNUMCOMPONENTS      F77_ID(visitmdvargetnumcomponents_,visitmdvargetnumcomponents, VISITMDVARGETNUMCOMPONENTS)
#define F_VISITMDVARADDMATERIALNAME       F77_ID(visitmdvaraddmaterialname_,visitmdvaraddmaterialname, VISITMDVARADDMATERIALNAME)
#define F_VISITMDVARGETNUMMATERIALNAMES   F77_ID(visitmdvargetnummaterialnames_,visitmdvargetnummaterialnames, VISITMDVARGETNUMMATERIALNAMES)
#define F_VISITMDVARGETMATERIALNAME       F77_ID(visitmdvargetmaterialname_,visitmdvargetmaterialname, VISITMDVARGETMATERIALNAME)

#define F_VISITMDENUMSETTYPE               F77_ID(visitmdenumsettype_,visitmdenumsettype, VISITMDENUMSETTYPE)
#define F_VISITMDENUMGETTYPE               F77_ID(visitmdenumgettype_,visitmdenumgettype, VISITMDENUMGETTYPE)
#define F_VISITMDVARADDENUMNAMEVALUE       F77_ID(visitmdvaraddenumnamevalue_,visitmdvaraddenumnamevalue,VISITMDVARADDENUMNAMEVALUE)
#define F_VISITMDVARADDENUMNAMERANGE       F77_ID(visitmdvaraddenumnamerange_,visitmdvaraddenumnamerange,VISITMDVARADDENUMNAMERANGE)
#define F_VISITMDVARGETNUMENUMNAMES        F77_ID(visitmdvargetnumenumnames_,visitmdvargetnumenumnames,VISITMDVARGETNUMENUMNAMES)
#define F_VISITMDVARGETENUMNAME            F77_ID(visitmdvargetenumname_,visitmdvargetenumname,VISITMDVARGETENUMNAME)
#define F_VISITMDVARGETENUMNAMERANGE       F77_ID(visitmdvargetenumnamerange_,visitmdvargetenumnamerange,VISITMDVARGETENUMNAMERANGE)
#define F_VISITMDVARADDENUMGRAPHEDGE       F77_ID(visitmdvaraddenumgraphedge_,visitmdvaraddenumgraphedge,VISITMDVARADDENUMGRAPHEDGE)
#define F_VISITMDVARGETNUMENUMGRAPHEDGES   F77_ID(visitmdvargetnumenumgraphedges_,visitmdvargetnumenumgraphedges,VISITMDVARGETNUMENUMGRAPHEDGES)
#define F_VISITMDVARGETENUMGRAPHEDGE       F77_ID(visitmdvargetenumgraphedge_,visitmdvargetenumgraphedge,VISITMDVARGETENUMGRAPHEDGE)

#define F_VISITMDVARSETENUMALWAYSEXCLUDEVALUE F77_ID(visitmdvarsetenumalwaysexcludevalue_,visitmdvarsetenumalwaysexcludevalue,VISITMDVARSETENUMALWAYSEXCLUDEVALUE)
#define F_VISITMDVARSETENUMALWAYSEXCLUDERANGE F77_ID(visitmdvarsetenumalwaysexcluderange_,visitmdvarsetenumalwaysexcluderange,VISITMDVARSETENUMALWAYSEXCLUDERANGE)
#define F_VISITMDVARGETENUMALWAYSEXCLUDERANGE F77_ID(visitmdvargetenumalwaysexcluderange_,visitmdvargetenumalwaysexcluderange,VISITMDVARGETENUMALWAYSEXCLUDERANGE)
#define F_VISITMDVARSETENUMALWAYSINCLUDEVALUE F77_ID(visitmdvarsetenumalwaysincludevalue_,visitmdvarsetenumalwaysincludevalue,VISITMDVARSETENUMALWAYSINCLUDEVALUE)
#define F_VISITMDVARSETENUMALWAYSINCLUDERANGE F77_ID(visitmdvarsetenumalwaysincluderange_,visitmdvarsetenumalwaysincluderange,VISITMDVARSETENUMALWAYSINCLUDERANGE)
#define F_VISITMDVARGETENUMALWAYSINCLUDERANGE F77_ID(visitmdvargetenumalwaysincluderange_,visitmdvargetenumalwaysincluderange,VISITMDVARGETENUMALWAYSINCLUDERANGE)

int
F_VISITMDVARALLOC(visit_handle *h)
{
    return VisIt_VariableMetaData_alloc(h);
}

int
F_VISITMDVARFREE(visit_handle *h)
{
    return VisIt_VariableMetaData_free(*h);
}

int
F_VISITMDVARSETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_VariableMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDVARGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_VariableMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDVARSETMESHNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_VariableMetaData_setMeshName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDVARGETMESHNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_VariableMetaData_getMeshName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDVARSETUNITS(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_VariableMetaData_setUnits(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDVARGETUNITS(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_VariableMetaData_getUnits(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDVARSETCENTERING(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_setCentering(*h, *val);
}

int
F_VISITMDVARGETCENTERING(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getCentering(*h, val);
}

int
F_VISITMDVARSETTYPE(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_setType(*h, *val);
}

int
F_VISITMDVARGETTYPE(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getType(*h, val);
}

int
F_VISITMDVARSETTREATASASCII(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_setTreatAsASCII(*h, *val);
}

int
F_VISITMDVARGETTREATASASCII(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getTreatAsASCII(*h, val);
}

int
F_VISITMDVARSETHIDEFROMGUI(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_setHideFromGUI(*h, *val);
}

int
F_VISITMDVARGETHIDEFROMGUI(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getHideFromGUI(*h, val);
}

int
F_VISITMDVARSETNUMCOMPONENTS(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_setNumComponents(*h, *val);
}

int
F_VISITMDVARGETNUMCOMPONENTS(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getNumComponents(*h, val);
}

int
F_VISITMDVARADDMATERIALNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_VariableMetaData_addMaterialName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDVARGETNUMMATERIALNAMES(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getNumMaterialNames(*h, val);
}

int
F_VISITMDVARGETMATERIALNAME(visit_handle *h, int *i, char *val, int *lval)
{
    char *s = NULL;
    int retval;
    retval = VisIt_VariableMetaData_getMaterialName(*h, *i, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDENUMSETTYPE(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_setEnumerationType(*h, *val);
}

int
F_VISITMDENUMGETTYPE(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getEnumerationType(*h, val);
}

int
F_VISITMDVARADDENUMNAMEVALUE(visit_handle *h, char *name, int *lname, double *val, int * index)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_VariableMetaData_addEnumNameValue( *h, f_name, *val, index );
    FREE(f_name);
    return retval;
}

int
F_VISITMDVARADDENUMNAMERANGE(visit_handle *h, char *name, int *lname, double *minVal, double *maxVal, int * index)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_VariableMetaData_addEnumNameRange( *h, f_name, *minVal, *maxVal, index );
    FREE(f_name);
    return retval;
}

int
F_VISITMDVARGETNUMENUMNAMES(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getNumEnumNames(*h, val);
}

int
F_VISITMDVARGETENUMNAME(visit_handle *h, int *i, char *val, int *lval)
{
    char *s = NULL;
    int retval;
    retval = VisIt_VariableMetaData_getEnumName(*h, *i, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDVARGETENUMNAMERANGE(visit_handle *h, int *i, double *head, double *tail)
{
    return VisIt_VariableMetaData_getEnumNameRange( *h, *i, head, tail);
}

int
F_VISITMDVARADDENUMGRAPHEDGE(visit_handle *h, int *head, int *tail, char *name, int *lname)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_VariableMetaData_addEnumGraphEdge( *h, *head, *tail, f_name);
    FREE(f_name);
    return retval;
}

int
F_VISITMDVARGETNUMENUMGRAPHEDGES(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getNumEnumGraphEdges(*h, val);
}

int
F_VISITMDVARGETENUMGRAPHEDGE(visit_handle *h, int *i, int *head, int *tail, char *name, int *lname)
{
    char *s = NULL;
    int retval;
    retval = VisIt_VariableMetaData_getEnumGraphEdge( *h, *i, head, tail, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, name, *lname);
        free(s);
    }
    return retval;
}

int
F_VISITMDVARSETENUMALWAYSINCLUDEVALUE(visit_handle *h, double *val)
{
    return VisIt_VariableMetaData_setEnumAlwaysIncludeValue( *h, *val);
}

int
F_VISITMDVARSETENUMALWAYSINCLUDERANGE(visit_handle *h, double *minVal, double *maxVal)
{
    return VisIt_VariableMetaData_setEnumAlwaysIncludeRange( *h, *minVal, *maxVal);
}

int
F_VISITMDVARGETENUMALWAYSINCLUDERANGE(visit_handle *h, double *minVal, double *maxVal)
{
    return VisIt_VariableMetaData_getEnumAlwaysIncludeRange( *h, minVal, maxVal);
}

int
F_VISITMDVARSETENUMALWAYSEXCLUDEVALUE(visit_handle *h, double *val)
{
    return VisIt_VariableMetaData_setEnumAlwaysExcludeValue( *h, *val);
}

int
F_VISITMDVARSETENUMALWAYSEXCLUDERANGE(visit_handle *h, double *minVal, double *maxVal)
{
    return VisIt_VariableMetaData_setEnumAlwaysExcludeRange( *h, *minVal, *maxVal);
}

int
F_VISITMDVARGETENUMALWAYSEXCLUDERANGE(visit_handle *h, double *minVal, double *maxVal)
{
    return VisIt_VariableMetaData_getEnumAlwaysExcludeRange( *h, minVal, maxVal);
}
