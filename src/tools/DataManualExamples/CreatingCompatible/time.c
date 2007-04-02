#include <silo.h>

void read_input_deck(void)       { }
void simulate_one_timestep(void) { }
int  simulation_done(void)       { return 0; }

/* SIMPLE SIMULATION SKELETON */
void write_vis_dump(int cycle)
{
    DBfile *dbfile = NULL;
    /* Create a unique filename for the new Silo file*/
    char filename[100];
    sprintf(filename, "output%04d.silo", cycle);
    /* Open the Silo file */
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL,
       "simulation time step", DB_HDF5);
    /* Add other Silo calls to write data here. */

    /* Close the Silo file. */
    DBClose(dbfile);
}

int main(int argc, char **argv)
{
    int cycle = 0;
    read_input_deck();
    do
    {
        simulate_one_timestep();
        write_vis_dump(cycle);
        cycle = cycle + 1;
    } while(!simulation_done());

    return 0;
}
