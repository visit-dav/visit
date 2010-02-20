#include <VisItDataInterface_V2.h>
#include <VisItDynamic.h>

/* This file should eventually be partitioned into separate files representing
 * the objects that are represented here.
 */

int
VisIt_DataArray_free(VisIt_DataArray *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_DataArray_free,
                    int (*)(VisIt_DataArray*),
                    int (*cb)(VisIt_DataArray*),
                    (*cb)(obj))
}

int
VisIt_NameList_free(VisIt_NameList *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_NameList_free,
                    int (*)(VisIt_NameList*),
                    int (*cb)(VisIt_NameList*),
                    (*cb)(obj))
}

/************************************************************************************/
int
VisIt_SimulationControlCommand_free(VisIt_SimulationControlCommand *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_SimulationControlCommand_free,
                    int (*)(VisIt_SimulationControlCommand*),
                    int (*cb)(VisIt_SimulationControlCommand*),
                    (*cb)(obj))
}

int
VisIt_MeshMetaData_free(VisIt_MeshMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_MeshMetaData_free,
                    int (*)(VisIt_MeshMetaData*),
                    int (*cb)(VisIt_MeshMetaData*),
                    (*cb)(obj))
}

int
VisIt_VariableMetaData_free(VisIt_VariableMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_VariableMetaData_free,
                    int (*)(VisIt_VariableMetaData*),
                    int (*cb)(VisIt_VariableMetaData*),
                    (*cb)(obj))
}

int
VisIt_MaterialMetaData_free(VisIt_MaterialMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_MaterialMetaData_free,
                    int (*)(VisIt_MaterialMetaData*),
                    int (*cb)(VisIt_MaterialMetaData*),
                    (*cb)(obj))
}

int
VisIt_CurveMetaData_free(VisIt_CurveMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_CurveMetaData_free,
                    int (*)(VisIt_CurveMetaData*),
                    int (*cb)(VisIt_CurveMetaData*),
                    (*cb)(obj))
}

int
VisIt_ExpressionMetaData_free(VisIt_ExpressionMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_ExpressionMetaData_free,
                    int (*)(VisIt_ExpressionMetaData*),
                    int (*cb)(VisIt_ExpressionMetaData*),
                    (*cb)(obj))
}

int
VisIt_SpeciesMetaData_free(VisIt_SpeciesMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_SpeciesMetaData_free,
                    int (*)(VisIt_SpeciesMetaData*),
                    int (*cb)(VisIt_SpeciesMetaData*),
                    (*cb)(obj))
}


int
VisIt_SimulationMetaData_free(VisIt_SimulationMetaData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_SimulationMetaData_free,
                    int (*)(VisIt_SimulationMetaData*),
                    int (*cb)(VisIt_SimulationMetaData*),
                    (*cb)(obj))
}

/************************************************************************************/

int
VisIt_CurvilinearMesh_free(VisIt_CurvilinearMesh *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_CurvilinearMesh_free,
                    int (*)(VisIt_CurvilinearMesh*),
                    int (*cb)(VisIt_CurvilinearMesh*),
                    (*cb)(obj))
}

int
VisIt_RectilinearMesh_free(VisIt_RectilinearMesh *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_RectilinearMesh_free,
                    int (*)(VisIt_RectilinearMesh*),
                    int (*cb)(VisIt_RectilinearMesh*),
                    (*cb)(obj))
}

int
VisIt_UnstructuredMesh_free(VisIt_UnstructuredMesh *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_UnstructuredMesh_free,
                    int (*)(VisIt_UnstructuredMesh*),
                    int (*cb)(VisIt_UnstructuredMesh*),
                    (*cb)(obj))
}

int
VisIt_PointMesh_free(VisIt_PointMesh *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_PointMesh_free,
                    int (*)(VisIt_PointMesh*),
                    int (*cb)(VisIt_PointMesh*),
                    (*cb)(obj))
}

int
VisIt_CSGMesh_free(VisIt_CSGMesh *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_CSGMesh_free,
                    int (*)(VisIt_CSGMesh*),
                    int (*cb)(VisIt_CSGMesh*),
                    (*cb)(obj))
}

int
VisIt_MeshData_free(VisIt_MeshData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_MeshData_free,
                    int (*)(VisIt_MeshData*),
                    int (*cb)(VisIt_MeshData*),
                    (*cb)(obj))
}

int
VisIt_CurveData_free(VisIt_CurveData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_CurveData_free,
                    int (*)(VisIt_CurveData*),
                    int (*cb)(VisIt_CurveData*),
                    (*cb)(obj))
}

int
VisIt_MaterialData_free(VisIt_MaterialData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_MaterialData_free,
                    int (*)(VisIt_MaterialData*),
                    int (*cb)(VisIt_MaterialData*),
                    (*cb)(obj))
}

int
VisIt_SpeciesData_free(VisIt_SpeciesData *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_SpeciesData_free,
                    int (*)(VisIt_SpeciesData*),
                    int (*cb)(VisIt_SpeciesData*),
                    (*cb)(obj))
}

int
VisIt_DomainList_free(VisIt_DomainList *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainList_free,
                    int (*)(VisIt_DomainList*),
                    int (*cb)(VisIt_DomainList*),
                    (*cb)(obj))
}

