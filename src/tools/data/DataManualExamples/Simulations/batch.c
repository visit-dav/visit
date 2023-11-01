// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include "extract.h"

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Communication Function prototypes */
int SimBroadcastInt(int *value, int sender, void *cbdata);
int SimBroadcastString(char *str, int len, int sender, void *cbdata);
void SimWorkerProcessCallback(void *);

/* Data Adaptor Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

typedef struct
{
#ifdef PARALLEL
    MPI_Comm par_comm;
#endif
    int      par_rank;
    int      par_size;
    int      maxcycles;
    int      cycle;
    double   time;
    char     format[30];
    int      domains[3];
    int      dims[3];
    float    extents[6];
    int      groupSize;
    int      export;
    int      render;
    int      image_width;
    int      image_height;
    int      setview;
    int      cinema;
    int      cinema_spec;
    int      cinema_camera;
    float   *x;
    float   *y;
    float   *z;
    float   *q;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->par_rank = 0;
    sim->par_size = 1;
    sim->maxcycles = 1000000;
    sim->cycle = 0;
    sim->time = 0.;
    strcpy(sim->format, "VTK_1.0");
    sim->domains[0] = 1;
    sim->domains[1] = 1;
    sim->domains[2] = 1;
    sim->dims[0] = 50;
    sim->dims[1] = 50;
    sim->dims[2] = 50;
    sim->extents[0] = 0.f;
    sim->extents[1] = 10.f;
    sim->extents[2] = 0.f;
    sim->extents[3] = 10.f;
    sim->extents[4] = 0.f;
    sim->extents[5] = 10.f;
    sim->groupSize = -1;
    sim->export = 0;
    sim->render = 0;
    sim->image_width = 1920/2;
    sim->image_height = 1080/2;
    sim->setview = 0;
    sim->cinema = 0;
    sim->cinema_spec = VISIT_CINEMA_SPEC_A;
    sim->cinema_camera = VISIT_CINEMA_CAMERA_PHI_THETA;
    sim->x = NULL;
    sim->y = NULL;
    sim->z = NULL;
    sim->q = NULL;
}

void
simulation_data_dtor(simulation_data *sim)
{
    if(sim->x != NULL)
    {
        free(sim->x);
        sim->x = NULL;
    }
    if(sim->y != NULL)
    {
        free(sim->y);
        sim->y = NULL;
    }
    if(sim->z != NULL)
    {
        free(sim->z);
        sim->z = NULL;
    }
    if(sim->q != NULL)
    {
        free(sim->q);
        sim->q = NULL;
    }
}

void
simulation_data_domainIJK(simulation_data *sim, int *idom, int *jdom, int *kdom)
{
    int NXY = sim->domains[0]*sim->domains[1];
    *kdom = sim->par_rank / (NXY);
    *jdom = (sim->par_rank % NXY) / sim->domains[0];
    *idom = (sim->par_rank % NXY) % sim->domains[0];
}

void 
simulation_data_update(simulation_data *sim)
{
    float x,y,z,offset[3];
    int index, i,j,k, npts, ncells, idom, jdom, kdom;
    npts = sim->dims[0]*sim->dims[1]*sim->dims[2];
    ncells = (sim->dims[0]-1)*(sim->dims[1]-1)*(sim->dims[2]-1);

    if(sim->par_rank == 0)
    {
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
        fflush(stdout);
    }

    /* Allocate the arrays */
    if(sim->x == NULL)
    {
        sim->x = (float *)malloc(sizeof(float) * npts);
        sim->y = (float *)malloc(sizeof(float) * npts);
        sim->z = (float *)malloc(sizeof(float) * npts);
        sim->q = (float *)malloc(sizeof(float) * ncells);

        simulation_data_domainIJK(sim, &idom, &jdom, &kdom);

        /* Init coordinates. */
        offset[0] = (sim->extents[1] - sim->extents[0]) * idom;
        offset[1] = (sim->extents[3] - sim->extents[2]) * jdom;
        offset[2] = (sim->extents[5] - sim->extents[4]) * kdom;
        index = 0;
        for(k = 0; k < sim->dims[2]; ++k)
        {
            float tz = ((float)k) / ((float)(sim->dims[2] - 1));
            z = (1.f-tz)*sim->extents[4] + tz*sim->extents[5];
            for(j = 0; j < sim->dims[1]; ++j)
            {
                float ty = ((float)j) / ((float)(sim->dims[1] - 1));
                y = (1.f-ty)*sim->extents[2] + ty*sim->extents[3];
                for(i = 0; i < sim->dims[0]; ++i)
                {
                    float tx = ((float)i) / ((float)(sim->dims[0] - 1));
                    x = (1.f-tx)*sim->extents[0] + tx*sim->extents[1];
                    sim->x[index] = x + offset[0];
                    sim->y[index] = y + offset[1];
                    sim->z[index] = z + offset[2];
                    ++index;
                }
            }
        }
    }
    /* Update q. */
    for(k = 0; k < sim->dims[2]-1; ++k)
    {
        for(j = 0; j < sim->dims[1]-1; ++j)
        {
            for(i = 0; i < sim->dims[0]-1; ++i)
            {
                int srcIndex, destIndex;
                srcIndex = k*(sim->dims[0])*(sim->dims[1]) +
                           j*(sim->dims[0]) +
                           i;
                destIndex = k*(sim->dims[0]-1)*(sim->dims[1]-1) +
                            j*(sim->dims[0]-1) +
                            i;

                sim->q[destIndex] = sin(sim->x[srcIndex] + sim->time);
            }
        }
    }
}

