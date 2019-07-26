// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/* DUMMY IMPLEMENTATIONS */
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int     cycle;
    double  time;
    int     runMode;
    int     done;
    int     par_rank;
    int     par_size;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = 1; /* VISIT_SIMMODE_RUNNING */
    sim->done = 0;
    sim->par_rank = 0;
    sim->par_size = 1;
}

void
simulation_data_dtor(simulation_data *sim)
{
}

void simulate_one_timestep(simulation_data *sim)
{
    /* simulate 1 time step. */
    ++sim->cycle;
    sim->time += 0.0134;

#ifdef PARALLEL
    printf("%d/%d: Simulating time step: cycle=%d, time=%lg\n",
           sim->par_rank, sim->par_size, sim->cycle, sim->time);
#else
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);    printf("Simulating time step\n");
#endif
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
}

void read_input_deck(simulation_data *sim)
{
    /* Read in problem setup. */
}

void write_vis_dump(simulation_data *sim)
{
    /* Write visualization dump. */
}
