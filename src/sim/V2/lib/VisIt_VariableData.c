#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"

int
VisIt_VariableData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableData_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_VariableData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableData_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_VariableData_setDataC(visit_handle obj, int owner, int ncomps,
    int ntuples, char *ptr)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_CHAR,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataI(visit_handle obj, int owner, int ncomps,
    int ntuples, int *ptr)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_INT,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataF(visit_handle obj, int owner, int ncomps,
    int ntuples, float *ptr)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_FLOAT,ncomps,ntuples,(void *)ptr));
}

int
VisIt_VariableData_setDataD(visit_handle obj, int owner, int ncomps,
    int ntuples, double *ptr)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableData_setData,
                    int (*)(visit_handle,int,int,int,int,void*), 
                    int (*cb)(visit_handle,int,int,int,int,void *), 
                    (*cb)(obj,owner,VISIT_DATATYPE_DOUBLE,ncomps,ntuples,(void *)ptr));
}

