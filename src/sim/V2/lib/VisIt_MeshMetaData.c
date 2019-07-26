// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_MeshMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_MeshMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_MeshMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setMeshType(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setMeshType,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getMeshType(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getMeshType,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setTopologicalDimension(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setTopologicalDimension,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getTopologicalDimension(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getTopologicalDimension,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setSpatialDimension(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setSpatialDimension,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getSpatialDimension(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getSpatialDimension,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setNumDomains(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setNumDomains,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getNumDomains(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumDomains,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setDomainTitle(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setDomainTitle,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getDomainTitle(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getDomainTitle,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setDomainPieceName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setDomainPieceName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getDomainPieceName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getDomainPieceName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_addDomainName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_addDomainName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getNumDomainName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumDomainName,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_MeshMetaData_getDomainName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getDomainName,
        int, (visit_handle, int, char **),
        (h, i, val));
}

int
VisIt_MeshMetaData_setNumGroups(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setNumGroups,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getNumGroups(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumGroups,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setGroupTitle(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setGroupTitle,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getGroupTitle(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getGroupTitle,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_addGroupName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_addGroupName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_setGroupPieceName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setGroupPieceName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getGroupPieceName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getGroupPieceName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_addGroupId(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_addGroupId,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getNumGroupId(visit_handle h, int *n)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumGroupId,
        int, (visit_handle, int*),
        (h, n));
}

int
VisIt_MeshMetaData_getGroupId(visit_handle h, int i, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getGroupId,
        int, (visit_handle, int,int*),
        (h, i, val));
}

int
VisIt_MeshMetaData_setXUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setXUnits,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getXUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getXUnits,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setYUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setYUnits,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getYUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getYUnits,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setZUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setZUnits,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getZUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getZUnits,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setXLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setXLabel,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getXLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getXLabel,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setYLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setYLabel,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getYLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getYLabel,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setZLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setZLabel,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MeshMetaData_getZLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getZLabel,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_MeshMetaData_setCellOrigin(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setCellOrigin,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getCellOrigin(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getCellOrigin,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setNodeOrigin(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setNodeOrigin,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getNodeOrigin(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNodeOrigin,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setHasSpatialExtents(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setHasSpatialExtents,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getHasSpatialExtents(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getHasSpatialExtents,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setSpatialExtents(visit_handle h, double val[6])
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setSpatialExtents,
        int, (visit_handle, double[6]),
        (h, val));
}

int
VisIt_MeshMetaData_getSpatialExtents(visit_handle h, double val[6])
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getSpatialExtents,
        int, (visit_handle, double[6]),
        (h, val));
}

int
VisIt_MeshMetaData_setHasLogicalBounds(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setHasLogicalBounds,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_MeshMetaData_getHasLogicalBounds(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getHasLogicalBounds,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_MeshMetaData_setLogicalBounds(visit_handle h, int val[3])
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setLogicalBounds,
        int, (visit_handle, int[3]),
        (h, val));
}

int
VisIt_MeshMetaData_getLogicalBounds(visit_handle h, int val[3])
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getLogicalBounds,
        int, (visit_handle, int[3]),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDMESHALLOC                F77_ID(visitmdmeshalloc_,visitmdmeshalloc,VISITMDMESHALLOC)
#define F_VISITMDMESHFREE                 F77_ID(visitmdmeshfree_,visitmdmeshfree,VISITMDMESHFREE)
#define F_VISITMDMESHSETNAME              F77_ID(visitmdmeshsetname_,visitmdmeshsetname, VISITMDMESHSETNAME)
#define F_VISITMDMESHGETNAME              F77_ID(visitmdmeshgetname_,visitmdmeshgetname, VISITMDMESHGETNAME)
#define F_VISITMDMESHSETMESHTYPE          F77_ID(visitmdmeshsetmeshtype_,visitmdmeshsetmeshtype, VISITMDMESHSETMESHTYPE)
#define F_VISITMDMESHGETMESHTYPE          F77_ID(visitmdmeshgetmeshtype_,visitmdmeshgetmeshtype, VISITMDMESHGETMESHTYPE)
#define F_VISITMDMESHSETTOPOLOGICALDIM    F77_ID(visitmdmeshsettopologicaldim_,visitmdmeshsettopologicaldim, VISITMDMESHSETTOPOLOGICALDIM)
#define F_VISITMDMESHGETTOPOLOGICALDIM    F77_ID(visitmdmeshgettopologicaldim_,visitmdmeshgettopologicaldim, VISITMDMESHGETTOPOLOGICALDIM)
#define F_VISITMDMESHSETSPATIALDIM        F77_ID(visitmdmeshsetspatialdim_,visitmdmeshsetspatialdim, VISITMDMESHSETSPATIALDIM)
#define F_VISITMDMESHGETSPATIALDIM        F77_ID(visitmdmeshgetspatialdim_,visitmdmeshgetspatialdim, VISITMDMESHGETSPATIALDIM)
#define F_VISITMDMESHSETNUMDOMAINS        F77_ID(visitmdmeshsetnumdomains_,visitmdmeshsetnumdomains, VISITMDMESHSETNUMDOMAINS)
#define F_VISITMDMESHGETNUMDOMAINS        F77_ID(visitmdmeshgetnumdomains_,visitmdmeshgetnumdomains, VISITMDMESHGETNUMDOMAINS)
#define F_VISITMDMESHSETDOMAINTITLE       F77_ID(visitmdmeshsetdomaintitle_,visitmdmeshsetdomaintitle, VISITMDMESHSETDOMAINTITLE)
#define F_VISITMDMESHGETDOMAINTITLE       F77_ID(visitmdmeshgetdomaintitle_,visitmdmeshgetdomaintitle, VISITMDMESHGETDOMAINTITLE)
#define F_VISITMDMESHSETDOMAINPIECENAME   F77_ID(visitmdmeshsetdomainpiecename_,visitmdmeshsetdomainpiecename, VISITMDMESHSETDOMAINPIECENAME)
#define F_VISITMDMESHGETDOMAINPIECENAME   F77_ID(visitmdmeshgetdomainpiecename_,visitmdmeshgetdomainpiecename, VISITMDMESHGETDOMAINPIECENAME)
#define F_VISITMDMESHADDDOMAINNAME        F77_ID(visitmdmeshadddomainname_,visitmdmeshadddomainname, VISITMDMESHADDDOMAINNAME)
#define F_VISITMDMESHGETNUMDOMAINNAME     F77_ID(visitmdmeshgetnumdomainname_,visitmdmeshgetnumdomainname, VISITMDMESHGETNUMDOMAINNAME)
#define F_VISITMDMESHGETDOMAINNAME        F77_ID(visitmdmeshgetdomainname_,visitmdmeshgetdomainname, VISITMDMESHGETDOMAINNAME)
#define F_VISITMDMESHSETNUMGROUPS         F77_ID(visitmdmeshsetnumgroups_,visitmdmeshsetnumgroups, VISITMDMESHSETNUMGROUPS)
#define F_VISITMDMESHGETNUMGROUPS         F77_ID(visitmdmeshgetnumgroups_,visitmdmeshgetnumgroups, VISITMDMESHGETNUMGROUPS)
#define F_VISITMDMESHSETGROUPTITLE        F77_ID(visitmdmeshsetgrouptitle_,visitmdmeshsetgrouptitle, VISITMDMESHSETGROUPTITLE)
#define F_VISITMDMESHGETGROUPTITLE        F77_ID(visitmdmeshgetgrouptitle_,visitmdmeshgetgrouptitle, VISITMDMESHGETGROUPTITLE)
#define F_VISITMDMESHADDGROUPNAME        F77_ID(visitmdmeshaddgroupname_,visitmdmeshaddgroupname, VISITMDMESHADDGROUPNAME)
#define F_VISITMDMESHSETGROUPPIECENAME    F77_ID(visitmdmeshsetgrouppiecename_,visitmdmeshsetgrouppiecename, VISITMDMESHSETGROUPPIECENAME)
#define F_VISITMDMESHGETGROUPPIECENAME    F77_ID(visitmdmeshgetgrouppiecename_,visitmdmeshgetgrouppiecename, VISITMDMESHGETGROUPPIECENAME)
#define F_VISITMDMESHSETGROUPIDS          F77_ID(visitmdmeshsetgroupids_,visitmdmeshsetgroupids, VISITMDMESHSETGROUPIDS)
#define F_VISITMDMESHGETGROUPIDS          F77_ID(visitmdmeshgetgroupids_,visitmdmeshgetgroupids, VISITMDMESHGETGROUPIDS)
#define F_VISITMDMESHSETXUNITS            F77_ID(visitmdmeshsetxunits_,visitmdmeshsetxunits, VISITMDMESHSETXUNITS)
#define F_VISITMDMESHGETXUNITS            F77_ID(visitmdmeshgetxunits_,visitmdmeshgetxunits, VISITMDMESHGETXUNITS)
#define F_VISITMDMESHSETYUNITS            F77_ID(visitmdmeshsetyunits_,visitmdmeshsetyunits, VISITMDMESHSETYUNITS)
#define F_VISITMDMESHGETYUNITS            F77_ID(visitmdmeshgetyunits_,visitmdmeshgetyunits, VISITMDMESHGETYUNITS)
#define F_VISITMDMESHSETZUNITS            F77_ID(visitmdmeshsetzunits_,visitmdmeshsetzunits, VISITMDMESHSETZUNITS)
#define F_VISITMDMESHGETZUNITS            F77_ID(visitmdmeshgetzunits_,visitmdmeshgetzunits, VISITMDMESHGETZUNITS)
#define F_VISITMDMESHSETXLABEL            F77_ID(visitmdmeshsetxlabel_,visitmdmeshsetxlabel, VISITMDMESHSETXLABEL)
#define F_VISITMDMESHGETXLABEL            F77_ID(visitmdmeshgetxlabel_,visitmdmeshgetxlabel, VISITMDMESHGETXLABEL)
#define F_VISITMDMESHSETYLABEL            F77_ID(visitmdmeshsetylabel_,visitmdmeshsetylabel, VISITMDMESHSETYLABEL)
#define F_VISITMDMESHGETYLABEL            F77_ID(visitmdmeshgetylabel_,visitmdmeshgetylabel, VISITMDMESHGETYLABEL)
#define F_VISITMDMESHSETZLABEL            F77_ID(visitmdmeshsetzlabel_,visitmdmeshsetzlabel, VISITMDMESHSETZLABEL)
#define F_VISITMDMESHGETZLABEL            F77_ID(visitmdmeshgetzlabel_,visitmdmeshgetzlabel, VISITMDMESHGETZLABEL)
#define F_VISITMDMESHSETCELLORIGIN        F77_ID(visitmdmeshsetcellorigin_,visitmdmeshsetcellorigin, VISITMDMESHSETCELLORIGIN)
#define F_VISITMDMESHGETCELLORIGIN        F77_ID(visitmdmeshgetcellorigin_,visitmdmeshgetcellorigin, VISITMDMESHGETCELLORIGIN)
#define F_VISITMDMESHSETNODEORIGIN        F77_ID(visitmdmeshsetnodeorigin_,visitmdmeshsetnodeorigin, VISITMDMESHSETNODEORIGIN)
#define F_VISITMDMESHGETNODEORIGIN        F77_ID(visitmdmeshgetnodeorigin_,visitmdmeshgetnodeorigin, VISITMDMESHGETNODEORIGIN)
#define F_VISITMDMESHSETHASSPATIALEXTENTS  F77_ID(visitmdmeshsethasspatialextents_,visitmdmeshsethasspatialextents, VISITMDMESHSETHASSPATIALEXTENTS)
#define F_VISITMDMESHGETHASSPATIALEXTENTS  F77_ID(visitmdmeshgethasspatialextents_,visitmdmeshgethasspatialextents, VISITMDMESHGETHASSPATIALEXTENTS)
#define F_VISITMDMESHSETSPATIALEXTENTS    F77_ID(visitmdmeshsetspatialextents_,visitmdmeshsetspatialextents, VISITMDMESHSETSPATIALEXTENTS)
#define F_VISITMDMESHGETSPATIALEXTENTS    F77_ID(visitmdmeshgetspatialextents_,visitmdmeshgetspatialextents, VISITMDMESHGETSPATIALEXTENTS)
#define F_VISITMDMESHSETHASLOGICALBOUNDS  F77_ID(visitmdmeshsethaslogicalbounds_,visitmdmeshsethaslogicalbounds, VISITMDMESHSETHASLOGICALBOUNDS)
#define F_VISITMDMESHGETHASLOGICALBOUNDS  F77_ID(visitmdmeshgethaslogicalbounds_,visitmdmeshgethaslogicalbounds, VISITMDMESHGETHASLOGICALBOUNDS)
#define F_VISITMDMESHSETLOGICALBOUNDS    F77_ID(visitmdmeshsetlogicalbounds_,visitmdmeshsetlogicalbounds, VISITMDMESHSETLOGICALBOUNDS)
#define F_VISITMDMESHGETLOGICALBOUNDS    F77_ID(visitmdmeshgetlogicalbounds_,visitmdmeshgetlogicalbounds, VISITMDMESHGETLOGICALBOUNDS)

int
F_VISITMDMESHALLOC(visit_handle *h)
{
    return VisIt_MeshMetaData_alloc(h);
}

int
F_VISITMDMESHFREE(visit_handle *h)
{
    return VisIt_MeshMetaData_free(*h);
}

int
F_VISITMDMESHSETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETMESHTYPE(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setMeshType(*h, *val);
}

int
F_VISITMDMESHGETMESHTYPE(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getMeshType(*h, val);
}

int
F_VISITMDMESHSETTOPOLOGICALDIM(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setTopologicalDimension(*h, *val);
}

int
F_VISITMDMESHGETTOPOLOGICALDIM(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getTopologicalDimension(*h, val);
}

int
F_VISITMDMESHSETSPATIALDIM(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setSpatialDimension(*h, *val);
}

int
F_VISITMDMESHGETSPATIALDIM(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getSpatialDimension(*h, val);
}

int
F_VISITMDMESHSETNUMDOMAINS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setNumDomains(*h, *val);
}

int
F_VISITMDMESHGETNUMDOMAINS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getNumDomains(*h, val);
}

int
F_VISITMDMESHSETDOMAINTITLE(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setDomainTitle(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETDOMAINTITLE(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getDomainTitle(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETDOMAINPIECENAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setDomainPieceName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETDOMAINPIECENAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getDomainPieceName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHADDDOMAINNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_addDomainName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETNUMDOMAINNAME(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getNumDomainName(*h, val);
}

int
F_VISITMDMESHGETDOMAINNAME(visit_handle *h, int *i, char *val, int *lval)
{
    char *s = NULL;
    int retval;
    retval = VisIt_MeshMetaData_getDomainName(*h, *i, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETNUMGROUPS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setNumGroups(*h, *val);
}

int
F_VISITMDMESHGETNUMGROUPS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getNumGroups(*h, val);
}

int
F_VISITMDMESHSETGROUPTITLE(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setGroupTitle(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETGROUPTITLE(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getGroupTitle(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHADDGROUPNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_addGroupName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHSETGROUPPIECENAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setGroupPieceName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETGROUPPIECENAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getGroupPieceName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETGROUPIDS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_addGroupId(*h, *val);
}

int
F_VISITMDMESHGETNUMGROUPIDS(visit_handle *h, int *n)
{
    return VisIt_MeshMetaData_getNumGroupId(*h, n);
}

int
F_VISITMDMESHGETGROUPIDS(visit_handle *h, int *i, int *val)
{
    return VisIt_MeshMetaData_getGroupId(*h, *i, val);
}

int
F_VISITMDMESHSETXUNITS(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setXUnits(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETXUNITS(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getXUnits(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETYUNITS(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setYUnits(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETYUNITS(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getYUnits(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETZUNITS(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setZUnits(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETZUNITS(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getZUnits(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETXLABEL(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setXLabel(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETXLABEL(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getXLabel(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETYLABEL(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setYLabel(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETYLABEL(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getYLabel(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETZLABEL(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MeshMetaData_setZLabel(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDMESHGETZLABEL(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MeshMetaData_getZLabel(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDMESHSETCELLORIGIN(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setCellOrigin(*h, *val);
}

int
F_VISITMDMESHGETCELLORIGIN(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getCellOrigin(*h, val);
}

int
F_VISITMDMESHSETNODEORIGIN(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setNodeOrigin(*h, *val);
}

int
F_VISITMDMESHGETNODEORIGIN(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getNodeOrigin(*h, val);
}

int
F_VISITMDMESHSETHASSPATIALEXTENTS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setHasSpatialExtents(*h, *val);
}

int
F_VISITMDMESHGETHASSPATIALEXTENTS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getHasSpatialExtents(*h, val);
}

int
F_VISITMDMESHSETSPATIALEXTENTS(visit_handle *h, double val[6])
{
    return VisIt_MeshMetaData_setSpatialExtents(*h, val);
}

int
F_VISITMDMESHGETSPATIALEXTENTS(visit_handle *h, double val[6])
{
    return VisIt_MeshMetaData_getSpatialExtents(*h, val);
}

int
F_VISITMDMESHSETHASLOGICALBOUNDS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_setHasLogicalBounds(*h, *val);
}

int
F_VISITMDMESHGETHASLOGICALBOUNDS(visit_handle *h, int *val)
{
    return VisIt_MeshMetaData_getHasLogicalBounds(*h, val);
}

int
F_VISITMDMESHSETLOGICALBOUNDS(visit_handle *h, int val[3])
{
    return VisIt_MeshMetaData_setLogicalBounds(*h, val);
}

int
F_VISITMDMESHGETLOGICALBOUNDS(visit_handle *h, int val[3])
{
    return VisIt_MeshMetaData_getLogicalBounds(*h, val);
}
