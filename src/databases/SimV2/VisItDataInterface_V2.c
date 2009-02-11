/* This is a temporary file but it contains functions for deleting the various
 * types of objects that are returned by the libsim callback functions. Eventually,
 * these will probably be in a different library.
 */
/*#include "VisItDataInterface_V2.h"*/

#define FREE(ptr) if(ptr != NULL) { free((void*)ptr); ptr = NULL; }

void
VisIt_DataArray_free(VisIt_DataArray *obj)
{
    if (obj == NULL || obj->owner != VISIT_OWNER_VISIT)
        return;

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
}

void
VisIt_NameList_free(VisIt_NameList *obj)
{
    if(obj == NULL)
        return;

    int i;
    for(i = 0; i < obj->numNames; ++i)
        FREE(obj->names[i]);
    FREE(obj->names);
    obj->numNames = 0;
}

/************************************************************************************/
void
VisIt_SimulationControlCommand_free(VisIt_SimulationControlCommand *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);
    FREE(obj->signal);
    FREE(obj->className);
    FREE(obj->parent);
    FREE(obj->text);
    FREE(obj->value);
    FREE(obj->uiType);
}

void
VisIt_MeshMetaData_free(VisIt_MeshMetaData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);
 
    FREE(obj->blockTitle);
    FREE(obj->blockPieceName);

    FREE(obj->groupTitle);
    FREE(obj->groupPieceName);
    FREE(obj->groupIds);

    FREE(obj->units);

    FREE(obj->xLabel);
    FREE(obj->yLabel);
    FREE(obj->zLabel);
}

void
VisIt_ScalarMetaData_free(VisIt_ScalarMetaData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);
    FREE(obj->meshName);
}

void
VisIt_MaterialMetaData_free(VisIt_MaterialMetaData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);
    FREE(obj->meshName);
    if(obj->materialNames != NULL)
    {
        int i;
        for(i = 0; i < obj->numMaterials; ++i)
            FREE(obj->materialNames[i]);
        FREE(obj->materialNames);
    }
}

void
VisIt_CurveMetaData_free(VisIt_CurveMetaData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);

    FREE(obj->xUnits);
    FREE(obj->yUnits);

    FREE(obj->xLabel);
    FREE(obj->yLabel);
}

void
VisIt_ExpressionMetaData_free(VisIt_ExpressionMetaData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);
    FREE(obj->definition);
}

void
VisIt_SpeciesMetaData_free(VisIt_SpeciesMetaData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->name);
    FREE(obj->meshName);
    FREE(obj->materialName);
    VisIt_NameList_free(obj->materialSpeciesNames);
    FREE(obj->materialSpeciesNames);
}

void
VisIt_SimulationMetaData_free(VisIt_SimulationMetaData *obj)
{
    int i;

    if(obj == NULL)
        return;

    if(obj->meshes != NULL)
    {
        for(i = 0; i < obj->numMeshes; ++i)
            VisIt_MeshMetaData_free(&obj->meshes[i]);
        FREE(obj->meshes);
    }

    if(obj->scalars != NULL)
    {
        for(i = 0; i < obj->numScalars; ++i)
            VisIt_ScalarMetaData_free(&obj->scalars[i]);
        FREE(obj->scalars);
    }

    if(obj->materials != NULL)
    {
        for(i = 0; i < obj->numMaterials; ++i)
            VisIt_MaterialMetaData_free(&obj->materials[i]);
        FREE(obj->materials);
    }

    if(obj->curves != NULL)
    {
        for(i = 0; i < obj->numCurves; ++i)
            VisIt_CurveMetaData_free(&obj->curves[i]);
        FREE(obj->curves);
    }

    if(obj->expressions != NULL)
    {
        for(i = 0; i < obj->numExpressions; ++i)
            VisIt_ExpressionMetaData_free(&obj->expressions[i]);
        FREE(obj->expressions);
    }

    if(obj->species != NULL)
    {
        for(i = 0; i < obj->numSpecies; ++i)
            VisIt_SpeciesMetaData_free(&obj->species[i]);
        FREE(obj->species);
    }

    if(obj->genericCommands != NULL)
    {
        for(i = 0; i < obj->numGenericCommands; ++i)
            VisIt_SimulationControlCommand_free(&obj->genericCommands[i]);
        FREE(obj->genericCommands);
    }

    if(obj->customCommands != NULL)
    {
        for(i = 0; i < obj->numGenericCommands; ++i)
            VisIt_SimulationControlCommand_free(&obj->customCommands[i]);
        FREE(obj->customCommands);
    }

    FREE(obj);
}

