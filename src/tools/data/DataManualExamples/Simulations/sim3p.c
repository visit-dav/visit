// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/* SIMPLE PARALLEL SIMULATION SKELETON */
#include <VisItControlInterface_V2.h>
#include <stdio.h>

#include <stubs.c>

#ifdef PARALLEL
#include <mpi.h>

static int visit_broadcast_int_callback(int *value, int sender)
{
    return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
}

static int visit_broadcast_string_callback(char *str, int len, int sender)
{
    return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
#endif

#include "SimulationExample.h"

/******************************************************************************
 *
 * Purpose: New function to contain the program's main loop. 
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:35:53 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(simulation_data *sim)
{
    do
    {
        simulate_one_timestep(sim);
        write_vis_dump(sim);
    } while(!sim->done);
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

    /* Initialize environment variables. */
    VisItSetupEnvironment();

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &sim.par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &sim.par_size);

    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell VSIL whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);
#endif

    /* Write out .sim2 file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    if(sim.par_rank == 0)
    {
        VisItInitializeSocketAndDumpSimFile("sim3p",
        "Moved do..while to mainloop function",
        "/path/to/where/sim/was/started", NULL, NULL, NULL);
    }

    /* Read input problem setup, geometry, data. */
    read_input_deck(&sim);

    /* Call the main loop. */
    mainloop(&sim);

    simulation_data_dtor(&sim);

#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}


