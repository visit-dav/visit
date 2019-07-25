// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_VARIABLE_DATA_H
#define VISIT_VARIABLE_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_VariableData_alloc(visit_handle*);
int VisIt_VariableData_free(visit_handle);

/* Pass data (contiguous version) */
int VisIt_VariableData_setDataC(visit_handle obj, int owner, int nComps, int nTuples, char *);
int VisIt_VariableData_setDataI(visit_handle obj, int owner, int nComps, int nTuples, int *);
int VisIt_VariableData_setDataL(visit_handle obj, int owner, int nComps, int nTuples, long *);
int VisIt_VariableData_setDataF(visit_handle obj, int owner, int nComps, int nTuples, float *);
int VisIt_VariableData_setDataD(visit_handle obj, int owner, int nComps, int nTuples, double *);

int VisIt_VariableData_setData(visit_handle, int, int, int, int, void *);
int VisIt_VariableData_setDataEx(visit_handle, int, int, int, int, void *, void(*)(void*), void *);

/* Pass data on a per-component basis with strided access. */
int VisIt_VariableData_setArrayDataC(visit_handle obj, int arrIndex, int owner, int nTuples, int byteOffset, int byteStride, char *);
int VisIt_VariableData_setArrayDataI(visit_handle obj, int arrIndex, int owner, int nTuples, int byteOffset, int byteStride, int *);
int VisIt_VariableData_setArrayDataL(visit_handle obj, int arrIndex, int owner, int nTuples, int byteOffset, int byteStride, long *);
int VisIt_VariableData_setArrayDataF(visit_handle obj, int arrIndex, int owner, int nTuples, int byteOffset, int byteStride, float *);
int VisIt_VariableData_setArrayDataD(visit_handle obj, int arrIndex, int owner, int nTuples, int byteOffset, int byteStride, double *);

/* Get data (contiguous version) */
int VisIt_VariableData_getData(visit_handle obj, int *owner, int *dataType, int *nComps, int *nTuples, void **);
int VisIt_VariableData_getDataC(visit_handle obj, int *owner, int *nComps, int *nTuples, char **);
int VisIt_VariableData_getDataI(visit_handle obj, int *owner, int *nComps, int *nTuples, int **);
int VisIt_VariableData_getDataL(visit_handle obj, int *owner, int *nComps, int *nTuples, long **);
int VisIt_VariableData_getDataF(visit_handle obj, int *owner, int *nComps, int *nTuples, float **);
int VisIt_VariableData_getDataD(visit_handle obj, int *owner, int *nComps, int *nTuples, double **);

#ifdef __cplusplus
}
#endif

#endif