void
simulation_data_global_extents(simulation_data *sim, double ext[6])
{
    ext[0] = 0.;
    ext[1] = (sim->extents[1] - sim->extents[0]) * sim->domains[0];
    ext[2] = 0.;
    ext[3] = (sim->extents[3] - sim->extents[2]) * sim->domains[1];
    ext[4] = 0.;
    ext[5] = (sim->extents[5] - sim->extents[4]) * sim->domains[2];
}

/******************************************************************************
 *
 * Function: mainloop_batch
 *
 * Purpose: The batch version of the main loop. It iterates and makes extracts.
 *
 * Programmer: Brad Whitlock
 * Date:      Tue Feb  3 13:10:23 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop_batch(simulation_data *sim)
{
    int err;
    visit_handle hcdb = VISIT_INVALID_HANDLE, hvar = VISIT_INVALID_HANDLE;
    char filebase[100];
    const char *cdb = "batch.cdb";
    const char *extractvars[] = {"q", "xc", "radius", "dom", NULL};
    double origin[] = {5., 5., 5.}, normal[] = {0., 0.707, 0.707};
    double isos[] = {5., 11., 18.};
    double v0[] = {1.,1.,1.}, v1[] = {5., 1.5, 7.}, v2[] = {8., 2., 5.};
    double seeds[] = {
        0.5, 0.5, 0.5,
        1.5, 0.5, 0.5,
        2.5, 0.5, 0.5,
        3.5, 0.5, 0.5,
        4.5, 0.5, 0.5,
        5.5, 0.5, 0.5,
        0.5, 4.5, 0.5,
        1.5, 4.5, 0.5,
        2.5, 4.5, 0.5,
        3.5, 4.5, 0.5,
        4.5, 4.5, 0.5,
        5.5, 4.5, 0.5};
    int lseeds = sizeof(seeds) / sizeof(double);
#ifdef PARALLEL
    double init0, init1;
    init0 = MPI_Wtime();
#endif
    /* Explicitly load VisIt runtime functions and install callbacks. */
    VisItInitializeRuntime();
    VisItSetWorkerProcessCallback2(SimWorkerProcessCallback, (void*)sim);
    VisItSetGetMetaData(SimGetMetaData, (void*)sim);
    VisItSetGetMesh(SimGetMesh, (void*)sim);
    VisItSetGetVariable(SimGetVariable, (void*)sim);
    VisItSetGetDomainList(SimGetDomainList, (void*)sim);
#ifdef PARALLEL
    init1 = MPI_Wtime();
    if(sim->par_rank == 0)
    {
        printf("Initialization time: %lg\n", init1 - init0);
    }
