/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    double xmin = -10., xmax = 10.;
    double ymin = 0., ymax = 30.;
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