/************************************************************************************/

void
VisIt_CurvilinearMesh_free(VisIt_CurvilinearMesh *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->xcoords);
    VisIt_DataArray_free(&obj->ycoords);
    VisIt_DataArray_free(&obj->zcoords);

    FREE(obj);
}

void
VisIt_RectilinearMesh_free(VisIt_RectilinearMesh *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->xcoords);
    VisIt_DataArray_free(&obj->ycoords);
    VisIt_DataArray_free(&obj->zcoords);

    FREE(obj);
}

void
VisIt_UnstructuredMesh_free(VisIt_UnstructuredMesh *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->xcoords);
    VisIt_DataArray_free(&obj->ycoords);
    VisIt_DataArray_free(&obj->zcoords);
    VisIt_DataArray_free(&obj->connectivity);

    FREE(obj);
}

void
VisIt_PointMesh_free(VisIt_PointMesh *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->xcoords);
    VisIt_DataArray_free(&obj->ycoords);
    VisIt_DataArray_free(&obj->zcoords);

    FREE(obj);
}

void
VisIt_CSGMesh_free(VisIt_CSGMesh *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->zones.typeflags);
    VisIt_DataArray_free(&obj->zones.leftids);
    VisIt_DataArray_free(&obj->zones.rightids);
    VisIt_DataArray_free(&obj->zones.xform);
    VisIt_DataArray_free(&obj->zones.zonelist);

    VisIt_DataArray_free(&obj->typeflags);
    VisIt_DataArray_free(&obj->coeffs);

    FREE(obj);
}

void
VisIt_MeshData_free(VisIt_MeshData *obj)
{
    if(obj == NULL)
        return;

    switch (obj->meshType)
    {
    case VISIT_MESHTYPE_CURVILINEAR:
        VisIt_CurvilinearMesh_free(obj->cmesh);
        break;
    case VISIT_MESHTYPE_RECTILINEAR:
        VisIt_RectilinearMesh_free(obj->rmesh);
        break;
    case VISIT_MESHTYPE_UNSTRUCTURED:
        VisIt_UnstructuredMesh_free(obj->umesh);
        break;
    case VISIT_MESHTYPE_POINT:
        break;
    case VISIT_MESHTYPE_CSG:
        break;
    default:
        break;
    }
    FREE(obj);
}

void
VisIt_CurveData_free(VisIt_CurveData *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->x);
    VisIt_DataArray_free(&obj->x);

    FREE(obj);
}

void
VisIt_ScalarData_free(VisIt_ScalarData *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->data);

    FREE(obj);
}

void
VisIt_MixedScalarData_free(VisIt_MixedScalarData *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->data);

    FREE(obj);
}

void
VisIt_MaterialData_free(VisIt_MaterialData *obj)
{
    if(obj == NULL)
        return;

    FREE(obj->materialNumbers);

    if(obj->materialNames != NULL)
    {
        int i;
        for(i = 0; i < obj->nMaterials; ++i)
            FREE(obj->materialNames[i]);
        FREE(obj->materialNames);
    }

    VisIt_DataArray_free(&obj->matlist);
    VisIt_DataArray_free(&obj->mix_mat);
    VisIt_DataArray_free(&obj->mix_zone);
    VisIt_DataArray_free(&obj->mix_next);
    VisIt_DataArray_free(&obj->mix_vf);

    FREE(obj);
}

void
VisIt_SpeciesData_free(VisIt_SpeciesData *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->materialSpecies);

    if(obj->materialSpeciesNames != NULL)
    {
        int i;
        for(i = 0; i < obj->nmaterialSpecies; ++i)
            VisIt_NameList_free(&obj->materialSpeciesNames[i]);
        FREE(obj->materialSpeciesNames);
    }

    VisIt_DataArray_free(&obj->speciesMF);
    VisIt_DataArray_free(&obj->species);
    VisIt_DataArray_free(&obj->mixedSpecies);

    FREE(obj);
}

void
VisIt_DomainList_free(VisIt_DomainList *obj)
{
    if(obj == NULL)
        return;

    VisIt_DataArray_free(&obj->myDomains);

    FREE(obj);
}

