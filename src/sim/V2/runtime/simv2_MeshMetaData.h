// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_MESHMETADATA_H
#define SIMV2_MESHMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_MeshMetaData_alloc(visit_handle *obj);
SIMV2_API int simv2_MeshMetaData_free(visit_handle obj);
SIMV2_API int simv2_MeshMetaData_setName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getName(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setMeshType(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getMeshType(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setTopologicalDimension(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getTopologicalDimension(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setSpatialDimension(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getSpatialDimension(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setNumDomains(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNumDomains(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setDomainTitle(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getDomainTitle(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setDomainPieceName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getDomainPieceName(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_addDomainName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getNumDomainName(visit_handle h, int *);
SIMV2_API int simv2_MeshMetaData_getDomainName(visit_handle h, int, char **);
SIMV2_API int simv2_MeshMetaData_setNumGroups(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNumGroups(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setGroupTitle(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getGroupTitle(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_addGroupName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getNumGroupName(visit_handle h, int *);
SIMV2_API int simv2_MeshMetaData_getGroupName(visit_handle h, int, char **);
SIMV2_API int simv2_MeshMetaData_setGroupPieceName(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getGroupPieceName(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_addGroupId(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNumGroupId(visit_handle h, int *);
SIMV2_API int simv2_MeshMetaData_getGroupId(visit_handle h, int, int *);
SIMV2_API int simv2_MeshMetaData_setXUnits(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getXUnits(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setYUnits(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getYUnits(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setZUnits(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getZUnits(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setXLabel(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getXLabel(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setYLabel(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getYLabel(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setZLabel(visit_handle h, const char *);
SIMV2_API int simv2_MeshMetaData_getZLabel(visit_handle h, char **);
SIMV2_API int simv2_MeshMetaData_setCellOrigin(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getCellOrigin(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setNodeOrigin(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getNodeOrigin(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setHasSpatialExtents(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getHasSpatialExtents(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setSpatialExtents(visit_handle h, double[6]);
SIMV2_API int simv2_MeshMetaData_getSpatialExtents(visit_handle h, double[6]);
SIMV2_API int simv2_MeshMetaData_setHasLogicalBounds(visit_handle h, int);
SIMV2_API int simv2_MeshMetaData_getHasLogicalBounds(visit_handle h, int*);
SIMV2_API int simv2_MeshMetaData_setLogicalBounds(visit_handle h, int[3]);
SIMV2_API int simv2_MeshMetaData_getLogicalBounds(visit_handle h, int[3]);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_MeshMetaData_check(visit_handle);

#endif
