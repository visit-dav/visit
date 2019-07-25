// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V2.h>
#include <stdio.h>
#include <stubs.c>

#include "SimulationExample.h"

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

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("sim2",
        "Added some initialization functions",
        "/path/to/where/sim/was/started", NULL, NULL, NULL);

    read_input_deck(&sim);
    do
    {
        simulate_one_timestep(&sim);
        write_vis_dump(&sim);
    } while(!sim.done);

    simulation_data_dtor(&sim);
    return 0;
}
