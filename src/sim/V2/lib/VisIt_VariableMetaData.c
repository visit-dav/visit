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
VisIt_VariableMetaData_getNumMaterialName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(VariableMetaData_getNumMaterialName,
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
#define F_VISITMDVARGETNUMMATERIALNAME    F77_ID(visitmdvargetnummaterialname_,visitmdvargetnummaterialname, VISITMDVARGETNUMMATERIALNAME)
#define F_VISITMDVARGETMATERIALNAME       F77_ID(visitmdvargetmaterialname_,visitmdvargetmaterialname, VISITMDVARGETMATERIALNAME)

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
F_VISITMDVARGETNUMMATERIALNAME(visit_handle *h, int *val)
{
    return VisIt_VariableMetaData_getNumMaterialName(*h, val);
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

