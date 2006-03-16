/* Programmer: Jeremy Meredith
 * Date      : April  4, 2005
 *
 * Modifications:
 *    Jeremy Meredith, Thu Apr 28 18:17:02 PDT 2005
 *    Changed it to a rectilinear mesh.
 *
 *    Jeremy Meredith, Wed May 11 11:05:50 PDT 2005
 *    Added ghost zones.  Added domain lists for restricted load balancing.
 *
 *    Shelly Prevost added custom command updating
 */
#include "sim.h"
#include "SimHelperFunc.h"
#include <VisItControlInterface_V1.h>
#include <VisItDataInterface_V1.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VisIt_SimulationMetaData *md = NULL; 
void Update_UI_Commands();

// Here is where you add all the initialization code you
// want to run only once.
void InitializeMD(int MaxNumCustCMD )
{
    md = malloc(sizeof(VisIt_SimulationMetaData));

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

    // this will set up the generic and custom
    // commands
    VisItInitAllCMD(md, MaxNumCustCMD);
}                    


VisIt_SimulationMetaData *VisItGetMetaData()
{
    // maximum number of UI components connections
    // that you will be creating
    int MAX_NUMBER_CUST_CMD = 15;

    // if the first time setup the meta data
    // and create meta data slots for the UI control
    // Do this only once.
    if (md == NULL )
    {
      InitializeMD(MAX_NUMBER_CUST_CMD);

      // set up the specific ui channel to control the
      // ui compenents in the VisIt custom commands dialog
      // These names should be the exact same name as defined
      // in the interface file.
      VisItCreateCMD( *md, "MainTextLabel");
      VisItCreateCMD( *md, "ShellySpinBox1");
      VisItCreateCMD( *md, "progressBar1");
      VisItCreateCMD( *md, "Top_Button_1");
      VisItCreateCMD( *md, "ShellySlider_1");
      VisItCreateCMD( *md, "ShellyDial_1");
      VisItCreateCMD( *md, "ShellyLineEdit1");
      VisItCreateCMD( *md, "ShellyText_2");
      VisItCreateCMD( *md, "RadioButton1");
      VisItCreateCMD( *md, "RadioButton2");
      VisItCreateCMD( *md, "RadioButton3");
      VisItCreateCMD( *md, "LCDNumber1");
      VisItCreateCMD( *md, "CheckBox1");
      VisItCreateCMD( *md, "timeEdit1");
      VisItCreateCMD( *md, "dateEdit1");
    }

    // now update the meta data so that the custom
    // command interface will be brought up to date
    // with what is going on in the simulation
    Update_UI_Commands();

    // no return the updated meta data that will
    // be sent to the VisIt program
    return md;
}

// Here is where you want to put the changing information.
// This information will be sent back to VisIt and update
// the matching UI components.
void Update_UI_Commands()
{
      int MAX_CMD_STR_LEN = 64;
      // put updated UI information here.
      static int timeStep = 0;
      char value[MAX_CMD_STR_LEN];
      char modValue[MAX_CMD_STR_LEN];

     // move the progess bar and update the value in the spin box
      VisItSetCMDValue (*md, "progressBar1",  (timeStep *10)% 100);
      VisItSetCMDValue (*md, "ShellySpinBox1", timeStep);
      VisItSetCMDValue (*md, "LCDNumber1",timeStep);

      // change the lable on the pushbutton
      VisItSetCMDText(*md, "Top_Button_1",  "Simulation Text");
      VisItSetCMDEnable(*md, "Top_Button_1", 0);
      VisItSetCMDIsOn(*md, "CheckBox1", 0);
      VisItSetCMDIsOn(*md, "RadioButton1", 1);
      VisItSetCMDIsOn(*md, "RadioButton2", 0);

      VisItSetCMDText(*md, "RadioButton1", "Label 1");
      VisItSetCMDText(*md, "RadioButton2", "Label 2");
      VisItSetCMDText(*md, "RadioButton3", "Label 3");


      // set the text on the lable
      sprintf(value, "%5d", timeStep *10);
      strcat( value, "Simulation Label");
      VisItSetCMDText(*md, "MainTextLabel", value);
      VisItSetCMDText(*md, "CheckBox1", "New Label");
      VisItSetCMDText(*md, "timeEdit1", "11:06:03");
      VisItSetCMDText(*md, "dateEdit1", "Mon Jan 23 2006");

      // move the slider and dial
      VisItSetCMDValue(*md, "ShellySlider_1",(timeStep * 10) % 100);
      VisItSetCMDValue(*md,  "ShellyDial_1",timeStep % 360);

      sprintf (modValue, "%5d", timeStep % 360);
      VisItSetCMDText(*md, "ShellyLineEdit1",modValue);
      VisItSetCMDText(*md, "ShellyText_2",modValue);
 
      md->currentCycle = cycle;
      md->currentTime  = timeStep;
      md->currentMode  = runflag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
      timeStep++;
}



// This function is called when Visit wants to retrieve
// the mesh data from the simulation
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






