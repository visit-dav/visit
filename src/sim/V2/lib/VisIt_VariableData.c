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
#include <string.h>

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_VariableData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_VariableData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_free,
                    int, (visit_handle),
                    (obj));
}

/* Set functions */
int
VisIt_VariableData_setData(visit_handle obj, int owner, int dataType,
     int ncomps, int ntuples, void *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int, (visit_handle,int,int,int,int,void*),
                    (obj,owner,dataType,ncomps,ntuples,ptr));
}

int
VisIt_VariableData_setDataEx(visit_handle obj, int owner, int dataType,
     int ncomps, int ntuples, void *ptr, void(*callback)(void*), void *callbackData)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setDataEx,
                    int, (visit_handle,int,int,int,int,void*,void(*)(void*),void*),
                    (obj,owner,dataType,ncomps,ntuples,ptr,callback,callbackData));
}

#define DEFINE_SET_DATA(FUNC, CTYPE, VISITTYPE) \
int \
FUNC(visit_handle obj, int owner, int ncomps, int ntuples, CTYPE *ptr) \
{ \
    VISIT_DYNAMIC_EXECUTE(VariableData_setData, \
                    int, (visit_handle,int,int,int,int,void*),  \
                    (obj,owner,VISITTYPE,ncomps,ntuples,(void *)ptr)); \
}

DEFINE_SET_DATA(VisIt_VariableData_setDataC, char,   VISIT_DATATYPE_CHAR);
DEFINE_SET_DATA(VisIt_VariableData_setDataI, int,    VISIT_DATATYPE_INT);
DEFINE_SET_DATA(VisIt_VariableData_setDataL, long,   VISIT_DATATYPE_LONG);
DEFINE_SET_DATA(VisIt_VariableData_setDataF, float,  VISIT_DATATYPE_FLOAT);
DEFINE_SET_DATA(VisIt_VariableData_setDataD, double, VISIT_DATATYPE_DOUBLE);

/*int
VisIt_VariableData_setNumberOfComponents(visit_handle obj, int nComps)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setNumberOfComponents,
                          int, (visit_handle,int),
                          (obj,nComps));
}*/

#define DEFINE_SET_ARRAY_DATA(FUNC, CTYPE, VISITTYPE) \
int \
FUNC(visit_handle obj, int arrIndex, int owner, int ntuples,  \
    int offset, int stride, CTYPE *ptr) \
{ \
    VISIT_DYNAMIC_EXECUTE(VariableData_setArrayData, \
                    int, (visit_handle,int,int,int,int,int,int,void*),  \
                    (obj,arrIndex,owner,VISITTYPE,ntuples,offset,stride,(void *)ptr)); \
}

DEFINE_SET_ARRAY_DATA(VisIt_VariableData_setArrayDataC, char,   VISIT_DATATYPE_CHAR)
DEFINE_SET_ARRAY_DATA(VisIt_VariableData_setArrayDataI, int,    VISIT_DATATYPE_INT)
DEFINE_SET_ARRAY_DATA(VisIt_VariableData_setArrayDataL, long,   VISIT_DATATYPE_LONG)
DEFINE_SET_ARRAY_DATA(VisIt_VariableData_setArrayDataF, float,  VISIT_DATATYPE_FLOAT)
DEFINE_SET_ARRAY_DATA(VisIt_VariableData_setArrayDataD, double, VISIT_DATATYPE_DOUBLE)

/* Get functions */

int
VisIt_VariableData_getData(visit_handle obj, int *owner, int *dataType, int *ncomps,
    int *ntuples, void **ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2,
                    int, (visit_handle,int*,int*,int*,int*,void**), 
                    (obj,owner,dataType,ncomps,ntuples,(void **)ptr));
}

#define DEFINE_GET_DATA(FUNC, CTYPE) \
int \
FUNC(visit_handle obj, int *owner, int *ncomps, int *ntuples, CTYPE **ptr) \
{ \
    int dataType; \
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2, \
                    int, (visit_handle,int*,int*,int*,int*,void**),  \
                    (obj,owner,&dataType,ncomps,ntuples,(void **)ptr)); \
}

