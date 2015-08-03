#ifndef SIMV2_VARIABLEDATA_H
#define SIMV2_VARIABLEDATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

#ifdef __cplusplus
extern "C"
{
#endif
SIMV2_API int simv2_VariableData_alloc(visit_handle *h);
SIMV2_API int simv2_VariableData_free(visit_handle h);

SIMV2_API int simv2_VariableData_setData(visit_handle h, int owner, int dataType,
                                         int nComps, int nTuples, void *data);

SIMV2_API int simv2_VariableData_setArrayData(visit_handle h, 
                                              int arrIndex,
                                              int owner, int dataType,
                                              int nTuples, int offset, int stride,
                                              void *data);

SIMV2_API int simv2_VariableData_setDeletionCallback(visit_handle h, 
                                                     void(*callback)(void*),
                                                     void *callbackData);

SIMV2_API int simv2_VariableData_getData2(visit_handle h, int *owner,
                                          int *dataType, int *nComps,
                                          int *nTuples, void **data);

#ifdef __cplusplus
}
#endif

// These functions are only available in the runtime.
SIMV2_API int simv2_VariableData_getData(visit_handle h, int &owner,
                                         int &dataType, int &nComps,
                                         int &nTuples, void *&data);

SIMV2_API int simv2_VariableData_getNumArrays(visit_handle h, int *nArrays);

SIMV2_API int simv2_VariableData_getArrayData(visit_handle h, 
                                              int arrIndex, int &memory,
                                              int &owner,   int &dataType,
                                              int &nComps,  int &nTuples,
                                              int &offset,  int &stride,
                                              void *&data);

SIMV2_API int simv2_VariableData_getDeletionCallback(visit_handle h,
                                                     void(*&callback)(void*),
                                                     void *&callbackData);

SIMV2_API int simv2_VariableData_nullData(visit_handle h);
#endif
