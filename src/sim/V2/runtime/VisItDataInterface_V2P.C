#include <VisItDataInterface_V2P.h>
#include <stdlib.h>

/* This is a temporary file but it contains functions for deleting the various
 * types of objects that are returned by the libsim callback functions. Eventually,
 * these will probably be in a different library.
 */
/*#include "VisItDataInterface_V2.h"*/

#define FREE(ptr) if(ptr != NULL) { free((void*)ptr); ptr = NULL; }

int
simv2_DataArray_free(VisIt_DataArray *obj)
{
    if (obj == NULL || obj->owner != VISIT_OWNER_VISIT)
        return VISIT_ERROR;

    switch (obj->dataType)
    {
    case VISIT_DATATYPE_CHAR:
        FREE(obj->cArray);
        break;
    case VISIT_DATATYPE_INT:
        FREE(obj->iArray);
        break;
    case VISIT_DATATYPE_FLOAT:
        FREE(obj->fArray);
        break;
    case VISIT_DATATYPE_DOUBLE:
        FREE(obj->dArray);
        break;
    }

    return VISIT_OKAY;
}

int
simv2_NameList_free(VisIt_NameList *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    int i;
    for(i = 0; i < obj->numNames; ++i)
        FREE(obj->names[i]);
    FREE(obj->names);
    obj->numNames = 0;

    return VISIT_OKAY;
}

/************************************************************************************/
int
simv2_SimulationControlCommand_free(VisIt_SimulationControlCommand *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);
    FREE(obj->signal);
    FREE(obj->className);
    FREE(obj->parent);
    FREE(obj->text);
    FREE(obj->value);
    FREE(obj->uiType);

    return VISIT_OKAY;
}

int
simv2_MeshMetaData_free(VisIt_MeshMetaData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);
 
    FREE(obj->blockTitle);
    FREE(obj->blockPieceName);
    if(obj->blockNames != NULL)
    {
        int i;
        for(i = 0; i < obj->numBlocks; ++i)
            FREE(obj->blockNames[i]);
        FREE(obj->blockNames);
    }

    FREE(obj->groupTitle);
    FREE(obj->groupPieceName);
    FREE(obj->groupIds);

    FREE(obj->units);

    FREE(obj->xLabel);
    FREE(obj->yLabel);
    FREE(obj->zLabel);

    return VISIT_OKAY;
}

int
simv2_VariableMetaData_free(VisIt_VariableMetaData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);
    FREE(obj->meshName);

    return VISIT_OKAY;
}

int
simv2_MaterialMetaData_free(VisIt_MaterialMetaData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);
    FREE(obj->meshName);
    if(obj->materialNames != NULL)
    {
        int i;
        for(i = 0; i < obj->numMaterials; ++i)
            FREE(obj->materialNames[i]);
        FREE(obj->materialNames);
    }

    return VISIT_OKAY;
}

int
simv2_CurveMetaData_free(VisIt_CurveMetaData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);

    FREE(obj->xUnits);
    FREE(obj->yUnits);

    FREE(obj->xLabel);
    FREE(obj->yLabel);

    return VISIT_OKAY;
}

int
simv2_ExpressionMetaData_free(VisIt_ExpressionMetaData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);
    FREE(obj->definition);

    return VISIT_OKAY;
}

int
simv2_SpeciesMetaData_free(VisIt_SpeciesMetaData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->name);
    FREE(obj->meshName);
    FREE(obj->materialName);
    simv2_NameList_free(obj->materialSpeciesNames);
    FREE(obj->materialSpeciesNames);

    return VISIT_OKAY;
}

int
simv2_SimulationMetaData_free(VisIt_SimulationMetaData *obj)
{
    int i;

    if(obj == NULL)
        return VISIT_ERROR;

    if(obj->meshes != NULL)
    {
        for(i = 0; i < obj->numMeshes; ++i)
            simv2_MeshMetaData_free(&obj->meshes[i]);
        FREE(obj->meshes);
    }

    if(obj->variables != NULL)
    {
        for(i = 0; i < obj->numVariables; ++i)
            simv2_VariableMetaData_free(&obj->variables[i]);
        FREE(obj->variables);
    }

    if(obj->materials != NULL)
    {
        for(i = 0; i < obj->numMaterials; ++i)
            simv2_MaterialMetaData_free(&obj->materials[i]);
        FREE(obj->materials);
    }

    if(obj->curves != NULL)
    {
        for(i = 0; i < obj->numCurves; ++i)
            simv2_CurveMetaData_free(&obj->curves[i]);
        FREE(obj->curves);
    }

    if(obj->expressions != NULL)
    {
        for(i = 0; i < obj->numExpressions; ++i)
            simv2_ExpressionMetaData_free(&obj->expressions[i]);
        FREE(obj->expressions);
    }

    if(obj->species != NULL)
    {
        for(i = 0; i < obj->numSpecies; ++i)
            simv2_SpeciesMetaData_free(&obj->species[i]);
        FREE(obj->species);
    }

    if(obj->genericCommands != NULL)
    {
        for(i = 0; i < obj->numGenericCommands; ++i)
            simv2_SimulationControlCommand_free(&obj->genericCommands[i]);
        FREE(obj->genericCommands);
    }

    if(obj->customCommands != NULL)
    {
        for(i = 0; i < obj->numGenericCommands; ++i)
            simv2_SimulationControlCommand_free(&obj->customCommands[i]);
        FREE(obj->customCommands);
    }

    FREE(obj);

    return VISIT_OKAY;
}