DEFINE_GET_DATA(VisIt_VariableData_getDataC, char);
DEFINE_GET_DATA(VisIt_VariableData_getDataI, int);
DEFINE_GET_DATA(VisIt_VariableData_getDataL, long);
DEFINE_GET_DATA(VisIt_VariableData_getDataF, float);
DEFINE_GET_DATA(VisIt_VariableData_getDataD, double);


/************************** Fortran callable routines *************************/
#define F_VISITVARDATAALLOC    F77_ID(visitvardataalloc_,visitvardataalloc,VISITVARDATAALLOC)
#define F_VISITVARDATAFREE     F77_ID(visitvardatafree_,visitvardatafree,VISITVARDATAFREE)

#define F_VISITVARDATASETC     F77_ID(visitvardatasetc_,visitvardatasetc,VISITVARDATASETC)
#define F_VISITVARDATASETI     F77_ID(visitvardataseti_,visitvardataseti,VISITVARDATASETI)
#define F_VISITVARDATASETL     F77_ID(visitvardatasetl_,visitvardatasetl,VISITVARDATASETL)
#define F_VISITVARDATASETF     F77_ID(visitvardatasetf_,visitvardatasetf,VISITVARDATASETF)
#define F_VISITVARDATASETD     F77_ID(visitvardatasetd_,visitvardatasetd,VISITVARDATASETD)

#define F_VISITVARDATASETARRC  F77_ID(visitvardatasetarrc_,visitvardatasetarrc,VISITVARDATASETARRC)
#define F_VISITVARDATASETARRI  F77_ID(visitvardatasetarri_,visitvardatasetarri,VISITVARDATASETARRI)
#define F_VISITVARDATASETARRL  F77_ID(visitvardatasetarrl_,visitvardatasetarrl,VISITVARDATASETARRL)
#define F_VISITVARDATASETARRF  F77_ID(visitvardatasetarrf_,visitvardatasetarrf,VISITVARDATASETARRF)
#define F_VISITVARDATASETARRD  F77_ID(visitvardatasetarrd_,visitvardatasetarrd,VISITVARDATASETARRD)

#define F_VISITVARDATAGETTYPE  F77_ID(visitvardatagettype_,visitvardatagettype,VISITVARDATAGETTYPE)
#define F_VISITVARDATAGETC     F77_ID(visitvardatagetc_,visitvardatagetc,VISITVARDATAGETC)
#define F_VISITVARDATAGETI     F77_ID(visitvardatageti_,visitvardatageti,VISITVARDATAGETI)
#define F_VISITVARDATAGETL     F77_ID(visitvardatagetl_,visitvardatagetl,VISITVARDATAGETL)
#define F_VISITVARDATAGETF     F77_ID(visitvardatagetf_,visitvardatagetf,VISITVARDATAGETF)
#define F_VISITVARDATAGETD     F77_ID(visitvardatagetd_,visitvardatagetd,VISITVARDATAGETD)

/*#define F_VISITVARDATAGETARRC F77_ID(visitvardatagetarrc_,visitvardatagetarrc,VISITVARDATAGETARRC)
#define F_VISITVARDATAGETARRI F77_ID(visitvardatagetarri_,visitvardatagetarri,VISITVARDATAGETARRI)
#define F_VISITVARDATAGETARRL F77_ID(visitvardatagetarrl_,visitvardatagetarrl,VISITVARDATAGETARRL)
#define F_VISITVARDATAGETARRF F77_ID(visitvardatagetarrf_,visitvardatagetarrf,VISITVARDATAGETARRF)
#define F_VISITVARDATAGETARRD F77_ID(visitvardatagetarrd_,visitvardatagetarrd,VISITVARDATAGETARRD)
*/

