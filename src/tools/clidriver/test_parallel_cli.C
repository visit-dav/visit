// ****************************************************************************
//  File:  test_parallel_cli
//
//  Purpose:
//    MPI program that initiates a separate fullscreen CLI
//    for each task, has each open a separate file, and steps
//    through them all in a synchronized fashion.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 18, 2007
//
// ****************************************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
using namespace std;

#include <mpi.h>

#include "VisItPythonConnection.h"

VisItPythonConnection visit;

int main (int argc, char **argv)
{
    int rank;
    int size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size >= argc)
    {
        if (rank==0)
        {
            cerr << endl;
            cerr << "Usage: "<<argv[0]<<" file_for_task0 file_for_task1 ...\n";
            cerr << endl;
            cerr << "   (make sure if you are running on N processors\n";
            cerr << "    that you have N filenames on the commandline)\n";
            cerr << endl;
        }
        MPI_Finalize();
        return 1;
    }

    cout << "rank="<<rank<<"  size="<<size<<" file="<<argv[rank+1]<<endl;

    // Open connection to a new visit CLI.
    // We want to add the fullscreen argument.
    vector<string> args;
    args.push_back("-fullscreen");
    visit.Open(args);

    // Instruct each CLI to open its file (based on rank) and add plots.
    char opencmd[256];
    sprintf(opencmd, "OpenDatabase('%s')", argv[rank+1]);
    visit.SendCommand(opencmd);
    visit.SendCommand("AddPlot('Molecule', 'element')");
    visit.SendCommand("DrawPlots()");

    // Wait to ensure all visit CLI's are syncronized.
    MPI_Barrier(MPI_COMM_WORLD);

    // Loop twenty times over the first 20 timesteps.
    for (int pass=1; pass<20; pass++)
    {
        for (int ts=0; ts<20; ts++)
        {
            char settimecmd[256];
            sprintf(settimecmd, "TimeSliderSetState(%d)", ts);
            visit.SendCommand(settimecmd);
            // And sync up again
            MPI_Barrier(MPI_COMM_WORLD);
            cout << "rank "<<rank<<" time state set to "<<ts<<endl;
        }
    }

    // Shut down
    visit.Close();
    MPI_Finalize();

    return 0;
}