/************************************************************************************/

int
simv2_CurvilinearMesh_free(VisIt_CurvilinearMesh *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->xcoords);
    simv2_DataArray_free(&obj->ycoords);
    simv2_DataArray_free(&obj->zcoords);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_RectilinearMesh_free(VisIt_RectilinearMesh *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->xcoords);
    simv2_DataArray_free(&obj->ycoords);
    simv2_DataArray_free(&obj->zcoords);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_UnstructuredMesh_free(VisIt_UnstructuredMesh *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->xcoords);
    simv2_DataArray_free(&obj->ycoords);
    simv2_DataArray_free(&obj->zcoords);
    simv2_DataArray_free(&obj->connectivity);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_PointMesh_free(VisIt_PointMesh *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->xcoords);
    simv2_DataArray_free(&obj->ycoords);
    simv2_DataArray_free(&obj->zcoords);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_CSGMesh_free(VisIt_CSGMesh *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->zones.typeflags);
    simv2_DataArray_free(&obj->zones.leftids);
    simv2_DataArray_free(&obj->zones.rightids);
    simv2_DataArray_free(&obj->zones.xform);
    simv2_DataArray_free(&obj->zones.zonelist);

    simv2_DataArray_free(&obj->typeflags);
    simv2_DataArray_free(&obj->coeffs);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_MeshData_free(VisIt_MeshData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    switch (obj->meshType)
    {
    case VISIT_MESHTYPE_CURVILINEAR:
        simv2_CurvilinearMesh_free(obj->cmesh);
        break;
    case VISIT_MESHTYPE_RECTILINEAR:
        simv2_RectilinearMesh_free(obj->rmesh);
        break;
    case VISIT_MESHTYPE_UNSTRUCTURED:
        simv2_UnstructuredMesh_free(obj->umesh);
        break;
    case VISIT_MESHTYPE_POINT:
        simv2_PointMesh_free(obj->pmesh);
        break;
    case VISIT_MESHTYPE_CSG:
        simv2_CSGMesh_free(obj->csgmesh);
        break;
    default:
        break;
    }
    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_CurveData_free(VisIt_CurveData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->x);
    simv2_DataArray_free(&obj->y);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_MaterialData_free(VisIt_MaterialData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    FREE(obj->materialNumbers);

    if(obj->materialNames != NULL)
    {
        int i;
        for(i = 0; i < obj->nMaterials; ++i)
            FREE(obj->materialNames[i]);
        FREE(obj->materialNames);
    }

    simv2_DataArray_free(&obj->matlist);
    simv2_DataArray_free(&obj->mix_mat);
    simv2_DataArray_free(&obj->mix_zone);
    simv2_DataArray_free(&obj->mix_next);
    simv2_DataArray_free(&obj->mix_vf);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_SpeciesData_free(VisIt_SpeciesData *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->materialSpecies);

    if(obj->materialSpeciesNames != NULL)
    {
        int i;
        for(i = 0; i < obj->nmaterialSpecies; ++i)
            simv2_NameList_free(&obj->materialSpeciesNames[i]);
        FREE(obj->materialSpeciesNames);
    }

    simv2_DataArray_free(&obj->speciesMF);
    simv2_DataArray_free(&obj->species);
    simv2_DataArray_free(&obj->mixedSpecies);

    FREE(obj);

    return VISIT_OKAY;
}

int
simv2_DomainList_free(VisIt_DomainList *obj)
{
    if(obj == NULL)
        return VISIT_ERROR;

    simv2_DataArray_free(&obj->myDomains);

    FREE(obj);

    return VISIT_OKAY;
}

