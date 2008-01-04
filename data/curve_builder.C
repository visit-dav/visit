/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                                 curve_builder.C                           //
// ************************************************************************* //

#include <visitstream.h>
#include <stdio.h>
#include <math.h>

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


