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
VisIt_OptionList_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_OptionList_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_free,
                    int, (visit_handle), 
                    (obj));
}

int VisIt_OptionList_setValueB(visit_handle h, const char *name, int value)
{
    unsigned char c = (unsigned char)value;
    VISIT_DYNAMIC_EXECUTE(OptionList_setValue,
        int, (visit_handle, const char *, int, void *),
        (h, name, VISIT_DATATYPE_CHAR, &c));
}

int VisIt_OptionList_setValueI(visit_handle h, const char *name, int value)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_setValue,
        int, (visit_handle, const char *, int, void *),
        (h, name, VISIT_DATATYPE_INT, &value));
}

int VisIt_OptionList_setValueF(visit_handle h, const char *name, float value)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_setValue,
        int, (visit_handle, const char *, int, void *),
        (h, name, VISIT_DATATYPE_FLOAT, &value));
}

int VisIt_OptionList_setValueD(visit_handle h, const char *name, double value)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_setValue,
        int, (visit_handle, const char *, int, void *),
        (h, name, VISIT_DATATYPE_DOUBLE, &value));
}

int VisIt_OptionList_setValueS(visit_handle h, const char *name, const char *value)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_setValue,
        int, (visit_handle, const char *, int, void *),
        (h, name, VISIT_DATATYPE_STRING, (void*)value));
}

int VisIt_OptionList_getNumValues(visit_handle h, int *nvalues)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_getNumValues,
        int, (visit_handle, int *),
        (h, nvalues));
}

int VisIt_OptionList_getIndex(visit_handle h, const char *name, int *index)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_getIndex,
        int, (visit_handle, const char *, int *),
        (h, name, index));
}

int VisIt_OptionList_getName(visit_handle h, int index, char **name)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_getName,
        int, (visit_handle, int, char **),
        (h, index, name));
}

int VisIt_OptionList_getType(visit_handle h, int index, int *type)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_getType,
        int, (visit_handle, int, int*),
        (h, index, type));
}

static int VisIt_OptionList_getValue(visit_handle h, int index, void **pvalue)
{
    VISIT_DYNAMIC_EXECUTE(OptionList_getValue,
        int, (visit_handle, int, void**),
        (h, index, pvalue));
}

#define GETVALUE_BODY(STYPE, CTYPE) \
    int retval = VISIT_ERROR, index, type; \
    if(VisIt_OptionList_getIndex(h, name, &index) != -1)\
    {\
        if(VisIt_OptionList_getType(h, index, &type) == VISIT_OKAY && \
           type == STYPE) \
        { \
            void *pvalue = NULL; \
            if(VisIt_OptionList_getValue(h, index, &pvalue) == VISIT_OKAY) \
            { \
                *value = *((CTYPE *)pvalue); \
                retval = VISIT_OKAY; \
            } \
        }\
    } \
    return retval;


int VisIt_OptionList_getValueI(visit_handle h, const char *name, int *value)
{
    GETVALUE_BODY(VISIT_DATATYPE_INT, int) 
}

int VisIt_OptionList_getValueF(visit_handle h, const char *name, float *value)
{
    GETVALUE_BODY(VISIT_DATATYPE_FLOAT, float) 
}

int VisIt_OptionList_getValueD(visit_handle h, const char *name, double *value)
{
    GETVALUE_BODY(VISIT_DATATYPE_DOUBLE, double) 
}