#endif

    /* Begin a Cinema database. */
    if(sim->cinema)
    {
#if 1
        /* Make a list of vars to export to Cinema. (optional) */
        VisIt_NameList_alloc(&hvar);
        VisIt_NameList_addName(hvar, "d");
        VisIt_NameList_addName(hvar, "q");
        VisIt_NameList_addName(hvar, "radius");
#endif
        VisItBeginCinema(&hcdb, cdb, sim->cinema_spec, 0, 
                         VISIT_IMAGEFORMAT_PNG, sim->image_width, sim->image_height,
                         sim->cinema_camera, 12, 7,
                         hvar);
    }

    while(sim->cycle < sim->maxcycles)
    {  
        /* Update the simulation data for this iteration.*/     
        simulation_data_update(sim);

        /* Tell VisIt that some metadata changed.*/
        VisItTimeStepChanged();

        if(sim->export)
        {
            /* Set some extract options. */
            extract_set_options(sim->format, (sim->groupSize > 0)?1:0, sim->groupSize);

            /* Make some extracts. */
            sprintf(filebase, "slice3v_%04d", sim->cycle);
            err = extract_slice_3v(filebase, v0, v1, v2, extractvars);
            if(sim->par_rank == 0)
            {
                printf("slice3v export returned %s\n", extract_err(err));
            }

            sprintf(filebase, "sliceON_%04d", sim->cycle);
            err = extract_slice_origin_normal(filebase, origin, normal, extractvars);
            if(sim->par_rank == 0)
            {
                printf("sliceON export returned %s\n", extract_err(err));
            }

            sprintf(filebase, "sliceX_%04d", sim->cycle);
            err = extract_slice(filebase, 0, 0.5, extractvars);
            if(sim->par_rank == 0)
            {
                printf("sliceX export returned %s\n", extract_err(err));
            }

            sprintf(filebase, "sliceY_%04d", sim->cycle);
            err = extract_slice(filebase, 1, 2.5, extractvars);
            if(sim->par_rank == 0)
            {
                printf("slice export returned %s\n", extract_err(err));
            }

            sprintf(filebase, "sliceZ_%04d", sim->cycle);
            err = extract_slice(filebase, 2, 5., extractvars);
            if(sim->par_rank == 0)
            {
                printf("sliceZ export returned %s\n", extract_err(err));
            }

            sprintf(filebase, "iso_%04d", sim->cycle);
            err = extract_iso(filebase, "radius", isos, 3, extractvars);
            if(sim->par_rank == 0)
            {
                printf("iso export returned %s\n", extract_err(err));
            }

            /* NOTE: This exercises expressions and operator-created vars. */
            sprintf(filebase, "streamline_%04d", sim->cycle);
            err = extract_streamline(filebase, "vec", seeds, lseeds, extractvars);
            if(sim->par_rank == 0)
            {
                printf("streamline export returned %s\n", extract_err(err));
            }
        }

        if(sim->render || sim->cinema)
        {
            /* Set up some plots. */
            VisItAddPlot("Contour", "d");
            VisItDrawPlots();

            if(sim->render)
            {
                char filename[100];
                if(sim->setview)
                {
                    visit_handle view;
                    double normal[3]={0.5422, 0.3510, 0.7633},
                            viewUp[3]={-0.2069, 0.9363, -0.2835};

                    /* Allocate a view and get the plot view.*/
                    VisIt_View3D_alloc(&view);
                    VisItGetView3D(view);
 
                    /* Override the normal and up vectors. */
                    VisIt_View3D_setViewNormal(view, normal);
                    VisIt_View3D_setViewUp(view, viewUp);               
                    VisItSetView3D(view);

                    /* Free the view. */
                    VisIt_View3D_free(view);
                }

                sprintf(filename, "batch%04d.png", sim->cycle);
                if(VisItSaveWindow(filename, sim->image_width, sim->image_height, VISIT_IMAGEFORMAT_PNG) == VISIT_OKAY)
                {
                    if(sim->par_rank == 0)
                        printf("Saved %s\n", filename);
                }
                else if(sim->par_rank == 0)
                    printf("The image could not be saved to %s\n", filename);
            }

            if(sim->cinema)
            {
                /* Save the current plots to the Cinema database. */
                VisItSaveCinema(hcdb, sim->time);
            }

            VisItDeleteActivePlots();
        }

        ++sim->cycle;
        sim->time += (M_PI / 10.);
    }

    /* End a Cinema database. */
    if(sim->cinema)
    {
        VisItEndCinema(hcdb);
    }
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:53:45 PST 2015
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
    int i;
    char tracefile[1000], options[1000], *opt, *env = NULL;
    simulation_data sim;
    simulation_data_ctor(&sim);

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);

    /* Create a new communicator. */
    if (MPI_Comm_dup(MPI_COMM_WORLD, &sim.par_comm) != MPI_SUCCESS)
        sim.par_comm = MPI_COMM_WORLD;

    MPI_Comm_rank (sim.par_comm, &sim.par_rank);
    MPI_Comm_size (sim.par_comm, &sim.par_size);
#endif

#if 1
    /* Let's restrict the plugins that we load in batch. */
    strcpy(options, "-plotplugins Contour,Mesh,Pseudocolor -operatorplugins Slice,IntegralCurve,Isosurface,Threshold -noconfig");
    opt = options + strlen(options);
#else
    opt = options;
    opt[0] = '\0';
