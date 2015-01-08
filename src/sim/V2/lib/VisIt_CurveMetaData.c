/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
VisIt_CurveMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_CurveMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_CurveMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_CurveMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_CurveMetaData_setXUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_setXUnits,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_CurveMetaData_getXUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_getXUnits,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_CurveMetaData_setYUnits(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_setYUnits,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_CurveMetaData_getYUnits(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_getYUnits,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_CurveMetaData_setXLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_setXLabel,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_CurveMetaData_getXLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_getXLabel,
        int, (visit_handle, char **),
        (h, val));
}

int
VisIt_CurveMetaData_setYLabel(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_setYLabel,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_CurveMetaData_getYLabel(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(CurveMetaData_getYLabel,
        int, (visit_handle, char **),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMDCURVEALLOC               F77_ID(visitmdcurvealloc_,visitmdcurvealloc,VISITMDCURVEALLOC)
#define F_VISITMDCURVEFREE                F77_ID(visitmdcurvefree_,visitmdcurvefree,VISITMDCURVEFREE)
#define F_VISITMDCURVESETNAME             F77_ID(visitmdcurvesetname_,visitmdcurvesetname, VISITMDCURVESETNAME)
#define F_VISITMDCURVEGETNAME             F77_ID(visitmdcurvegetname_,visitmdcurvegetname, VISITMDCURVEGETNAME)
#define F_VISITMDCURVESETXUNITS           F77_ID(visitmdcurvesetxunits_,visitmdcurvesetxunits, VISITMDCURVESETXUNITS)
#define F_VISITMDCURVEGETXUNITS           F77_ID(visitmdcurvegetxunits_,visitmdcurvegetxunits, VISITMDCURVEGETXUNITS)
#define F_VISITMDCURVESETYUNITS           F77_ID(visitmdcurvesetyunits_,visitmdcurvesetyunits, VISITMDCURVESETYUNITS)
#define F_VISITMDCURVEGETYUNITS           F77_ID(visitmdcurvegetyunits_,visitmdcurvegetyunits, VISITMDCURVEGETYUNITS)
#define F_VISITMDCURVESETXLABEL           F77_ID(visitmdcurvesetxlabel_,visitmdcurvesetxlabel, VISITMDCURVESETXLABEL)
#define F_VISITMDCURVEGETXLABEL           F77_ID(visitmdcurvegetxlabel_,visitmdcurvegetxlabel, VISITMDCURVEGETXLABEL)
#define F_VISITMDCURVESETYLABEL           F77_ID(visitmdcurvesetylabel_,visitmdcurvesetylabel, VISITMDCURVESETYLABEL)
#define F_VISITMDCURVEGETYLABEL           F77_ID(visitmdcurvegetylabel_,visitmdcurvegetylabel, VISITMDCURVEGETYLABEL)

int
F_VISITMDCURVEALLOC(visit_handle *h)
{
    return VisIt_CurveMetaData_alloc(h);
}

int
F_VISITMDCURVEFREE(visit_handle *h)
{
    return VisIt_CurveMetaData_free(*h);
}

int
F_VISITMDCURVESETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_CurveMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDCURVEGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_CurveMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDCURVESETXUNITS(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_CurveMetaData_setXUnits(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDCURVEGETXUNITS(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_CurveMetaData_getXUnits(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDCURVESETYUNITS(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_CurveMetaData_setYUnits(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDCURVEGETYUNITS(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_CurveMetaData_getYUnits(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDCURVESETXLABEL(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_CurveMetaData_setXLabel(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDCURVEGETXLABEL(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_CurveMetaData_getXLabel(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

int
F_VISITMDCURVESETYLABEL(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_CurveMetaData_setYLabel(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMDCURVEGETYLABEL(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_CurveMetaData_getYLabel(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