int VisIt_OptionList_getValueS(visit_handle h, const char *name, char **value)
{
    int retval = VISIT_ERROR, index, type;
    if(VisIt_OptionList_getIndex(h, name, &index) != -1)
    {
        if(VisIt_OptionList_getType(h, index, &type) == VISIT_OKAY &&
           type == VISIT_DATATYPE_STRING)
        {
            void *pvalue = NULL;
            if(VisIt_OptionList_getValue(h, index, &pvalue) == VISIT_OKAY)
            {
                *value = strdup((char *)pvalue);
                retval = VISIT_OKAY;
            }
        }
    }
    return retval;
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITOPTIONLISTALLOC              F77_ID(visitoptionlistalloc_,visitoptionlistalloc,VISITOPTIONLISTALLOC)
#define F_VISITOPTIONLISTFREE               F77_ID(visitoptionlistfree_,visitoptionlistfree,VISITOPTIONLISTFREE)
#define F_VISITOPTIONLISTSETVALUEB          F77_ID(visitoptionlistsetvalueb_,visitoptionlistsetvalueb, VISITOPTIONLISTSETVALUEB)
#define F_VISITOPTIONLISTSETVALUEI          F77_ID(visitoptionlistsetvaluei_,visitoptionlistsetvaluei, VISITOPTIONLISTSETVALUEI)
#define F_VISITOPTIONLISTSETVALUEL          F77_ID(visitoptionlistsetvaluel_,visitoptionlistsetvaluel, VISITOPTIONLISTSETVALUEL)
#define F_VISITOPTIONLISTSETVALUEF          F77_ID(visitoptionlistsetvaluef_,visitoptionlistsetvaluef, VISITOPTIONLISTSETVALUEF)
#define F_VISITOPTIONLISTSETVALUED          F77_ID(visitoptionlistsetvalued_,visitoptionlistsetvalued, VISITOPTIONLISTSETVALUED)
#define F_VISITOPTIONLISTSETVALUES          F77_ID(visitoptionlistsetvalues_,visitoptionlistsetvalues, VISITOPTIONLISTSETVALUES)

#define F_VISITOPTIONLISTGETNUMVALUES       F77_ID(visitoptionlistgetnumvalues_, visitoptionlistgetnumvalues, VISITOPTIONLISTGETNUMVALUES)
#define F_VISITOPTIONLISTGETTYPE            F77_ID(visitoptionlistgettype_, visitoptionlistgettype, VISITOPTIONLISTGETTYPE)
#define F_VISITOPTIONLISTGETNAME            F77_ID(visitoptionlistgetname_, visitoptionlistgetname, VISITOPTIONLISTGETNAME)

#define F_VISITOPTIONLISTGETVALUEC          F77_ID(visitoptionlistgetvaluec_, visitoptionlistgetvaluec, VISITOPTIONLISTGETVALUEC)
#define F_VISITOPTIONLISTGETVALUEI          F77_ID(visitoptionlistgetvaluei_, visitoptionlistgetvaluei, VISITOPTIONLISTGETVALUEI)
#define F_VISITOPTIONLISTGETVALUEL          F77_ID(visitoptionlistgetvaluel_, visitoptionlistgetvaluel, VISITOPTIONLISTGETVALUEL)
#define F_VISITOPTIONLISTGETVALUEF          F77_ID(visitoptionlistgetvaluef_, visitoptionlistgetvaluef, VISITOPTIONLISTGETVALUEF)
#define F_VISITOPTIONLISTGETVALUED          F77_ID(visitoptionlistgetvalued_, visitoptionlistgetvalued, VISITOPTIONLISTGETVALUED)
#define F_VISITOPTIONLISTGETVALUES          F77_ID(visitoptionlistgetvalues_, visitoptionlistgetvalues, VISITOPTIONLISTGETVALUES)

int
F_VISITOPTIONLISTALLOC(visit_handle *h)
{
    return VisIt_OptionList_alloc(h);
}

int
F_VISITOPTIONLISTFREE(visit_handle *h)
{
    return VisIt_OptionList_free(*h);
}

int
F_VISITOPTIONLISTSETVALUEB(visit_handle *h, const char *name, int *lname, int *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_setValueB(*h, f_name, *value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTSETVALUEI(visit_handle *h, const char *name, int *lname, int *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_setValueI(*h, f_name, *value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTSETVALUEF(visit_handle *h, const char *name, int *lname, float *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_setValueF(*h, f_name, *value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTSETVALUED(visit_handle *h, const char *name, int *lname, double *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_setValueD(*h, f_name, *value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTSETVALUES(visit_handle *h, const char *name, int *lname, const char *value, int *lvalue)
{
    char *f_name = NULL, *f_value = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    COPY_FORTRAN_STRING(f_value, value, lvalue);
    retval = VisIt_OptionList_setValueS(*h, f_name, f_value);
    FREE(f_name);
    FREE(f_value);
    return retval;
}

int
F_VISITOPTIONLISTGETNUMVALUES(visit_handle *h, int *nv)
{
    return VisIt_OptionList_getNumValues(*h, nv);
}

int
F_VISITOPTIONLISTGETTYPE(visit_handle *h, int *index, int *type)
{
    return VisIt_OptionList_getType(*h, *index, type);
}

int
F_VISITOPTIONLISTGETNAME(visit_handle *h, int *index, char *name, int *lname)
{
    char *cname = NULL;
    int retval = VISIT_ERROR;
    if(VisIt_OptionList_getName(*h, *index, &cname) == VISIT_OKAY)
    {
        strncpy(name, cname, *lname);
        free(cname);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
F_VISITOPTIONLISTGETVALUEI(visit_handle *h, char *name, int *lname, int *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_getValueI(*h, f_name, value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTGETVALUEF(visit_handle *h, char *name, int *lname, float *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_getValueF(*h, f_name, value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTGETVALUED(visit_handle *h, char *name, int *lname, double *value)
{
    char *f_name = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_name, name, lname);
    retval = VisIt_OptionList_getValueD(*h, f_name, value);
    FREE(f_name);
    return retval;
}

int
F_VISITOPTIONLISTGETVALUES(visit_handle *h, char *name, int *lname, char *value, int *lvalue)
{
    int retval = VISIT_ERROR;
    char *f_name = NULL, *cvalue = NULL;
    COPY_FORTRAN_STRING(f_name, name, lname);
    if(VisIt_OptionList_getValueS(*h, f_name, &cvalue) == VISIT_OKAY)
    {
        strncpy(value, cvalue, *lvalue);
        free(cvalue);
        retval = VISIT_OKAY;
    }
    FREE(f_name);
    return retval;
}