#endif

    /* Check for command line arguments. */
    for(i = 1; i < argc; ++i)
    {
        /* Arguments with no values */
        if(strcmp(argv[i], "-setview") == 0)
        {
            sim.setview = 1;
        }
        else if(strcmp(argv[i], "-cinema") == 0)
        {
            sim.cinema = 1;
        }
        /* These arguments have a value. */
        else if((i+1) < argc)
        {
            if(strcmp(argv[i], "-dims") == 0)
            {
                sscanf(argv[i+1], "%d,%d,%d", &sim.dims[0], &sim.dims[1], &sim.dims[2]);
                i++;
            }
            else if(strcmp(argv[i], "-domains") == 0)
            {
                sscanf(argv[i+1], "%d,%d,%d", &sim.domains[0], &sim.domains[1], &sim.domains[2]);
                i++;
            }
            else if(strcmp(argv[i], "-maxcycles") == 0)
            {
                sscanf(argv[i+1], "%d", &sim.maxcycles);
                i++;
            }
            else if(strcmp(argv[i], "-dir") == 0)
            {
                /* Specify the path to VisIt installation. */
                VisItSetDirectory(argv[i+1]);
                i++;
            }
            else if(strcmp(argv[i], "-groupsize") == 0)
            {
                sim.groupSize = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(argv[i], "-trace") == 0)
            {
                sprintf(tracefile, "%s.%d.log", argv[i+1], sim.par_rank);
                VisItOpenTraceFile(tracefile);
                i++;
            }
            else if(strcmp(argv[i], "-format") == 0)
            {
                strncpy(sim.format, argv[i+1], 30);
                i++;
            }
            else if(strcmp(argv[i], "-export") == 0)
            {
                sim.export = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(argv[i], "-render") == 0)
            {
                sim.render = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(argv[i], "-image-width") == 0)
            {
                sim.image_width = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(argv[i], "-image-height") == 0)
            {
                sim.image_height = atoi(argv[i+1]);
                i++;
            }
            else if(strcmp(argv[i], "-cinema-spec") == 0)
            {
                if(strcmp(argv[i+1], "A") == 0)
                    sim.cinema_spec = VISIT_CINEMA_SPEC_A;
                if(strcmp(argv[i+1], "C") == 0)
                    sim.cinema_spec = VISIT_CINEMA_SPEC_C;
                if(strcmp(argv[i+1], "D") == 0)
                    sim.cinema_spec = VISIT_CINEMA_SPEC_D;
                i++;
            }
            else if(strcmp(argv[i], "-cinema-camera") == 0)
            {
                if(strcmp(argv[i+1], "static") == 0)
                    sim.cinema_camera = VISIT_CINEMA_CAMERA_STATIC;
                if(strcmp(argv[i+1], "phi-theta") == 0)
                    sim.cinema_camera = VISIT_CINEMA_CAMERA_PHI_THETA;
                i++;
            }
            else
            {
                if(strlen(options) > 0)
                    opt = strcat(opt, " ");
                opt = strcat(opt, argv[i]);
            }
        }
        else
        {
            if(strlen(options) > 0)
                opt = strcat(opt, " ");
            opt = strcat(opt, argv[i]);
        }
    }

    if(sim.domains[0]*sim.domains[1]*sim.domains[2] != sim.par_size)
    {
        if(sim.par_rank == 0)
            fprintf(stderr, "The number of domains must match the number of ranks.\n");
#ifdef PARALLEL
        MPI_Finalize();
#endif
        return -1;
    }

    if(strlen(options) > 0)
    {
        printf("options: %s\n", options);
        VisItSetOptions(options);
    }

#ifdef PARALLEL
    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction2(SimBroadcastInt, (void*)&sim);
    VisItSetBroadcastStringFunction2(SimBroadcastString, (void*)&sim);

    /* Tell libsim whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);

    /* Tell libsim which communicator to use. You must pass the address of
     * an MPI_Comm object.
     */
    VisItSetMPICommunicator((void *)&sim.par_comm);
#endif

    /* Only read the environment on rank 0. This could happen before MPI_Init if
     * we are using an MPI that does not like to let us spawn processes but we
     * would not know our processor rank.
     */
    if(sim.par_rank == 0)
        env = VisItGetEnvironment();

    /* Pass the environment to all other processors collectively. */
    VisItSetupEnvironment2(env);
    if(env != NULL)
        free(env);

    /* Call the main loop. */
    mainloop_batch(&sim);

    simulation_data_dtor(&sim);
#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/*******************************************************************************
********************************************************************************
*****                           ADAPTOR FUNCTIONS
********************************************************************************
*******************************************************************************/

