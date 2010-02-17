#ifndef SIMULATION_EXAMPLE_H
#define SIMULATION_EXAMPLE_H
#include <string.h>

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
            VisItSetDirectory(argv[i]);
            ++i;
        }
        else if(strcmp(argv[i], "-options") == 0 &&
           (i+1) < argc)
        {
            VisItSetOptions(argv[i]);
            ++i;
        }
#ifdef VISIT_CONTROL_INTERFACE_V2_H
        else if(strcmp(argv[i], "-trace") == 0 &&
           (i+1) < argc)
        {
            VisItOpenTraceFile(argv[i]);
            ++i;
        }
#endif
    }
}

#endif
