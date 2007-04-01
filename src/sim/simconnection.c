#include "sim.h"

#include <VisItDataInterface_V1.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VisIt_SimulationMetaData *VisItGetMetaData()
{
    fprintf(stderr, "VisItGetMetaData\n");

    VisIt_SimulationMetaData *md = malloc(sizeof(VisIt_SimulationMetaData));
    md->currentCycle = cycle;
    md->currentTime  = 0;

    md->numMeshes = 1;
    md->meshes = malloc(sizeof(VisIt_MeshMetaData) * md->numMeshes);

    md->meshes[0].name = strdup("mesh");
    md->meshes[0].meshType = VISIT_MESHTYPE_CURVILINEAR;
    md->meshes[0].topologicalDimension = 3;
    md->meshes[0].spatialDimension = 3;
    md->meshes[0].numBlocks = numdomains;
    md->meshes[0].blockTitle = strdup("Domains");
    md->meshes[0].blockPieceName = strdup("domain");
    md->meshes[0].numGroups = 0;
    md->meshes[0].units = NULL;

    md->numScalars = 2;
    md->scalars = malloc(sizeof(VisIt_ScalarMetaData) * md->numScalars);

    md->scalars[0].name = strdup("speed");
    md->scalars[0].meshName = strdup("mesh");
    md->scalars[0].centering = VISIT_VARCENTERING_NODE;
    md->scalars[0].treatAsASCII = 0;

    md->scalars[1].name = strdup("density");
    md->scalars[1].meshName = strdup("mesh");
    md->scalars[1].centering = VISIT_VARCENTERING_ZONE;
    md->scalars[1].treatAsASCII = 0;

    return md;
}

VisIt_MeshData *VisItGetMesh(int domain,const char *name)
{
    VisIt_MeshData *mesh = malloc(sizeof(VisIt_MeshData));
    mesh->meshType = VISIT_MESHTYPE_CURVILINEAR;
    mesh->cmesh = malloc(sizeof(VisIt_CurvilinearMesh));
    
    mesh->cmesh->dims[0] = p_nx;
    mesh->cmesh->dims[1] = p_ny;
    mesh->cmesh->dims[2] = p_nz;
    mesh->cmesh->xcoords = p_xcoords;
    mesh->cmesh->ycoords = p_ycoords;
    mesh->cmesh->zcoords = p_zcoords;

    return mesh;
}

VisIt_MaterialData *VisItGetMaterial(int domain,const char *name)
{
    fprintf(stderr, "VisItGetMaterial\n");
    return NULL;
}

VisIt_ScalarData *VisItGetScalar(int domain,const char *name)
{
    if (strcmp(name,"density")==0)
    {
        VisIt_ScalarData *sd = malloc(sizeof(VisIt_ScalarData));
        sd->len  = (p_nx-1)*(p_ny-1)*(p_nz-1);
        sd->data = p_zvalues;
        return sd;
    }
    else if (strcmp(name,"speed")==0)
    {
        VisIt_ScalarData *sd = malloc(sizeof(VisIt_ScalarData));
        sd->len  = (p_nx)*(p_ny)*(p_nz);
        sd->data = p_nvalues;
        return sd;
    }

    return NULL;
}


VisIt_SimulationCallback visitCallbacks =
{
    &VisItGetMetaData,
    &VisItGetMesh,
    &VisItGetMaterial,
    &VisItGetScalar,
    NULL
};
