/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V1.h>
#include <stdio.h>
#include <stubs.c>

int main(int argc, char **argv)
{
    /* Initialize environment variables. */
    VisItSetupEnvironment();
    /* Write out .sim file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("sim2",
        "Added some VSIL initialization functions",
        "/path/to/where/sim/was/started", NULL, NULL);

    read_input_deck();
    do
    {
        simulate_one_timestep();
        write_vis_dump();
    } while(!simulation_done());
    return 0;
}
