/* DUMMY IMPLEMENTATIONS */
#include <unistd.h>

#ifdef PARALLEL
extern int par_rank;
extern int par_size;
#endif

void simulate_one_timestep(void)
{
    /* simulate 1 time step. */
#ifdef PARALLEL
    printf("%d/%d: Simulating time step\n", par_rank, par_size);
#else
    printf("Simulating time step\n");
#endif
    sleep(1);
}

void read_input_deck(void)
{
    /* Read in problem setup. */
}

void write_vis_dump(void)
{
    /* Write visualization dump. */
}

int simulation_done(void)
{
    return 0;
}
