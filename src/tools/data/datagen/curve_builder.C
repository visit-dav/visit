// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 curve_builder.C                           //
// ************************************************************************* //

#include <stdio.h>
#include <math.h>

// Cyrus Harrison, Mon Feb  1 13:50:34 PST 2010
// changed to use fstream directry instead of of visitstream.h

#include <fstream>
using namespace std;

// ****************************************************************************
//  Function: main
//
//  Purpose:
//      Creates a time animation of four curves.
//
//  Programmer: Hank Childs
//  Creation:   January 15, 2003
//
//  Modifications:
//  Kathleen Bonnell, Fri Oct 28 14:27:12 PDT 2005
//  Added TIME to beginning of files.
//
// ****************************************************************************

int main()
{
    int  i, j;

    //
    // First write out the .visit file.
    //
    int ntimesteps = 100;
    ofstream visit_file("curve.visit");
    for (i = 0 ; i < ntimesteps ; i++)
    {
        char filename[1024];
        sprintf(filename, "c%03d.curve", i);
        visit_file << filename << endl;
    }

    //
    // Now write out each of timesteps.
    //
    int curve_resolution = 100;
    double timeDelta = 0.123;
    for (i = 0 ; i < ntimesteps ; i++)
    {
        char filename[1024];
        sprintf(filename, "c%03d.curve", i);
        ofstream ofile(filename);
        ofile << "# TIME " << i*timeDelta << endl;
        ofile << "# going_up" << endl;
        for (j = 0 ; j < curve_resolution ; j++)
        {
            double proportion = ((double)j) / (curve_resolution-1.);
            double prop2 = ((double) i) / (ntimesteps-1);
            ofile << proportion << " " << pow(proportion, 1.+3*prop2) << endl;
        }
        ofile << "# flat" << endl;
        for (j = 0 ; j < curve_resolution ; j++)
        {
            double proportion = ((double)j) / (curve_resolution-1.);
            ofile << proportion << " 0.5" << endl;
        }
        ofile << "# going_down" << endl;
        for (j = 0 ; j < curve_resolution ; j++)
        {
            double proportion = ((double)j) / (curve_resolution-1.);
            double prop2 = ((double) i) / (ntimesteps-1);
            ofile << proportion << " " << 1.-pow(proportion, 1.+3*prop2) 
                  << endl;
        }
        ofile << "# parabolic" << endl;
        for (j = 0 ; j < curve_resolution ; j++)
        {
            double proportion = ((double)j) / (curve_resolution-1.);
            double prop2 = ((double) i) / (ntimesteps-1);
            ofile << proportion << " " << 4*(1.-proportion)*proportion*prop2 
                  << endl;
        }
    }
}


