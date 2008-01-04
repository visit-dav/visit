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
//                               text2polys.C                                //
// ************************************************************************* //

// ****************************************************************************
//  Program: text2polys
//
//  Purpose:
//      Takes in an input string and creates a .vtk file that contains
//      a polygon representation of the input string.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2002
//
// ****************************************************************************

#include <stdlib.h>

#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>

#include <arial_font.h>


// ****************************************************************************
//  Function: Usage
//
//  Purpose:
//      Explains the appropriate usage of text2polys and exits.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2002
//
// ****************************************************************************

void Usage(void)
{
    cerr << "Usage: text2polys <output-file-name> [-2D] TEXT" << endl;
    exit(EXIT_FAILURE);
}


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      Parses the command line arguments and converts the desired string to 
//      polygonal data.  Writes that data out to a file.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2002
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    //
    // Parse the command line arguments.
    //
    if (argc < 3)
    {
        Usage();
    }
    char *output_file_name = argv[1];
    bool make3D = true;
    int first_arg = 2;
    if (strcmp(argv[2], "-2D") == 0)
    {
        if (argc == 3)
        {
            Usage();
        }
        first_arg = 3;
        make3D = false;
    }
    
    vtkPolyData *output = CreateText(argc-first_arg, argv+first_arg);
    if (make3D)
    {
        vtkPoints *pts = output->GetPoints();
        if (pts->GetNumberOfPoints() > 0)
        {
            double pt[3];
            pts->GetPoint(0, pt);
            pt[2] = 0.0001;
            pts->SetPoint(0, pt);
        }
    }

    //
    // Now write the output to a file.
    //
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetInput(output);
    writer->SetFileName(output_file_name);
    writer->Write();

    writer->Delete();
    output->Delete();
}