int
F_VISITVARDATAALLOC(visit_handle *obj)
{
    return VisIt_VariableData_alloc(obj);
}

int
F_VISITVARDATAFREE(visit_handle *obj)
{
    return VisIt_VariableData_free(*obj);
}

#define DEFINE_F_SET_DATA(FUNC, FFUNC, CTYPE) \
int \
FFUNC(visit_handle *obj, int *owner, int *ncomps, int *ntuples, CTYPE *ptr) \
{ \
    int realOwner = (*owner == VISIT_OWNER_VISIT) ? VISIT_OWNER_COPY : *owner; \
    return FUNC(*obj, realOwner, *ncomps, *ntuples, ptr); \
}

DEFINE_F_SET_DATA(VisIt_VariableData_setDataC, F_VISITVARDATASETC, char);
DEFINE_F_SET_DATA(VisIt_VariableData_setDataI, F_VISITVARDATASETI, int);
DEFINE_F_SET_DATA(VisIt_VariableData_setDataL, F_VISITVARDATASETL, long);
DEFINE_F_SET_DATA(VisIt_VariableData_setDataF, F_VISITVARDATASETF, float);
DEFINE_F_SET_DATA(VisIt_VariableData_setDataD, F_VISITVARDATASETD, double);

#define DEFINE_F_SET_ARR_DATA(FUNC, FFUNC, CTYPE) \
int \
FFUNC(visit_handle *obj, int *arrIndex, int *owner, int *ntuples, int *offset, int *stride, CTYPE *ptr) \
{ \
    int realOwner = (*owner == VISIT_OWNER_VISIT) ? VISIT_OWNER_COPY : *owner; \
    return FUNC(*obj, *arrIndex, realOwner, *ntuples, *offset, *stride, ptr); \
}

DEFINE_F_SET_ARR_DATA(VisIt_VariableData_setArrayDataC, F_VISITVARDATASETARRC, char);
DEFINE_F_SET_ARR_DATA(VisIt_VariableData_setArrayDataI, F_VISITVARDATASETARRI, int);
DEFINE_F_SET_ARR_DATA(VisIt_VariableData_setArrayDataL, F_VISITVARDATASETARRL, long);
DEFINE_F_SET_ARR_DATA(VisIt_VariableData_setArrayDataF, F_VISITVARDATASETARRF, float);
DEFINE_F_SET_ARR_DATA(VisIt_VariableData_setArrayDataD, F_VISITVARDATASETARRD, double);

int
F_VISITVARDATAGETTYPE(visit_handle *obj, int *dataType)
{
    int owner, ncomps, ntuples;
    void *ptr = NULL;
    return VisIt_VariableData_getData(*obj, &owner, dataType, &ncomps, &ntuples, &ptr);
}

#define DEFINE_F_GET_DATA(FUNC, FFUNC, CTYPE) \
int \
FFUNC(visit_handle *obj, int *owner, int *ncomps, int *ntuples, CTYPE *dest, int *ldest) \
{ \
    int retval, sz; \
    CTYPE *ptr = NULL; \
    sz = *ldest; \
    retval = FUNC(*obj, owner, ncomps, ntuples, &ptr); \
    if(*ncomps * *ntuples < *ldest) \
        sz = *ncomps * *ntuples; \
    memcpy((void *)dest, (void *)ptr, sz * sizeof(CTYPE)); \
    return retval; \
}

DEFINE_F_GET_DATA(VisIt_VariableData_getDataC, F_VISITVARDATAGETC, char);
DEFINE_F_GET_DATA(VisIt_VariableData_getDataI, F_VISITVARDATAGETI, int);
DEFINE_F_GET_DATA(VisIt_VariableData_getDataL, F_VISITVARDATAGETL, long);
DEFINE_F_GET_DATA(VisIt_VariableData_getDataF, F_VISITVARDATAGETF, float);
DEFINE_F_GET_DATA(VisIt_VariableData_getDataD, F_VISITVARDATAGETD, double);
