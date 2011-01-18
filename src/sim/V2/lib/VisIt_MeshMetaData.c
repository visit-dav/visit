/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_MeshMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_MeshMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_MeshMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setMeshType(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setMeshType,
        int (*)(visit_handle, int),
        int (*cb)(visit_handle, int),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getMeshType(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getMeshType,
        int (*)(visit_handle, int*),
        int (*cb)(visit_handle, int*),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setTopologicalDimension(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setTopologicalDimension,
        int (*)(visit_handle, int),
        int (*cb)(visit_handle, int),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getTopologicalDimension(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getTopologicalDimension,
        int (*)(visit_handle, int*),
        int (*cb)(visit_handle, int*),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setSpatialDimension(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setSpatialDimension,
        int (*)(visit_handle, int),
        int (*cb)(visit_handle, int),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getSpatialDimension(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getSpatialDimension,
        int (*)(visit_handle, int*),
        int (*cb)(visit_handle, int*),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setNumDomains(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setNumDomains,
        int (*)(visit_handle, int),
        int (*cb)(visit_handle, int),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getNumDomains(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumDomains,
        int (*)(visit_handle, int*),
        int (*cb)(visit_handle, int*),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setDomainTitle(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setDomainTitle,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getDomainTitle(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getDomainTitle,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setDomainPieceName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setDomainPieceName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getDomainPieceName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getDomainPieceName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_addDomainName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_addDomainName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getNumDomainName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumDomainName,
        int (*)(visit_handle, int *),
        int (*cb)(visit_handle, int *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getDomainName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_addDomainName,
        int (*)(visit_handle, int, char **),
        int (*cb)(visit_handle, int, char **),
        (*cb)(h, i, val));
}

int
VisIt_MeshMetaData_setNumGroups(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setNumGroups,
        int (*)(visit_handle, int),
        int (*cb)(visit_handle, int),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getNumGroups(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumGroups,
        int (*)(visit_handle, int*),
        int (*cb)(visit_handle, int*),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setGroupTitle(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setGroupTitle,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getGroupTitle(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getGroupTitle,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setGroupPieceName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setGroupPieceName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getGroupPieceName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getGroupPieceName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_addGroupId(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_addGroupId,
        int (*)(visit_handle, int),
        int (*cb)(visit_handle, int),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getNumGroupId(visit_handle h, int *n)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumGroupId,
        int (*)(visit_handle, int*),
        int (*cb)(visit_handle, int*),
        (*cb)(h, n));
}

int
VisIt_MeshMetaData_getGroupId(visit_handle h, int i, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getNumGroupId,
        int (*)(visit_handle, int,int*),
        int (*cb)(visit_handle, int,int*),
        (*cb)(h, i, val));
}

int
VisIt_MeshMetaData_setXUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setXUnits,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getXUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getXUnits,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setYUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setYUnits,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getYUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getYUnits,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setZUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setZUnits,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getZUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getZUnits,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setXLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setXLabel,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getXLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getXLabel,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setYLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setYLabel,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getYLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getYLabel,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_setZLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_setZLabel,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MeshMetaData_getZLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MeshMetaData_getZLabel,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
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

