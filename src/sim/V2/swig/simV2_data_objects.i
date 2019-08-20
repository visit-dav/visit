// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_COMMANDMETADATA_H
#define VISIT_COMMANDMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_CommandMetaData_alloc(visit_handle *obj);
int VisIt_CommandMetaData_free(visit_handle obj);
int VisIt_CommandMetaData_setName(visit_handle h, const char *);
int VisIt_CommandMetaData_setEnabled(visit_handle h, int);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CSG_MESH_H
#define VISIT_CSG_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_CSGMesh_alloc(visit_handle *obj);
int VisIt_CSGMesh_free(visit_handle obj);
int VisIt_CSGMesh_setRegions(visit_handle obj, visit_handle boolops, visit_handle leftids, visit_handle rightids);
int VisIt_CSGMesh_setZonelist(visit_handle obj, visit_handle zl);
int VisIt_CSGMesh_setBoundaryTypes(visit_handle obj, visit_handle csgtypes);
int VisIt_CSGMesh_setBoundaryCoeffs(visit_handle obj, visit_handle coeffs);
int VisIt_CSGMesh_setExtents(visit_handle obj, double min[3], double max[3]);


#ifdef __cplusplus
}
#endif

#endif
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
int VisIt_CurveMetaData_setXUnits(visit_handle h, const char *);
int VisIt_CurveMetaData_setYUnits(visit_handle h, const char *);
int VisIt_CurveMetaData_setXLabel(visit_handle h, const char *);
int VisIt_CurveMetaData_setYLabel(visit_handle h, const char *);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CURVILINEAR_MESH_H
#define VISIT_CURVILINEAR_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_CurvilinearMesh_alloc(visit_handle *obj);
int VisIt_CurvilinearMesh_free(visit_handle obj);
int VisIt_CurvilinearMesh_setCoordsXY(visit_handle obj, int dims[2], visit_handle x, visit_handle y);
int VisIt_CurvilinearMesh_setCoordsXYZ(visit_handle obj, int dims[3], visit_handle x, visit_handle y, visit_handle z);
int VisIt_CurvilinearMesh_setCoords2(visit_handle obj, int dims[2], visit_handle c);
int VisIt_CurvilinearMesh_setCoords3(visit_handle obj, int dims[3], visit_handle c);
int VisIt_CurvilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3]);
int VisIt_CurvilinearMesh_setBaseIndex(visit_handle obj, int base_index[3]);
int VisIt_CurvilinearMesh_setGhostCells(visit_handle obj, visit_handle gz);
int VisIt_CurvilinearMesh_setGhostNodes(visit_handle obj, visit_handle gn);


#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DOMAINBOUNDARIES_H
#define VISIT_DOMAINBOUNDARIES_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainBoundaries_alloc(visit_handle*);
int VisIt_DomainBoundaries_free(visit_handle);
/* Pass 0=rectilinear, 1=curvilinear for the type. */
int VisIt_DomainBoundaries_set_type(visit_handle, int);
int VisIt_DomainBoundaries_set_numDomains(visit_handle, int);

/* Set extents for structured mesh. */
int VisIt_DomainBoundaries_set_rectIndices(visit_handle, int dom, const int e[6]);

/* Set extents for an AMR patch. */
int VisIt_DomainBoundaries_set_amrIndices(visit_handle, int patch, int level, const int e[6]);

/* Functions to add a custom number of neighbors for a domain. */
int VisIt_DomainBoundaries_set_extents(visit_handle, int dom, const int e[6]);
int VisIt_DomainBoundaries_add_neighbor(visit_handle, int dom, int d, int mi, 
                                        const int orientation[3], const int extents[6]);
int VisIt_DomainBoundaries_finish(visit_handle, int dom);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DOMAINLIST_H
#define VISIT_DOMAINLIST_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainList_alloc(visit_handle *obj);
int VisIt_DomainList_free(visit_handle obj);
int VisIt_DomainList_setDomains(visit_handle obj, int alldoms,
                                visit_handle mydoms);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DOMAINNESTING_H
#define VISIT_DOMAINNESTING_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainNesting_alloc(visit_handle*);
int VisIt_DomainNesting_free(visit_handle);
int VisIt_DomainNesting_set_dimensions(visit_handle,int,int,int);
int VisIt_DomainNesting_set_levelRefinement(visit_handle, int, int[3]);
int VisIt_DomainNesting_set_nestingForPatch(visit_handle, int, int, const int *, int, int[6]);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_EXPRESSIONMETADATA_H
#define VISIT_EXPRESSIONMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_ExpressionMetaData_alloc(visit_handle *obj);
int VisIt_ExpressionMetaData_free(visit_handle obj);
int VisIt_ExpressionMetaData_setName(visit_handle h, const char *);
int VisIt_ExpressionMetaData_setDefinition(visit_handle h, const char *);
int VisIt_ExpressionMetaData_setType(visit_handle h, int);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MATERIALDATA_H
#define VISIT_MATERIALDATA_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_MaterialData_alloc(visit_handle*);
int VisIt_MaterialData_free(visit_handle);

