// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MESHMETADATA_H
#define VISIT_MESHMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_MeshMetaData_alloc(visit_handle *obj);
int VisIt_MeshMetaData_free(visit_handle obj);
int VisIt_MeshMetaData_setName(visit_handle h, const char *);
int VisIt_MeshMetaData_getName(visit_handle h, char **);
int VisIt_MeshMetaData_setMeshType(visit_handle h, int);
int VisIt_MeshMetaData_getMeshType(visit_handle h, int*);
int VisIt_MeshMetaData_setTopologicalDimension(visit_handle h, int);
int VisIt_MeshMetaData_getTopologicalDimension(visit_handle h, int*);
int VisIt_MeshMetaData_setSpatialDimension(visit_handle h, int);
int VisIt_MeshMetaData_getSpatialDimension(visit_handle h, int*);
int VisIt_MeshMetaData_setNumDomains(visit_handle h, int);
int VisIt_MeshMetaData_getNumDomains(visit_handle h, int*);
int VisIt_MeshMetaData_setDomainTitle(visit_handle h, const char *);
int VisIt_MeshMetaData_getDomainTitle(visit_handle h, char **);
int VisIt_MeshMetaData_setDomainPieceName(visit_handle h, const char *);
int VisIt_MeshMetaData_getDomainPieceName(visit_handle h, char **);
int VisIt_MeshMetaData_addDomainName(visit_handle h, const char *);
int VisIt_MeshMetaData_getNumDomainName(visit_handle h, int *);
int VisIt_MeshMetaData_getDomainName(visit_handle h, int, char **);
int VisIt_MeshMetaData_setNumGroups(visit_handle h, int);
int VisIt_MeshMetaData_getNumGroups(visit_handle h, int*);
int VisIt_MeshMetaData_setGroupTitle(visit_handle h, const char *);
int VisIt_MeshMetaData_getGroupTitle(visit_handle h, char **);
int VisIt_MeshMetaData_addGroupName(visit_handle h, const char *);
int VisIt_MeshMetaData_setGroupPieceName(visit_handle h, const char *);
int VisIt_MeshMetaData_getGroupPieceName(visit_handle h, char **);
int VisIt_MeshMetaData_addGroupId(visit_handle h, int);
int VisIt_MeshMetaData_getNumGroupId(visit_handle h, int *);
int VisIt_MeshMetaData_getGroupId(visit_handle h, int, int *);
int VisIt_MeshMetaData_setXUnits(visit_handle h, const char *);
int VisIt_MeshMetaData_getXUnits(visit_handle h, char **);
int VisIt_MeshMetaData_setYUnits(visit_handle h, const char *);
int VisIt_MeshMetaData_getYUnits(visit_handle h, char **);
int VisIt_MeshMetaData_setZUnits(visit_handle h, const char *);
int VisIt_MeshMetaData_getZUnits(visit_handle h, char **);
int VisIt_MeshMetaData_setXLabel(visit_handle h, const char *);
int VisIt_MeshMetaData_getXLabel(visit_handle h, char **);
int VisIt_MeshMetaData_setYLabel(visit_handle h, const char *);
int VisIt_MeshMetaData_getYLabel(visit_handle h, char **);
int VisIt_MeshMetaData_setZLabel(visit_handle h, const char *);
int VisIt_MeshMetaData_getZLabel(visit_handle h, char **);
int VisIt_MeshMetaData_setCellOrigin(visit_handle h, int);
int VisIt_MeshMetaData_getCellOrigin(visit_handle h, int*);
int VisIt_MeshMetaData_setNodeOrigin(visit_handle h, int);
int VisIt_MeshMetaData_getNodeOrigin(visit_handle h, int*);
int VisIt_MeshMetaData_setHasSpatialExtents(visit_handle h, int);
int VisIt_MeshMetaData_getHasSpatialExtents(visit_handle h, int*);
int VisIt_MeshMetaData_setSpatialExtents(visit_handle h, double[6]);
int VisIt_MeshMetaData_getSpatialExtents(visit_handle h, double[6]);
int VisIt_MeshMetaData_setHasLogicalBounds(visit_handle h, int);
int VisIt_MeshMetaData_getHasLogicalBounds(visit_handle h, int*);
int VisIt_MeshMetaData_setLogicalBounds(visit_handle h, int[3]);
int VisIt_MeshMetaData_getLogicalBounds(visit_handle h, int[3]);

#ifdef __cplusplus
}
#endif

#endif
