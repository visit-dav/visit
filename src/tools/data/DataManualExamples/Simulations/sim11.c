// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
#include <stdio.h>

#include "SimulationExample.h"

#include <stubs.c>

visit_handle
SimGetMetaData(void *cbdata)
{
    visit_handle md = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    /* Create metadata with no variables. */
    if(VisIt_SimulationMetaData_alloc(&md) == VISIT_OKAY)
    {
        visit_handle m1 = VISIT_INVALID_HANDLE;
        visit_handle m2 = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;
        visit_handle cmd = VISIT_INVALID_HANDLE;
        visit_handle mat = VISIT_INVALID_HANDLE;
        visit_handle emd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        if(sim->runMode == VISIT_SIMMODE_STOPPED)
            VisIt_SimulationMetaData_setMode(md, VISIT_SIMMODE_STOPPED);
        else
            VisIt_SimulationMetaData_setMode(md, VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Set the first mesh's properties.*/
        if(VisIt_MeshMetaData_alloc(&m1) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(m1, "mesh2d");
            VisIt_MeshMetaData_setMeshType(m1, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(m1, 2);
            VisIt_MeshMetaData_setSpatialDimension(m1, 2);
            VisIt_MeshMetaData_setXUnits(m1, "cm");
            VisIt_MeshMetaData_setYUnits(m1, "cm");
            VisIt_MeshMetaData_setXLabel(m1, "Width");
            VisIt_MeshMetaData_setYLabel(m1, "Height");

            VisIt_SimulationMetaData_addMesh(md, m1);
        }

        /* Set the second mesh's properties.*/
        if(VisIt_MeshMetaData_alloc(&m2) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(m2, "mesh3d");
            VisIt_MeshMetaData_setMeshType(m2, VISIT_MESHTYPE_CURVILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(m2, 3);
            VisIt_MeshMetaData_setSpatialDimension(m2, 3);
            VisIt_MeshMetaData_setXUnits(m2, "cm");
            VisIt_MeshMetaData_setYUnits(m2, "cm");
            VisIt_MeshMetaData_setZUnits(m2, "cm");
            VisIt_MeshMetaData_setXLabel(m2, "Width");
            VisIt_MeshMetaData_setYLabel(m2, "Height");
            VisIt_MeshMetaData_setZLabel(m2, "Depth");

            VisIt_SimulationMetaData_addMesh(md, m2);
        }

        /* Add a zonal scalar variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a nodal scalar variable on mesh3d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh3d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a curve variable. */
        if(VisIt_CurveMetaData_alloc(&cmd) == VISIT_OKAY)
        {
            VisIt_CurveMetaData_setName(cmd, "sine");
            VisIt_CurveMetaData_setXLabel(cmd, "Angle");
            VisIt_CurveMetaData_setXUnits(cmd, "radians");
            VisIt_CurveMetaData_setYLabel(cmd, "Amplitude");
            VisIt_CurveMetaData_setYUnits(cmd, "");

            VisIt_SimulationMetaData_addCurve(md, cmd);
        }

        /* Add a material */
        if(VisIt_MaterialMetaData_alloc(&mat) == VISIT_OKAY)
        {
            VisIt_MaterialMetaData_setName(mat, "mat");
            VisIt_MaterialMetaData_setMeshName(mat, "mesh2d");
            VisIt_MaterialMetaData_addMaterialName(mat, "Iron");
            VisIt_MaterialMetaData_addMaterialName(mat, "Copper");
            VisIt_MaterialMetaData_addMaterialName(mat, "Nickel");

            VisIt_SimulationMetaData_addMaterial(md, mat);
        }

        /* Add some expressions. */
        if(VisIt_ExpressionMetaData_alloc(&emd) == VISIT_OKAY)
        {
            VisIt_ExpressionMetaData_setName(emd, "zvec");
            VisIt_ExpressionMetaData_setDefinition(emd, "{zonal, zonal}");
            VisIt_ExpressionMetaData_setType(emd, VISIT_VARTYPE_VECTOR);

            VisIt_SimulationMetaData_addExpression(md, emd);
        }
        if(VisIt_ExpressionMetaData_alloc(&emd) == VISIT_OKAY)
        {
            VisIt_ExpressionMetaData_setName(emd, "nid");
            VisIt_ExpressionMetaData_setDefinition(emd, "nodeid(mesh3d)");
            VisIt_ExpressionMetaData_setType(emd, VISIT_VARTYPE_SCALAR);

            VisIt_SimulationMetaData_addExpression(md, emd);
        }
    }

    return md;
}

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:35:53 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(simulation_data *sim)
{
    int blocking, visitstate, err = 0;

    do
    {
        blocking = (sim->runMode == VISIT_SIMMODE_RUNNING) ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        visitstate = VisItDetectInput(blocking, -1);

        /* Do different things depending on the output from VisItDetectInput. */
        if(visitstate >= -5 && visitstate <= -1)
        {
            fprintf(stderr, "Can't recover from error!\n");
            err = 1;
        }
        else if(visitstate == 0)
        {
            /* There was no input from VisIt, return control to sim. */
            simulate_one_timestep(sim);
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection())
            {
                fprintf(stderr, "VisIt connected\n");

                /* Register data access callbacks */
                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            sim->runMode = VISIT_SIMMODE_STOPPED;
            if(!VisItProcessEngineCommand())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                sim->runMode = VISIT_SIMMODE_RUNNING;
            }
        }
    } while(!sim->done && err == 0);
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:36:17 PST 2007
 *
 * Input Arguments:
 *   argc : The number of command line arguments.
 *   argv : The command line arguments.
 *
 * Modifications:
 *
 *****************************************************************************/

int main(int argc, char **argv)
{
    simulation_data sim;
    simulation_data_ctor(&sim);
    SimulationArguments(argc, argv);
   
    /* Initialize envuronment variables. */
    VisItSetupEnvironment();
    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("sim11",
        "Demonstrates creating expression metadata",
        "/path/to/where/sim/was/started",
        NULL, NULL, NULL);

    /* Read input problem setup, geometry, data. */
    read_input_deck(&sim);

    /* Call the main loop. */
    mainloop(&sim);

    simulation_data_dtor(&sim);
    return 0;
}
