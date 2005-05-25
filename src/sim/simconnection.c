// Programmer: Jeremy Meredith
// Date      : April  4, 2005
//
// Modifications:
//    Jeremy Meredith, Thu Apr 28 18:17:02 PDT 2005
//    Changed it to a rectilinear mesh.
//
//    Jeremy Meredith, Wed May 11 11:05:50 PDT 2005
//    Added ghost zones.  Added domain lists for restricted load balancing.
// 
#include "sim.h"

#include <VisItControlInterface_V1.h>
#include <VisItDataInterface_V1.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VisIt_SimulationMetaData *VisItGetMetaData()
{
    VisIt_SimulationMetaData *md = malloc(sizeof(VisIt_SimulationMetaData));
    md->currentCycle = cycle;
    md->currentTime  = 0;
    md->currentMode  = runflag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;

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
    md->meshes[0].xLabel = NULL;
    md->meshes[0].yLabel = NULL;
    md->meshes[0].zLabel = NULL;

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

    md->numMaterials   = 0;
    md->numCurves      = 0;
    md->numExpressions = 0;

    md->numCommands = 4;
    md->commands = malloc(sizeof(VisIt_SimulationControlCommand) * md->numCommands);

    md->commands[0].name = strdup("halt");
    md->commands[0].argType = VISIT_CMDARG_NONE;
    md->commands[0].enabled = 1;

    md->commands[1].name = strdup("step");
    md->commands[1].argType = VISIT_CMDARG_NONE;
    md->commands[1].enabled = 1;

    md->commands[2].name = strdup("run");
    md->commands[2].argType = VISIT_CMDARG_NONE;
    md->commands[2].enabled = 1;

    md->commands[3].name = strdup("testcommand");
    md->commands[3].argType = VISIT_CMDARG_NONE;
    md->commands[3].enabled = runflag ? 0 : 1;

    return md;
}

VisIt_MeshData *VisItGetMesh(int domain,const char *name)
{
    VisIt_MeshData *mesh = malloc(sizeof(VisIt_MeshData));

    mesh->meshType = VISIT_MESHTYPE_RECTILINEAR;
    mesh->rmesh = malloc(sizeof(VisIt_RectilinearMesh));

    mesh->rmesh->ndims = 3;

    mesh->rmesh->baseIndex[0] = 0;
    mesh->rmesh->baseIndex[1] = 0;
    mesh->rmesh->baseIndex[2] = 0;

    mesh->rmesh->minRealIndex[0] = 0;
    mesh->rmesh->minRealIndex[1] = 0;
    mesh->rmesh->minRealIndex[2] = 0;

    mesh->rmesh->maxRealIndex[0] = p_nx-1;
    mesh->rmesh->maxRealIndex[1] = p_ny-1;
    mesh->rmesh->maxRealIndex[2] = p_nz-1;

    mesh->rmesh->dims[0] = p_nx;
    mesh->rmesh->dims[1] = p_ny;
    mesh->rmesh->dims[2] = p_nz;

    mesh->rmesh->xcoords = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM,p_xcoords);
    mesh->rmesh->ycoords = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM,p_ycoords);
    mesh->rmesh->zcoords = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM,p_zcoords);

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
        sd->data = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM,p_zvalues);
        return sd;
    }
    else if (strcmp(name,"speed")==0)
    {
        VisIt_ScalarData *sd = malloc(sizeof(VisIt_ScalarData));
        sd->len  = (p_nx)*(p_ny)*(p_nz);
        sd->data = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM,p_nvalues);
        return sd;
    }

    return NULL;
}

VisIt_DomainList *VisItGetDomainList()
{
    int i;
    VisIt_DomainList *dl = malloc(sizeof(VisIt_DomainList));

    dl->nTotalDomains = numdomains;

    dl->nMyDomains = 1;
    dl->myDomains = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT,
                                               malloc(sizeof(int)));
    dl->myDomains.iArray[0] = par_rank;

    return dl;
}

VisIt_SimulationCallback visitCallbacks =
{
    &VisItGetMetaData,
    &VisItGetMesh,
    NULL, /* material */
    NULL, /* species */
    &VisItGetScalar,
    NULL, /* curve */
    NULL,  /* mixed scalar */
    VisItGetDomainList
};
