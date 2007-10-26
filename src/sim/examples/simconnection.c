/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "sim.h"
#include "SimHelperFunc.h"
#include <VisItControlInterface_V1.h>
#include <VisItDataInterface_V1.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SimWidgetNames.h>

VisIt_SimulationMetaData *md = NULL; 
void Update_UI_Commands();


/*****************************************************************************
* Method: InitializeMD
*
* Purpose:
*   Initialize the metadata structure including allocating space for the
*   Command arrays.
*
* Arguments:
*   MaxNumCustCMD: The maximun number of ui components will will be controling
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
* 
*   Hank Childs, Fri Oct 26 08:45:58 PDT 2007
*   Add an expression.
*
*****************************************************************************/
void
InitializeMD(int MaxNumCustCMD)
{
    int sz;

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
    md->numExpressions = 2;
    sz = sizeof(VisIt_ExpressionMetaData) * md->numExpressions;
    md->expressions = (VisIt_ExpressionMetaData *) malloc(sz);
    memset(md->expressions, 0, sz);
    md->expressions[0].name = strdup("double_speed");
    md->expressions[0].definition = strdup("2*speed");
    md->expressions[0].vartype = VISIT_VARTYPE_SCALAR;
    md->expressions[1].name = strdup("half_density");
    md->expressions[1].definition = strdup("0.5*density");
    md->expressions[1].vartype = VISIT_VARTYPE_SCALAR;

    /* this will set up the generic and custom commands*/
    VisItInitAllCMD(md, MaxNumCustCMD);
}                    

/*****************************************************************************
* Method: VisItGetMetaData
*
* Purpose:
*   Called via VisIt to get the simulation MetaData.
*
* Arguments:
*   MaxNumCustCMD: The maximun number of ui components will will be controling
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Added Custom UI command control creation.
*   Enhanced function header compliance.
*   Added Gerneric button lable calls
*
*****************************************************************************/
VisIt_SimulationMetaData *
VisItGetMetaData()
{
    /*
     * maximum number of UI components connections
     * that you will be creating
     */
    int MAX_NUMBER_CUST_CMD = 15;

    /*
     * If the first time setup the meta data
     * and create meta data slots for the UI control
     * Do this only once.
     */
    if (md == NULL )
    {
        InitializeMD(MAX_NUMBER_CUST_CMD);

        /*
         * set up the specific ui channel to control the
         * ui compenents in the VisIt custom commands dialog
         * These names should be the exact same name as defined
         * in the interface file.
         */
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

        /*
         * if you want to rename the generic
         * button on the main sim window use
         * these commands. You are limited to 5
         * main buttons
         */
        VisItLabelGenericButton( md, 0, "halt",1);
        VisItLabelGenericButton( md, 1, "step",1);
        VisItLabelGenericButton( md, 2, "run",1);
        VisItLabelGenericButton( md, 3, "wdmp",1);
        VisItLabelGenericButton( md, 4, "wgdmp",1);
    }

    /*
     * now update the meta data so that the custom
     * command interface will be brought up to date
     * with what is going on in the simulation
     */
    Update_UI_Commands();

    /*
     * no return the updated meta data that will
     * be sent to the VisIt program
     */
    return md;
}

/*****************************************************************************
* Method: Update_UI_Commands
*
* Purpose:
*   This function updates the values displayed in the VisIt simulation Window.
*   Here is where you want to put the changing information.
*   This information will be sent back to VisIt and update
*   the matching UI components in the simulation window.
*
* Arguments:
*  
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Added Custom UI command control functions to demonstrate how
*   to set simulation data values in the custom UI window in VisIt
*   Enhanced function header compliance.
*   Added a call to demostrate displaying a message in the
*   simulation window.
*
*****************************************************************************/
#define MAX_CMD_STR_LEN 64

