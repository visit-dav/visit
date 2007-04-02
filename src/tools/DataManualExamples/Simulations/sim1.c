/* SIMPLE SIMULATION SKELETON */

#include <stubs.c>

int main(int argc, char **argv)
{
    read_input_deck();
    do
    {
        simulate_one_timestep();
        write_vis_dump();
    } while(!simulation_done());
    return 0;
}
