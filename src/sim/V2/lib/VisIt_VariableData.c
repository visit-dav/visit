#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_VariableData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_VariableData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_VariableData_setDataC(visit_handle obj, int owner, int ncomps,
    int ntuples, char *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_CHAR,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataI(visit_handle obj, int owner, int ncomps,
    int ntuples, int *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_INT,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataF(visit_handle obj, int owner, int ncomps,
    int ntuples, float *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_FLOAT,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataD(visit_handle obj, int owner, int ncomps,
    int ntuples, double *ptr)
{
    VISIT_DYNAMIC_EXECUTE(VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_DOUBLE,ncomps,ntuples,(void *)ptr));
}

/************************** Fortran callable routines *************************/
#define F_VISITVARDATAALLOC F77_ID(visitvardataalloc_,visitvardataalloc,VISITVARDATAALLOC)
#define F_VISITVARDATAFREE  F77_ID(visitvardatafree_,visitvardatafree,VISITVARDATAFREE)
#define F_VISITVARDATASETC  F77_ID(visitvardatasetc_,visitvardatasetc,VISITVARDATASETC)
#define F_VISITVARDATASETI  F77_ID(visitvardataseti_,visitvardataseti,VISITVARDATASETI)
#define F_VISITVARDATASETF  F77_ID(visitvardatasetf_,visitvardatasetf,VISITVARDATASETF)
#define F_VISITVARDATASETD  F77_ID(visitvardatasetd_,visitvardatasetd,VISITVARDATASETD)

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
    return VisIt_VariableData_setDataC(*obj, *owner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATASETI(visit_handle *obj, int *owner, int *ncomps, int *ntuples, int *ptr)
{
    return VisIt_VariableData_setDataI(*obj, *owner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATASETF(visit_handle *obj, int *owner, int *ncomps, int *ntuples, float *ptr)
{
    return VisIt_VariableData_setDataF(*obj, *owner, *ncomps, *ntuples, ptr);
}

int
F_VISITVARDATASETD(visit_handle *obj, int *owner, int *ncomps, int *ntuples, double *ptr)
{
    return VisIt_VariableData_setDataD(*obj, *owner, *ncomps, *ntuples, ptr);
}