void
Update_UI_Commands()
{
      /* put updated UI information here. */
      static int timeStep = 10;
      static double dataValue = 0;
      char value[MAX_CMD_STR_LEN];
      char modValue[MAX_CMD_STR_LEN];
      int enable = 1;

      /* move the progess bar and update the value in the spin box */
      VisItSetCMDValue (*md, "progressBar1",  (timeStep *10)% 100);
      VisItSetCMDValue (*md, "ShellySpinBox1", timeStep);
      VisItSetCMDValue (*md, "LCDNumber1",timeStep);

      /* change the lable on the pushbutton   */
      VisItSetCMDText(*md, "Top_Button_1",  "Simulation Text");
      VisItSetCMDEnable(*md, "Top_Button_1", 0);
      VisItSetCMDIsOn(*md, "CheckBox1", 0);
      VisItSetCMDIsOn(*md, "RadioButton1", 1);
      VisItSetCMDIsOn(*md, "RadioButton2", 0);
      VisItSetCMDText(*md, "RadioButton1", "Label 1");
      VisItSetCMDText(*md, "RadioButton2", "Label 2");
      VisItSetCMDText(*md, "RadioButton3", "Label 3");

      /* set the text on the lable  */
      sprintf(value, "%5d", timeStep *10);
      strcat( value, "Simulation Label");
      VisItSetCMDText(*md, "MainTextLabel", value);
      VisItSetCMDText(*md, "CheckBox1", "New Label");
      VisItSetCMDText(*md, "timeEdit1", "11:06:03");
      VisItSetCMDText(*md, "dateEdit1", "Mon Jan 23 2006"); 

      /* move the slider and dial */
      VisItSetCMDValue(*md, "ShellySlider_1",(timeStep * 10) % 100);
      VisItSetCMDValue(*md,  "ShellyDial_1",timeStep % 360);
      sprintf (modValue, "%5d", timeStep % 360);
      VisItSetCMDText(*md, "ShellyLineEdit1",modValue);
      /* issues some example simulation messages */
      if ( !(timeStep % 11)) VisItSetStatusMessage( *md,"My Error Message","red");
      if ( !(timeStep % 5)) VisItSetStatusMessage( *md,"My Warning Message","yellow");
      if ( !(timeStep % 6)) VisItSetStatusMessage( *md,"My Sim Message","black");
      VisItSetCMDText(*md, "ShellyText_2",modValue);
    
      VisItSetStripChartTabName(*md, STRIP_CHART_1_TAB_NAME, "Density");
      VisItSetStripChartTabName(*md, STRIP_CHART_2_TAB_NAME, "Pressure"); 
      VisItSetStripChartTabName(*md, STRIP_CHART_3_TAB_NAME, "Temp");
      VisItSetStripChartTabName(*md, STRIP_CHART_4_TAB_NAME, "Volume");

      // send a data point to the strip chart
      if (timeStep > 0 )
      { 
          VisItAddStripChartDataPointByName( *md, STRIP_CHART_1_WIDGET_NAME, timeStep, dataValue+355.3, enable);
          VisItAddStripChartDataPointByName( *md, STRIP_CHART_2_WIDGET_NAME, timeStep, dataValue*10.0, enable);
          VisItAddStripChartDataPointByName( *md, STRIP_CHART_3_WIDGET_NAME, timeStep, dataValue+ (-0.3), enable);
          VisItAddStripChartDataPointByName( *md, STRIP_CHART_4_WIDGET_NAME, timeStep, dataValue+5.3, enable);
          VisItAddStripChartDataPointByName( *md, STRIP_CHART_5_WIDGET_NAME, timeStep, dataValue+0.3, enable);
      }
      
      
      md->currentCycle = cycle;
      md->currentTime  = timeStep;
      md->currentMode  = runflag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
      timeStep++;
      dataValue = dataValue + 0.008;
      if (dataValue > 0.2 ) dataValue = -0.01;
}

/*****************************************************************************
* Method: VisItGetMesh
*
* Purpose:
*   This function gets the simulation mesh data.
*
* Arguments:
*   domain: the domain requested.
*   name  : the name of the variable that the data is requested from.
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Enhanced function header compliance.
*
*****************************************************************************/
VisIt_MeshData *
VisItGetMesh(int domain,const char *name)
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

/*****************************************************************************
* Method: VisItGetMaterial
*
* Purpose:
*   This function gets the simulation meterial data.
*
* Arguments:
*   domain: the domain requested.  (not used)
*   name  : the name of the variable that the data is requested from.
*           (not used)
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Enhanced function header compliance.
*
*****************************************************************************/
VisIt_MaterialData *
VisItGetMaterial(int domain,const char *name)
{
    fprintf(stderr, "VisItGetMaterial\n");
    return NULL;
}

/*****************************************************************************
* Method: VisItGetScalar
*
* Purpose:
*   This function gets the simulation scalar data.
*
* Arguments:
*   domain: the domain requested.
*   name  : the name of the variable that the data is requested from.
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Enhanced function header compliance.
*
*****************************************************************************/
VisIt_ScalarData *
VisItGetScalar(int domain,const char *name)
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

/*****************************************************************************
* Method: VisItGetDomainList
*
* Purpose:
*   This function create a list of domains and return them to the
*   calling function.
*
* Arguments:
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Enhanced function header compliance.
*
*****************************************************************************/
VisIt_DomainList *
VisItGetDomainList()
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

/*****************************************************************************
* Struct: visitCallbacks
*
* Purpose:
*   This structure holds all the function pointers that allow visit to get
*   access to the simulation data.
*
* Arguments:
*
* Programmer: Jeremy Meredith
* Creation:   April  4, 2005
*
* Modifications:
*   Shelly Prevost, Tue Sep 12 12:02:21 PDT 2006
*   Enhanced function header compliance.
*
*****************************************************************************/
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
