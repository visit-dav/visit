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

int
VisIt_VariableData_setDataC(visit_handle obj, int owner, int ncomps,
    int ntuples, char *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int, (visit_handle,int,int,int,int,void*), 
                    (obj,owner,VISIT_DATATYPE_CHAR,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataI(visit_handle obj, int owner, int ncomps,
    int ntuples, int *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int, (visit_handle,int,int,int,int,void*), 
                    (obj,owner,VISIT_DATATYPE_INT,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataF(visit_handle obj, int owner, int ncomps,
    int ntuples, float *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int, (visit_handle,int,int,int,int,void*), 
                    (obj,owner,VISIT_DATATYPE_FLOAT,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataD(visit_handle obj, int owner, int ncomps,
    int ntuples, double *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int, (visit_handle,int,int,int,int,void*), 
                    (obj,owner,VISIT_DATATYPE_DOUBLE,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_getData(visit_handle obj, int *owner, int *dataType, int *ncomps,
    int *ntuples, void **ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2,
                    int, (visit_handle,int*,int*,int*,int*,void**), 
                    (obj,owner,dataType,ncomps,ntuples,(void **)ptr));
}

int
VisIt_VariableData_getDataC(visit_handle obj, int *owner, int *ncomps,
    int *ntuples, char **ptr)
{
    int dataType;
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2,
                    int, (visit_handle,int*,int*,int*,int*,void**), 
                    (obj,owner,&dataType,ncomps,ntuples,(void **)ptr));
}

int
VisIt_VariableData_getDataI(visit_handle obj, int *owner, int *ncomps,
    int *ntuples, int **ptr)
{
    int dataType;
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2,
                    int, (visit_handle,int*,int*,int*,int*,void**), 
                    (obj,owner,&dataType,ncomps,ntuples,(void **)ptr));
}

int
VisIt_VariableData_getDataF(visit_handle obj, int *owner, int *ncomps,
    int *ntuples, float **ptr)
{
    int dataType;
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2,
                    int, (visit_handle,int*,int*,int*,int*,void**), 
                    (obj,owner,&dataType,ncomps,ntuples,(void **)ptr));
}

int
VisIt_VariableData_getDataD(visit_handle obj, int *owner, int *ncomps,
    int *ntuples, double **ptr)
{
    int dataType;
    VISIT_DYNAMIC_EXECUTE(VariableData_getData2,
                    int, (visit_handle,int*,int*,int*,int*,void**), 
                    (obj,owner,&dataType,ncomps,ntuples,(void **)ptr));
}


/************************** Fortran callable routines *************************/
#define F_VISITVARDATAALLOC   F77_ID(visitvardataalloc_,visitvardataalloc,VISITVARDATAALLOC)
#define F_VISITVARDATAFREE    F77_ID(visitvardatafree_,visitvardatafree,VISITVARDATAFREE)
#define F_VISITVARDATASETC    F77_ID(visitvardatasetc_,visitvardatasetc,VISITVARDATASETC)
#define F_VISITVARDATASETI    F77_ID(visitvardataseti_,visitvardataseti,VISITVARDATASETI)
#define F_VISITVARDATASETF    F77_ID(visitvardatasetf_,visitvardatasetf,VISITVARDATASETF)
#define F_VISITVARDATASETD    F77_ID(visitvardatasetd_,visitvardatasetd,VISITVARDATASETD)
#define F_VISITVARDATAGETTYPE F77_ID(visitvardatagettype_,visitvardatagettype,VISITVARDATAGETTYPE)
#define F_VISITVARDATAGETC    F77_ID(visitvardatagetc_,visitvardatagetc,VISITVARDATAGETC)
#define F_VISITVARDATAGETI    F77_ID(visitvardatageti_,visitvardatageti,VISITVARDATAGETI)
#define F_VISITVARDATAGETF    F77_ID(visitvardatagetf_,visitvardatagetf,VISITVARDATAGETF)
#define F_VISITVARDATAGETD    F77_ID(visitvardatagetd_,visitvardatagetd,VISITVARDATAGETD)

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

int
F_VISITVARDATASETC(visit_handle *obj, int *owner, int *ncomps, int *ntuples, char *ptr)
{
    int realOwner = (*owner == VISIT_OWNER_VISIT) ? VISIT_OWNER_COPY : *owner;
    return VisIt_VariableData_setDataC(*obj, realOwner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATASETI(visit_handle *obj, int *owner, int *ncomps, int *ntuples, int *ptr)
{
    int realOwner = (*owner == VISIT_OWNER_VISIT) ? VISIT_OWNER_COPY : *owner;
    return VisIt_VariableData_setDataI(*obj, realOwner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATASETF(visit_handle *obj, int *owner, int *ncomps, int *ntuples, float *ptr)
{
    int realOwner = (*owner == VISIT_OWNER_VISIT) ? VISIT_OWNER_COPY : *owner;
    return VisIt_VariableData_setDataF(*obj, realOwner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATASETD(visit_handle *obj, int *owner, int *ncomps, int *ntuples, double *ptr)
{
    int realOwner = (*owner == VISIT_OWNER_VISIT) ? VISIT_OWNER_COPY : *owner;
    return VisIt_VariableData_setDataD(*obj, realOwner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATAGETTYPE(visit_handle *obj, int *dataType)
{
    int owner, ncomps, ntuples;
    void *ptr = NULL;
    return VisIt_VariableData_getData(*obj, &owner, dataType, &ncomps, &ntuples, &ptr);
}

int
F_VISITVARDATAGETC(visit_handle *obj, int *owner, int *ncomps, int *ntuples, char *dest, int *ldest)
{
    int retval, sz;
    char *ptr = NULL;
    sz = *ldest;
    retval = VisIt_VariableData_getDataC(*obj, owner, ncomps, ntuples, &ptr);
    if(*ncomps * *ntuples < *ldest)
        sz = *ncomps * *ntuples;
    memcpy((void *)dest, (void *)ptr, sz * sizeof(char));
    return retval;
}

int
F_VISITVARDATAGETI(visit_handle *obj, int *owner, int *ncomps, int *ntuples, int *dest, int *ldest)
{
    int retval, sz;
    int *ptr = NULL;
    sz = *ldest;
    retval = VisIt_VariableData_getDataI(*obj, owner, ncomps, ntuples, &ptr);
    if(*ncomps * *ntuples < *ldest)
        sz = *ncomps * *ntuples;
    memcpy((void *)dest, (void *)ptr, sz * sizeof(int));
    return retval;
}

int
F_VISITVARDATAGETF(visit_handle *obj, int *owner, int *ncomps, int *ntuples, float *dest, int *ldest)
{
    int retval, sz;
    float *ptr = NULL;
    sz = *ldest;
    retval = VisIt_VariableData_getDataF(*obj, owner, ncomps, ntuples, &ptr);
    if(*ncomps * *ntuples < *ldest)
        sz = *ncomps * *ntuples;
    memcpy((void *)dest, (void *)ptr, sz * sizeof(float));
    return retval;
}

int
F_VISITVARDATAGETD(visit_handle *obj, int *owner, int *ncomps, int *ntuples, double *dest, int *ldest)
{
    int retval, sz;
    double *ptr = NULL;
    sz = *ldest;
    retval = VisIt_VariableData_getDataD(*obj, owner, ncomps, ntuples, &ptr);
    if(*ncomps * *ntuples < *ldest)
        sz = *ncomps * *ntuples;
    memcpy((void *)dest, (void *)ptr, sz * sizeof(double));
    return retval;
}
