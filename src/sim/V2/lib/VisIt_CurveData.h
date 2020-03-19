// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CURVEDATA_H
#define VISIT_CURVEDATA_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_CurveData_alloc(visit_handle *obj);
int VisIt_CurveData_free(visit_handle obj);
int VisIt_CurveData_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);

#ifdef __cplusplus
}
#endif

#endif
