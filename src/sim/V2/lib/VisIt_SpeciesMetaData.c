/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
VisIt_SpeciesMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_SpeciesMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_SpeciesMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_setName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_getName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_setMeshName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_setMeshName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_getMeshName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_getMeshName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_setMaterialName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_setMaterialName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_getMaterialName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_getMaterialName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_addSpeciesName(visit_handle h, visit_handle val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_addSpeciesName,
        int (*)(visit_handle, visit_handle),
        int (*cb)(visit_handle, visit_handle),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_getNumSpeciesName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_getNumSpeciesName,
        int (*)(visit_handle, int *),
        int (*cb)(visit_handle, int *),
        (*cb)(h, val));
}

int
VisIt_SpeciesMetaData_getSpeciesName(visit_handle h, int i, visit_handle *val)
{
    VISIT_DYNAMIC_EXECUTE(SpeciesMetaData_addSpeciesName,
        int (*)(visit_handle, int, visit_handle *),
        int (*cb)(visit_handle, int, visit_handle *),
        (*cb)(h, i, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDSPECIESALLOC             F77_ID(visitmdspeciesalloc_,visitmdspeciesalloc,VISITMDSPECIESALLOC)
#define F_VISITMDSPECIESFREE              F77_ID(visitmdspeciesfree_,visitmdspeciesfree,VISITMDSPECIESFREE)
#define F_VISITMDSPECIESSETNAME           F77_ID(visitmdspeciessetname_,visitmdspeciessetname, VISITMDSPECIESSETNAME)
#define F_VISITMDSPECIESGETNAME           F77_ID(visitmdspeciesgetname_,visitmdspeciesgetname, VISITMDSPECIESGETNAME)
#define F_VISITMDSPECIESSETMESHNAME       F77_ID(visitmdspeciessetmeshname_,visitmdspeciessetmeshname, VISITMDSPECIESSETMESHNAME)
#define F_VISITMDSPECIESGETMESHNAME       F77_ID(visitmdspeciesgetmeshname_,visitmdspeciesgetmeshname, VISITMDSPECIESGETMESHNAME)
#define F_VISITMDSPECIESSETMATERIALNAME   F77_ID(visitmdspeciessetmaterialname_,visitmdspeciessetmaterialname, VISITMDSPECIESSETMATERIALNAME)
#define F_VISITMDSPECIESGETMATERIALNAME   F77_ID(visitmdspeciesgetmaterialname_,visitmdspeciesgetmaterialname, VISITMDSPECIESGETMATERIALNAME)
#define F_VISITMDSPECIESADDSPECIESNAME    F77_ID(visitmdspeciesaddspeciesname_,visitmdspeciesaddspeciesname, VISITMDSPECIESADDSPECIESNAME)
#define F_VISITMDSPECIESGETNUMSPECIESNAME  F77_ID(visitmdspeciesgetnumspeciesname_,visitmdspeciesgetnumspeciesname, VISITMDSPECIESGETNUMSPECIESNAME)
#define F_VISITMDSPECIESGETSPECIESNAME    F77_ID(visitmdspeciesgetspeciesname_,visitmdspeciesgetspeciesname, VISITMDSPECIESGETSPECIESNAME)

int
F_VISITMDSPECIESALLOC(visit_handle *h)
{
    return VisIt_SpeciesMetaData_alloc(h);
}

int
F_VISITMDSPECIESFREE(visit_handle *h)
{
    return VisIt_SpeciesMetaData_free(*h);
}

int
F_VISITMDSPECIESSETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_SpeciesMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDSPECIESGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_SpeciesMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDSPECIESSETMESHNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_SpeciesMetaData_setMeshName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDSPECIESGETMESHNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_SpeciesMetaData_getMeshName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDSPECIESSETMATERIALNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_SpeciesMetaData_setMaterialName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDSPECIESGETMATERIALNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_SpeciesMetaData_getMaterialName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDSPECIESADDSPECIESNAME(visit_handle *h, visit_handle *val)
{
    return VisIt_SpeciesMetaData_addSpeciesName(*h, *val);
}

int
F_VISITMDSPECIESGETNUMSPECIESNAME(visit_handle *h, int *val)
{
    return VisIt_SpeciesMetaData_getNumSpeciesName(*h, val);
}

int
F_VISITMDSPECIESGETSPECIESNAME(visit_handle *h, int *i, visit_handle *val)
{
    return VisIt_SpeciesMetaData_getSpeciesName(*h, *i, val);
}