int VisIt_MaterialData_addMaterial(visit_handle obj, 
                                   const char *matName, int *matno);

/* Add materials 1 cell at a time... */
int VisIt_MaterialData_appendCells(visit_handle h, int ncells);

int VisIt_MaterialData_addCleanCell(visit_handle h, int cell, int matno);

int VisIt_MaterialData_addMixedCell(visit_handle h, int cell, 
        const int *matnos, const float *mixvf, int nmats);

/* Or add materials all at once */
int VisIt_MaterialData_setMaterials(visit_handle obj, 
                                    visit_handle matlist);

int VisIt_MaterialData_setMixedMaterials(visit_handle obj,
        visit_handle mix_mat, visit_handle mix_zone, 
        visit_handle mix_next, visit_handle mix_vf);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MATERIALMETADATA_H
#define VISIT_MATERIALMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_MaterialMetaData_alloc(visit_handle *obj);
int VisIt_MaterialMetaData_free(visit_handle obj);
int VisIt_MaterialMetaData_setName(visit_handle h, const char *);
int VisIt_MaterialMetaData_setMeshName(visit_handle h, const char *);
int VisIt_MaterialMetaData_addMaterialName(visit_handle h, const char *);

#ifdef __cplusplus
}
#endif

#endif
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
int VisIt_MeshMetaData_setMeshType(visit_handle h, int);
int VisIt_MeshMetaData_setTopologicalDimension(visit_handle h, int);
int VisIt_MeshMetaData_setSpatialDimension(visit_handle h, int);
int VisIt_MeshMetaData_setNumDomains(visit_handle h, int);
int VisIt_MeshMetaData_setDomainTitle(visit_handle h, const char *);
int VisIt_MeshMetaData_setDomainPieceName(visit_handle h, const char *);
int VisIt_MeshMetaData_addDomainName(visit_handle h, const char *);
int VisIt_MeshMetaData_setNumGroups(visit_handle h, int);
int VisIt_MeshMetaData_setGroupTitle(visit_handle h, const char *);
int VisIt_MeshMetaData_addGroupName(visit_handle h, const char *);
int VisIt_MeshMetaData_setGroupPieceName(visit_handle h, const char *);
int VisIt_MeshMetaData_addGroupId(visit_handle h, int);
int VisIt_MeshMetaData_setXUnits(visit_handle h, const char *);
int VisIt_MeshMetaData_setYUnits(visit_handle h, const char *);
int VisIt_MeshMetaData_setZUnits(visit_handle h, const char *);
int VisIt_MeshMetaData_setXLabel(visit_handle h, const char *);
int VisIt_MeshMetaData_setYLabel(visit_handle h, const char *);
int VisIt_MeshMetaData_setZLabel(visit_handle h, const char *);
int VisIt_MeshMetaData_setCellOrigin(visit_handle h, int);
int VisIt_MeshMetaData_setNodeOrigin(visit_handle h, int);
int VisIt_MeshMetaData_setHasSpatialExtents(visit_handle h, int);
int VisIt_MeshMetaData_setSpatialExtents(visit_handle h, double[6]);
int VisIt_MeshMetaData_setHasLogicalBounds(visit_handle h, int);
int VisIt_MeshMetaData_setLogicalBounds(visit_handle h, int[3]);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_NAMELIST_H
#define VISIT_NAMELIST_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_NameList_alloc(visit_handle *obj);
int VisIt_NameList_free(visit_handle obj);
int VisIt_NameList_addName(visit_handle h, const char *);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_OPTIONLIST_H
#define VISIT_OPTIONLIST_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_OptionList_alloc(visit_handle *obj);
int VisIt_OptionList_free(visit_handle obj);
int VisIt_OptionList_setValueB(visit_handle h, const char *, int);
int VisIt_OptionList_setValueI(visit_handle h, const char *, int);
int VisIt_OptionList_setValueF(visit_handle h, const char *, float);
int VisIt_OptionList_setValueD(visit_handle h, const char *, double);
int VisIt_OptionList_setValueS(visit_handle h, const char *, const char *);


#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_POINTMESH_H
#define VISIT_POINTMESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_PointMesh_alloc(visit_handle *obj);
int VisIt_PointMesh_free(visit_handle obj);
int VisIt_PointMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
int VisIt_PointMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
int VisIt_PointMesh_setCoords(visit_handle obj, visit_handle c);


#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_RECTILINEAR_MESH_H
#define VISIT_RECTILINEAR_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_RectilinearMesh_alloc(visit_handle *obj);
int VisIt_RectilinearMesh_free(visit_handle obj);
int VisIt_RectilinearMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
int VisIt_RectilinearMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
int VisIt_RectilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3]);
int VisIt_RectilinearMesh_setBaseIndex(visit_handle obj, int base_index[3]);
int VisIt_RectilinearMesh_setGhostCells(visit_handle obj, visit_handle gz);
int VisIt_RectilinearMesh_setGhostNodes(visit_handle obj, visit_handle gn);