/* COMMUNICATION ADAPTOR FUNCTIONS. */

#ifdef PARALLEL
int SimBroadcastInt(int *value, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(value, 1, MPI_INT, sender, sim->par_comm);
}

int SimBroadcastString(char *str, int len, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(str, len, MPI_CHAR, sender, sim->par_comm);
}
#endif

void SimWorkerProcessCallback(void *cbdata)
{
#ifdef PARALLEL
    simulation_data *sim = (simulation_data *)cbdata;
    int command = VISIT_COMMAND_PROCESS;
    MPI_Bcast(&command, 1, MPI_INT, 0, sim->par_comm);
#endif
}

/* DATA ADAPTOR FUNCTIONS */

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMetaData(void *cbdata)
{
    visit_handle md = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    /* Create metadata. */
    if(VisIt_SimulationMetaData_alloc(&md) == VISIT_OKAY)
    {
        visit_handle mmd = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;
        visit_handle emd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Add mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "mesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CURVILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, sim->domains[0]*sim->domains[1]*sim->domains[2]);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");
            VisIt_MeshMetaData_setNumGroups(mmd, 0);
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setZUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");
            VisIt_MeshMetaData_setZLabel(mmd, "Depth");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "q");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "xc");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "radius");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "dom");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "d");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add expressions for VisIt to calculate. */
        if(VisIt_ExpressionMetaData_alloc(&emd) == VISIT_OKAY)
        {
            VisIt_ExpressionMetaData_setName(emd, "vec");
            VisIt_ExpressionMetaData_setDefinition(emd, "gradient(radius)");
            VisIt_ExpressionMetaData_setType(emd, VISIT_VARTYPE_VECTOR);
            VisIt_SimulationMetaData_addExpression(md, emd);
        }
    }

    return md;
}

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "mesh") == 0)
    {
        if(VisIt_CurvilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int npts;
            visit_handle hx, hy, hz;
            npts = sim->dims[0]*sim->dims[1]*sim->dims[2];
            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_alloc(&hz);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, npts, sim->x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, npts, sim->y);
            VisIt_VariableData_setDataF(hz, VISIT_OWNER_SIM, 1, npts, sim->z);
            VisIt_CurvilinearMesh_setCoordsXYZ(h, sim->dims, hx, hy, hz);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    int ncells, npts;
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    npts = sim->dims[0] * sim->dims[1] * sim->dims[2];
    ncells = (sim->dims[0]-1) * (sim->dims[1]-1) * (sim->dims[2]-1);

    if(strcmp(name, "q") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            ncells, sim->q);
    }
    else if(strcmp(name, "xc") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            npts, sim->x);
    }
    else if(strcmp(name, "radius") == 0)
    {
        int index;
        double *rad = NULL;
        VisIt_VariableData_alloc(&h);
        /* On the fly data generation. We donate the array to VisIt. */
        rad = (double *)malloc(npts * sizeof(double));
        for(index = 0; index < npts; ++index)
            rad[index] = sqrt(sim->x[index]*sim->x[index] + 
                              sim->y[index]*sim->y[index] + 
                              sim->z[index]*sim->z[index]);
        VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
            npts, rad);
    }
    else if(strcmp(name, "d") == 0)
    {
        int index;
        double *rad = NULL;
        VisIt_VariableData_alloc(&h);
        /* On the fly data generation. We donate the array to VisIt. */
        rad = (double *)malloc(npts * sizeof(double));
        for(index = 0; index < npts; ++index)
            rad[index] = sin(sim->time + 
                             0.25 * sqrt(sim->x[index]*sim->x[index] + 
                              sim->y[index]*sim->y[index] + 
                              sim->z[index]*sim->z[index]));
        VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
            npts, rad);
    }
    else if(strcmp(name, "dom") == 0)
    {
        int index;
        float *dom = NULL;
        VisIt_VariableData_alloc(&h);
        /* On the fly data generation. We donate the array to VisIt. */
        dom = (float *)malloc(npts * sizeof(int));
        for(index = 0; index < npts; ++index)
            dom[index] = domain;
        VisIt_VariableData_setDataF(h, VISIT_OWNER_VISIT, 1,
            npts, dom);
    }
    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list (the list of domains
 *          owned by this processor).
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetDomainList(const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
    {
        visit_handle hdl;
        int *iptr = NULL;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int));
        *iptr = sim->par_rank;

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, iptr);
        VisIt_DomainList_setDomains(h, sim->par_size, hdl);
    }
    return h;
}
