// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CURVEMETADATA_H
#define VISIT_CURVEMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_CurveMetaData_alloc(visit_handle *obj);
int VisIt_CurveMetaData_free(visit_handle obj);
int VisIt_CurveMetaData_setName(visit_handle h, const char *);
int VisIt_CurveMetaData_getName(visit_handle h, char **);
int VisIt_CurveMetaData_setXUnits(visit_handle h, const char *);
int VisIt_CurveMetaData_getXUnits(visit_handle h, char **);
int VisIt_CurveMetaData_setYUnits(visit_handle h, const char *);
int VisIt_CurveMetaData_getYUnits(visit_handle h, char **);
int VisIt_CurveMetaData_setXLabel(visit_handle h, const char *);
int VisIt_CurveMetaData_getXLabel(visit_handle h, char **);
int VisIt_CurveMetaData_setYLabel(visit_handle h, const char *);
int VisIt_CurveMetaData_getYLabel(visit_handle h, char **);

#ifdef __cplusplus
}
#endif

#endif
