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

extern "C" {
#include "extract.h"
}

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

// ****************************************************************************
// Class: Triplex
//
// Purpose:
//   Hypercomplex number.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:29:33 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class Triplex
{
public:
    Triplex() { x = y = z = 0.f; }
    Triplex(float v) { x = y = z = v; }
    Triplex(float a, float b, float c) { x = a; y = b; z = c; }

    float mag2() const
    {
        return x*x + y*y + z*z;
    }

    float mag() const
    {
        return sqrt(mag2());
    }

    float x,y,z;
};

inline Triplex operator + (const Triplex &obj, float val)
{
    return Triplex(obj.x+val, obj.y+val, obj.z+val);
}

inline Triplex operator + (float val, const Triplex &obj)
{
    return Triplex(obj.x+val, obj.y+val, obj.z+val);
}

inline Triplex operator + (const Triplex &lhs, const Triplex &rhs)
{
    return Triplex(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
}

inline Triplex operator * (const Triplex &obj, float val)
{
    return Triplex(obj.x*val, obj.y*val, obj.z*val);
}

inline Triplex operator * (float val, const Triplex &obj)
{
    return Triplex(obj.x*val, obj.y*val, obj.z*val);
}

inline Triplex operator ^ (const Triplex &obj, float n)
{
    // http://www.bugman123.com/Hypercomplex/index.html
    float r = obj.mag();
    float theta = n * atan2(obj.y,obj.x);
    float phi = (r <= 0.) ? (n * asin(1.)) : (n * asin(obj.z/r));
    float rn = pow(r,n);
    float newx = rn * cos(theta) * cos(phi);
    float newy = rn * sin(theta) * cos(phi);
    float newz = rn * -sin(phi);
    return Triplex(newx, newy, newz);
}

// ****************************************************************************
// Method: mandelbulb
//
// Purpose:
//   Calculate power 8 mandelbulb.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       Order 8 Mandelbulb
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:17:20 PDT 2014
//
// Modifications:
//
// ****************************************************************************
 
inline char
mandelbulb(const Triplex &C)
{
    const char MAXIT = 20;
    Triplex Z;
    for(char zit = 0; zit < MAXIT; ++zit)
    {
        Z = (Z ^ 8.f) + C;
        if(Z.mag2() > 4.f)
            return zit+1;
    }
    return 0;
}

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
    float    extents0[6];
    float    extents1[6];
    int      render;
    int      exports;
    int      image_width;
    int      image_height;
    float    *x;
    float    *y;
    float    *z;
    float    *data;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->par_rank = 0;
    sim->par_size = 1;
    sim->maxcycles = 1000;
    sim->cycle = 0;
    sim->time = 0.;
    strcpy(sim->format, "FieldViewXDB_1.0");
    sim->domains[0] = 1;
    sim->domains[1] = 1;
    sim->domains[2] = 1;
    sim->dims[0] = 50;
    sim->dims[1] = 50;
    sim->dims[2] = 50;
    sim->extents0[0] = 0.6f;
    sim->extents0[1] = 1.2f;
    sim->extents0[2] = 0.f;
    sim->extents0[3] = M_PI;
    sim->extents0[4] = 0.f;
    sim->extents0[5] = 2.*M_PI;
    sim->extents1[0] = 0.6f;
    sim->extents1[1] = 1.2f;
    sim->extents1[2] = M_PI/2 - M_PI/12.;
    sim->extents1[3] = M_PI/2 + M_PI/12.;
    sim->extents1[4] = M_PI - M_PI / 12.;
    sim->extents1[5] = M_PI + M_PI / 12.;
    sim->render = 1;
    sim->exports = 0;
    sim->image_width = 2000;
    sim->image_height = 2000;
    sim->x = NULL;
    sim->y = NULL;
    sim->z = NULL;
    sim->data = NULL;
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
    if(sim->data != NULL)
    {
        free(sim->data);
        sim->data = NULL;
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
    float x,y,z,offset[3], ext[6];
    int i,j,k, npts, idom, jdom, kdom;
    npts = sim->dims[0]*sim->dims[1]*sim->dims[2];

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
        sim->data = (float *)malloc(sizeof(float) * npts);
    }

    simulation_data_domainIJK(sim, &idom, &jdom, &kdom);

    /* We'll oscillate between 2 boxes.*/
    double t = 0.5 * cos(M_PI/2. + sim->time) + 0.5;
    for(i = 0; i < 6; ++i)
        ext[i] = (1.-t)*sim->extents0[i] + t*sim->extents1[i];
    float sideX = ext[1] - ext[0];
    float sideY = ext[3] - ext[2];
    float sideZ = ext[5] - ext[4];
    float dX = sideX / static_cast<float>(sim->domains[0]);
    float dY = sideY / static_cast<float>(sim->domains[1]);
    float dZ = sideZ / static_cast<float>(sim->domains[2]);
    float x0 = ext[0] + idom * dX;
    float y0 = ext[2] + jdom * dY;
    float z0 = ext[4] + kdom * dZ;
    float x1 = x0 + dX;
    float y1 = y0 + dY;
    float z1 = z0 + dZ;

    // Fill in coordinates and data.
    int index = 0;
    for(k = 0; k < sim->dims[2]; ++k)
    {
        float tz = float(k) / float(sim->dims[2]-1);
        float z = (1.-tz)*z0 + tz*z1;
        for(j = 0; j < sim->dims[1]; ++j)
        {
            float ty = float(j) / float(sim->dims[1]-1);
            float y = (1.-ty)*y0 + ty*y1;
            for(i = 0; i < sim->dims[0]; ++i)
            {
                // Store coordinates
                float tx = float(i) / float(sim->dims[0]-1);
                float x = (1.-tx)*x0 + tx*x1;
                sim->x[index] = x;
                sim->y[index] = y;
                sim->z[index] = z;

                float r = sim->x[index];
                float theta = sim->z[index];
                float phi = sim->y[index];
                Triplex cartesian;
                cartesian.x = r * cos(theta) * sin(phi);
                cartesian.y = r * sin(theta) * sin(phi);
                cartesian.z = r * cos(phi);

                sim->data[index] = mandelbulb(cartesian);

                // Let's save the transformed coordinates. (rotated ry90)
                sim->x[index] = cartesian.z;
                sim->y[index] = cartesian.y;
                sim->z[index] = -cartesian.x;

                ++index;
            }
        }
    }

    // TODO: Transform the coordinates a so we can have a better default view.
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
    double contours[] = {3., 5.};
    int nContours = sizeof(contours) / sizeof(double);
    int nViews = 1;
    char filename[100];
    const char *extractvars[] = {"bulb", "dom", NULL};

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

    while(sim->cycle < sim->maxcycles)
    {  
        /* Update the simulation data for this iteration.*/     
        simulation_data_update(sim);

        /* Tell VisIt that some metadata changed.*/
        VisItTimeStepChanged();

        if(sim->exports)
        {
            /* Set some extract options. */
            extract_set_options(sim->format, 0,0);

            sprintf(filename, "mandelbulbsim_iso_%04d", sim->cycle);
            err = extract_iso(filename, "bulb", contours, nContours, extractvars);
            if(sim->par_rank == 0)
            {
                printf("iso export returned %s\n", extract_err(err));
            }
        }

        if(sim->render)
        {
            for(int i = 0; i < nContours; ++i)
            {
                VisItAddPlot("Pseudocolor", "dom");
                VisItSetPlotOptionsS("colorTableName", "hot_desaturated");
                VisItAddOperator("Isosurface", 1);
                VisItSetOperatorOptionsS("variable", "bulb");
                VisItSetOperatorOptionsI("contourMethod", 1); /* value */
                VisItSetOperatorOptionsDv("contourValue", &contours[i], 1);
                VisItDrawPlots();

                for(int v = 0; v < nViews; ++v)
                {
                    sprintf(filename, "mandelbulbsim_%d_%d_%04d.png",
                            int(contours[i]), v, sim->cycle);

                    if(VisItSaveWindow(filename, sim->image_width, sim->image_height, VISIT_IMAGEFORMAT_PNG) == VISIT_OKAY)
                    {
                        if(sim->par_rank == 0)
                            printf("Saved %s\n", filename);
                    }
                    else if(sim->par_rank == 0)
                        printf("The image could not be saved to %s\n", filename);
                }

                VisItDeleteActivePlots();
            }
        }

        ++sim->cycle;
        sim->time += (M_PI / 10.);
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
        if((i+1) < argc)
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
                sim.exports = atoi(argv[i+1]);
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
        VisItSetOptions(options);

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
            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "bulb");
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
    int npts;
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    npts = sim->dims[0] * sim->dims[1] * sim->dims[2];

    if(strcmp(name, "bulb") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            npts, sim->data);
    }
    else if(strcmp(name, "dom") == 0)
    {
        float *dom = (float *)malloc(npts * sizeof(float));
        for(int i = 0; i < npts; ++i)
            dom[i] = domain;
        VisIt_VariableData_alloc(&h);
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
