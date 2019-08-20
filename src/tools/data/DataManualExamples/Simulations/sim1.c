// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/* SIMPLE SIMULATION SKELETON */

#include <stubs.c>

int main(int argc, char **argv)
{
    simulation_data sim;
    simulation_data_ctor(&sim);

    read_input_deck(&sim);
    do
    {
        simulate_one_timestep(&sim);
        write_vis_dump(&sim);
    } while(!sim.done);

    simulation_data_dtor(&sim);
    return 0;
}
