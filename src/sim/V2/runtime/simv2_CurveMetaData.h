// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_CURVEMETADATA_H
#define SIMV2_CURVEMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_CurveMetaData_alloc(visit_handle *obj);
SIMV2_API int simv2_CurveMetaData_free(visit_handle obj);
SIMV2_API int simv2_CurveMetaData_setName(visit_handle h, const char *);
SIMV2_API int simv2_CurveMetaData_getName(visit_handle h, char **);
SIMV2_API int simv2_CurveMetaData_setXUnits(visit_handle h, const char *);
SIMV2_API int simv2_CurveMetaData_getXUnits(visit_handle h, char **);
SIMV2_API int simv2_CurveMetaData_setYUnits(visit_handle h, const char *);
SIMV2_API int simv2_CurveMetaData_getYUnits(visit_handle h, char **);
SIMV2_API int simv2_CurveMetaData_setXLabel(visit_handle h, const char *);
SIMV2_API int simv2_CurveMetaData_getXLabel(visit_handle h, char **);
SIMV2_API int simv2_CurveMetaData_setYLabel(visit_handle h, const char *);
SIMV2_API int simv2_CurveMetaData_getYLabel(visit_handle h, char **);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_CurveMetaData_check(visit_handle);

#endif
