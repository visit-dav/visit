#ifndef VISIT_DATA_INTERFACE_PRIVATE_H
#define VISIT_DATA_INTERFACE_PRIVATE_H
/* This will be a short-lived file since we'll eventually split these 
 * functions into separate headers as they multiply. For example, simV2_CurvilinearMesh.{C,h}
 * would contain the functions for creating,freeing,and manipulating the
 * CurvilinearMesh data type.
 */

#include <VisItDataInterface_V2.h>
#include <VisItSimV2Exports.h>

int SIMV2_API simv2_DataArray_free(VisIt_DataArray *obj);
int SIMV2_API simv2_NameList_free(VisIt_NameList *obj);
int SIMV2_API simv2_SimulationControlCommand_free(VisIt_SimulationControlCommand *obj);
int SIMV2_API simv2_MeshMetaData_free(VisIt_MeshMetaData *obj);
int SIMV2_API simv2_VariableMetaData_free(VisIt_VariableMetaData *obj);
int SIMV2_API simv2_MaterialMetaData_free(VisIt_MaterialMetaData *obj);
int SIMV2_API simv2_CurveMetaData_free(VisIt_CurveMetaData *obj);
int SIMV2_API simv2_ExpressionMetaData_free(VisIt_ExpressionMetaData *obj);
int SIMV2_API simv2_SpeciesMetaData_free(VisIt_SpeciesMetaData *obj);
int SIMV2_API simv2_SimulationMetaData_free(VisIt_SimulationMetaData *obj);

int SIMV2_API simv2_CurvilinearMesh_free(VisIt_CurvilinearMesh *obj);
int SIMV2_API simv2_RectilinearMesh_free(VisIt_RectilinearMesh *obj);
int SIMV2_API simv2_UnstructuredMesh_free(VisIt_UnstructuredMesh *obj);
int SIMV2_API simv2_PointMesh_free(VisIt_PointMesh *obj);
int SIMV2_API simv2_CSGMesh_free(VisIt_CSGMesh *obj);
int SIMV2_API simv2_MeshData_free(VisIt_MeshData *obj);
int SIMV2_API simv2_CurveData_free(VisIt_CurveData *obj);
int SIMV2_API simv2_VariableData_free(VisIt_VariableData *obj);
int SIMV2_API simv2_MixedVariableData_free(VisIt_MixedVariableData *obj);
int SIMV2_API simv2_MaterialData_free(VisIt_MaterialData *obj);
int SIMV2_API simv2_SpeciesData_free(VisIt_SpeciesData *obj);
int SIMV2_API simv2_DomainList_free(VisIt_DomainList *obj);

#endif
