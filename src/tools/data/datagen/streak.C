// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visitstream.h>
#include <math.h>
#include <stdio.h>

//
// Include classes that help write PDB files.
//
#include <pdbhelper.C>

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the program.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov  7 11:31:37 PST 2008
//
// Modifications:
//
// ****************************************************************************

int
main(int, char *[])
{
    int timeStates = 15;
    int kmax = 3, lmax = 5;

    FieldWithData<double> rt(timeStates, 1, kmax, lmax, 0., true);
    FieldWithData<double> zt(timeStates, 1, kmax, lmax, 0., true);
    FieldWithData<int>  ireg(timeStates, 1, kmax, lmax, 0, false);

    //
    // Populate the rt and zt arrays
    //
    for(int t = 0; t < timeStates; ++t)
    {
        int ii = t;
        for(int j = 0; j < lmax; ++j)
        {
            for(int i = 0; i < kmax; ++i)
            {
                rt.SetValue(t, 0, i, j, (double)j);
                zt.SetValue(t, 0, i, j, (double)i);
                ireg.SetValue(t, 0, i, j, ii++);
            }
        }
    }

    //
    // Create the PDB file and set up the fields that we want to write to it.
    //
    FieldWriter pdb;
    for(int t = 0; t < timeStates; ++t)
    {
         // Figure out the cycle and the time.
         int   cycle = t;
         double time = double(t) * 1.1111111111111;

         // Add a new time state.
         pdb.AddTimeStep(cycle, time);
    }

    //
    // Add fields to the pdb file.
    //
    pdb.AddField("rt", "rt@value", &rt);
    pdb.AddField("zt", "zt@value", &zt);
    pdb.AddField("ireg", "ireg@value", &ireg);

    //
    // Write the data to a set of PDB files that each have multiple times.
    //
    pdb.SetDatabaseComment("Flash database: multiple time states per file.");
    pdb.WriteData("streak_multi", kmax, lmax, 10, true);

    //
    // Write the data to a set of individual PDB files.
    //
    pdb.SetDatabaseComment("Flash database: 1 time state per file.");
    pdb.WriteData("streak_family", kmax, lmax, 1, true);

    //
    // Write the data to a single PDB file.
    //
    pdb.SetDatabaseComment("Flash database: all time states in one file.");
    pdb.WriteData("streak_allinone", kmax, lmax, timeStates, true);

    // Write out the streak files.
    const char *streak_files[] = {
        "streak_allinone.pdb.streak",
        "streak_family00.pdb.streak",
        "streak_multi00.pdb.streak"};
    for(int i = 0; i < 3; ++i)
    {
        FILE *f = fopen(streak_files[i], "wt");
        if(f != 0)
        {
            fprintf(f, "streakplot STREAK_I times@history rt@value ireg@value I 1 1. 0. off off\n");
            fprintf(f, "streakplot STREAK_J times@history zt@value ireg@value J 1 1. 0. off off\n");
            fclose(f);
        }
    }

    return 0;
}
