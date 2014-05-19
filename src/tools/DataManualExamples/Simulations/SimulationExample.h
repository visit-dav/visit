#ifndef SIMULATION_EXAMPLE_H
#define SIMULATION_EXAMPLE_H
#include <string.h>
#include <stdlib.h>

static char *__sim2_filename__ = NULL;

/*****************************************************************************
 * Method: SimulationExample
 *
 * Purpose: 
 *   This helper function lets simulation examples process command line 
 *   arguments relevant for debugging simulations in a standard way.
 *
 * Arguments:
 *   argc : The command line argument count.
 *   argv : The command line argument array.
 *
 * Returns:    None
 *
 * Note:       We mainly use the -dir option during debugging to tell the
 *             simulations the path to the VisIt directory since we can't
 *             rely on the values that the installed VisIt would provide for
 *             "visit -env" while doing development.
 *
 * Programmer: Brad Whitlock
 * Creation:   Tue Feb 16 16:22:24 PST 2010
 *
 * Modifications:
 *   
 ****************************************************************************/

void
SimulationArguments(int argc, char **argv)
{
    int i;
    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-dir") == 0 &&
           (i+1) < argc)
        {
            VisItSetDirectory(argv[i+1]);
            ++i;
        }
        else if(strcmp(argv[i], "-options") == 0 &&
           (i+1) < argc)
        {
            VisItSetOptions(argv[i+1]);
            ++i;
        }
#ifdef VISIT_CONTROL_INTERFACE_V2_H
        else if(strcmp(argv[i], "-trace") == 0 &&
           (i+1) < argc)
        {
#ifdef PARALLEL
            int rank;
            char *tmpfile = NULL;
            tmpfile = (char*)malloc(strlen(argv[i+1]) + 10);
            MPI_Comm_rank (MPI_COMM_WORLD, &rank);
            sprintf(tmpfile, "%s.%04d", argv[i+1], rank);
            VisItOpenTraceFile(tmpfile);
            free(tmpfile);
#else
            VisItOpenTraceFile(argv[i+1]);
#endif
            ++i;
        }
#endif
        else if(strcmp(argv[i], "-sim2") == 0 &&
               (i+1) < argc)
        {
            __sim2_filename__ = argv[i+1];
            ++i;
        }
    }
}

char *
SimulationFilename(void)
{
    return __sim2_filename__;
}

/*****************************************************************************
 * Method: sim_sleep
 *
 * Purpose: 
 *   This helper function lets simulation examples sleep so it looks like they
 *   are busy.
 *
 * Arguments:
 *   sec : The number of seconds
 *
 * Returns:    None
 *
 * Programmer: Brad Whitlock
 * Creation:   Tue Feb 16 16:22:24 PST 2010
 *
 * Modifications:
 *   
 ****************************************************************************/

#ifdef _WIN32
#include <Windows.h>
void
sim_sleep(int sec)
{
    Sleep(sec * 1000);
}
#else
#include <unistd.h>
void
sim_sleep(int sec)
{
    sleep(sec);
}
#endif

#endif
