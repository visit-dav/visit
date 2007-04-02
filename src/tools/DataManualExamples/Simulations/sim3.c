/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V1.h>
#include <stdio.h>

#include <stubs.c>

void mainloop(void)
{
    do
    {
        simulate_one_timestep();
        write_vis_dump();
    } while(!simulation_done());
}

int main(int argc, char **argv)
{
    /* Initialize envuronment variables. */
    VisItSetupEnvironment();
    /* Write out .sim file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("sim3",
        "Moved do..while to mainloop function",
        "/path/to/where/sim/was/started", NULL, NULL);

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

    return 0;
}
