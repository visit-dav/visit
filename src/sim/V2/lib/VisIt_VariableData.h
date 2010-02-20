#ifndef VISIT_VARIABLE_DATA_H
#define VISIT_VARIABLE_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_VariableData_alloc(visit_handle*);
int VisIt_VariableData_free(visit_handle);
int VisIt_VariableData_setDataC(visit_handle obj, int owner, int nComps, int nTuples, char *);
int VisIt_VariableData_setDataI(visit_handle obj, int owner, int nComps, int nTuples, int *);
int VisIt_VariableData_setDataF(visit_handle obj, int owner, int nComps, int nTuples, float *);
int VisIt_VariableData_setDataD(visit_handle obj, int owner, int nComps, int nTuples, double *);

#ifdef __cplusplus
}
#endif

#endif
