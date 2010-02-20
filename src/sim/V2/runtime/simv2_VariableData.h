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

/* This function is only available in the runtime. */
SIMV2_API int simv2_VariableData_getData(visit_handle h, int &owner, 
                                         int &dataType, int &nComps,
                                         int &nTuples, void *&data);
#ifdef __cplusplus
}
#endif

#endif