#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_SIMULATIONMETADATA_H
#define VISIT_SIMULATIONMETADATA_H
#include <VisItInterfaceTypes_V2.h>

/* C-callable implementation of front end functions */
#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_SimulationMetaData_alloc(visit_handle*);
int VisIt_SimulationMetaData_free(visit_handle);

int VisIt_SimulationMetaData_setMode(visit_handle h, int mode);
int VisIt_SimulationMetaData_setCycleTime(visit_handle h, int cycle, double time);
int VisIt_SimulationMetaData_addMesh(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addVariable(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addMaterial(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addCurve(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addExpression(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addSpecies(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addGenericCommand(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addCustomCommand(visit_handle h, visit_handle obj);

int VisIt_SimulationMetaData_addMessage(visit_handle h, visit_handle obj);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_SPECIESDATA_H
#define VISIT_SPECIESDATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_SpeciesData_alloc(visit_handle *obj);
int VisIt_SpeciesData_free(visit_handle obj);
int VisIt_SpeciesData_addSpeciesName(visit_handle h, visit_handle);
int VisIt_SpeciesData_setSpecies(visit_handle h, visit_handle);
int VisIt_SpeciesData_setSpeciesMF(visit_handle h, visit_handle);
int VisIt_SpeciesData_setMixedSpecies(visit_handle h, visit_handle);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_SPECIESMETADATA_H
#define VISIT_SPECIESMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_SpeciesMetaData_alloc(visit_handle *obj);
int VisIt_SpeciesMetaData_free(visit_handle obj);
int VisIt_SpeciesMetaData_setName(visit_handle h, const char *);
int VisIt_SpeciesMetaData_setMeshName(visit_handle h, const char *);
int VisIt_SpeciesMetaData_setMaterialName(visit_handle h, const char *);
int VisIt_SpeciesMetaData_addSpeciesName(visit_handle h, visit_handle);

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_UNSTRUCTURED_MESH_H
#define VISIT_UNSTRUCTURED_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_UnstructuredMesh_alloc(visit_handle *obj);
int VisIt_UnstructuredMesh_free(visit_handle obj);
int VisIt_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
int VisIt_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
int VisIt_UnstructuredMesh_setCoords(visit_handle obj, visit_handle c);
int VisIt_UnstructuredMesh_setConnectivity(visit_handle obj, int nCells, visit_handle c);
int VisIt_UnstructuredMesh_setRealIndices(visit_handle obj, int min, int max);
int VisIt_UnstructuredMesh_setGhostCells(visit_handle obj, visit_handle gz);
int VisIt_UnstructuredMesh_setGhostNodes(visit_handle obj, visit_handle gn);
int VisIt_UnstructuredMesh_setGlobalCellIds(visit_handle obj, visit_handle glz);
int VisIt_UnstructuredMesh_setGlobalNodeIds(visit_handle obj, visit_handle gln);


#ifdef __cplusplus
}
#endif

#endif
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

#ifdef __cplusplus
}
#endif

#endif
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_VARIABLEMETADATA_H
#define VISIT_VARIABLEMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_VariableMetaData_alloc(visit_handle *obj);
int VisIt_VariableMetaData_free(visit_handle obj);
int VisIt_VariableMetaData_setName(visit_handle h, const char *);
int VisIt_VariableMetaData_setMeshName(visit_handle h, const char *);
int VisIt_VariableMetaData_setUnits(visit_handle h, const char *);
int VisIt_VariableMetaData_setCentering(visit_handle h, int);
int VisIt_VariableMetaData_setType(visit_handle h, int);
int VisIt_VariableMetaData_setTreatAsASCII(visit_handle h, int);
int VisIt_VariableMetaData_setHideFromGUI(visit_handle h, int);
int VisIt_VariableMetaData_setNumComponents(visit_handle h, int);
int VisIt_VariableMetaData_addMaterialName(visit_handle h, const char *);
int VisIt_VariableMetaData_setEnumerationType(visit_handle h, int);
int VisIt_VariableMetaData_addEnumGraphEdge(visit_handle h, int, int, const char *);
int VisIt_VariableMetaData_addEnumNameValue(visit_handle h, const char *, double, int *);
int VisIt_VariableMetaData_addEnumNameRange(visit_handle h, const char *, double, double, int *);
int VisIt_VariableMetaData_setEnumAlwaysExcludeValue(visit_handle h, double);
int VisIt_VariableMetaData_setEnumAlwaysExcludeRange(visit_handle h, double, double);
int VisIt_VariableMetaData_setEnumAlwaysIncludeValue(visit_handle h, double);
int VisIt_VariableMetaData_setEnumAlwaysIncludeRange(visit_handle h, double, double);
  
#ifdef __cplusplus
}
#endif

#endif
