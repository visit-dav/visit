/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
VisIt_MaterialMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_MaterialMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_MaterialMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_setName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MaterialMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MaterialMetaData_setMeshName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_setMeshName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MaterialMetaData_getMeshName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getMeshName,
        int (*)(visit_handle, char **),
        int (*cb)(visit_handle, char **),
        (*cb)(h, val));
}

int
VisIt_MaterialMetaData_addMaterialName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_addMaterialName,
        int (*)(visit_handle, const char *),
        int (*cb)(visit_handle, const char *),
        (*cb)(h, val));
}

int
VisIt_MaterialMetaData_getNumMaterialName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_getNumMaterialName,
        int (*)(visit_handle, int *),
        int (*cb)(visit_handle, int *),
        (*cb)(h, val));
}

int
VisIt_MaterialMetaData_getMaterialName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MaterialMetaData_addMaterialName,
        int (*)(visit_handle, int, char **),
        int (*cb)(visit_handle, int, char **),
        (*cb)(h, i, val));
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
#define F_VISITMDMATGETNUMMATERIALNAME    F77_ID(visitmdmatgetnummaterialname_,visitmdmatgetnummaterialname, VISITMDMATGETNUMMATERIALNAME)
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
F_VISITMDMATGETNUMMATERIALNAME(visit_handle *h, int *val)
{
    return VisIt_MaterialMetaData_getNumMaterialName(*h, val);
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

