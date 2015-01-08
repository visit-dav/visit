/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                              avtXYZWriter.C                             //
// ************************************************************************* //

#include <avtXYZWriter.h>

#include <vector>

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <AtomicProperties.h>
#include <avtDatabaseMetaData.h>

using namespace std;


// ****************************************************************************
//  Method: avtXYZWriter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June  2, 2009
//
// ****************************************************************************

avtXYZWriter::avtXYZWriter(void)
{
}

// ****************************************************************************
//  Method: avtXYZWriter::OpenFile
//
//  Purpose:
//      We write everything to one file, so open the file here.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  2, 2009
//
// ****************************************************************************

void
avtXYZWriter::OpenFile(const string &stemname, int numblocks)
{
    stem = stemname;
    string file = stem + ".xyz";
    out.open(file.c_str());
}


// ****************************************************************************
//  Method: avtXYZWriter::WriteHeaders
//
//  Purpose:
//      Writes out a header file, e.g. a VisIt file to tie
//      the XYZ files together.  Nothing to do in this case.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  2, 2009
//
// ****************************************************************************

void
avtXYZWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           vector<string> &scalars, vector<string> &vectors,
                           vector<string> &materials)
{
    // nothing to do; we're just going to write everything to a single
    // XYZ fle (as if each chunk was a timestep), so the headers are
    // added on a per-chunk basis
}


// ****************************************************************************
//  Method: avtXYZWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  2, 2009
//
//  Modifications:
//    Jeremy Meredith, Tue Jun 30 12:14:58 EDT 2009
//    Added check for negative atomic numbers.
//
//    Mark C. Miller, Thu Aug 28 15:41:12 PDT 2014
//    Made it less strict about type of vtk dataset that arrives here.
// ****************************************************************************

void
avtXYZWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    int natoms = ds->GetNumberOfPoints();
    if (natoms == 0)
        return;

    // Collect up the data arrays, and find the atomic number one
#define MAX_XYZ_VARS 6
    int nvars = 0;
    vtkDataArray *arrays[MAX_XYZ_VARS];
    vtkDataArray *element = NULL;

    for (int i=0; i<ds->GetPointData()->GetNumberOfArrays(); i++)
    {
        vtkDataArray *arr = ds->GetPointData()->GetArray(i);
        if (strlen(arr->GetName()) >= 7 &&
            strncmp(arr->GetName(),"element",7) == 0)
        {
            element = arr;
        }
        else if (nvars < MAX_XYZ_VARS &&
                 strcmp(arr->GetName(),"avtOriginalNodeNumbers") != 0)
        {
            arrays[nvars++] = arr;
        }
    }


    // Write out the atoms.
    out << "  "<< natoms << endl;
    out << "visit export chunk "<<chunk<<endl;
    for (int a = 0; a < natoms; a++)
    {
        double *coord = ds->GetPoint((vtkIdType)a);

        // Get a viable atomic number
        int atomicNumber = 0;
        if (element)
            atomicNumber = element->GetTuple1((vtkIdType)a);
        if (atomicNumber < 0 || atomicNumber > MAX_ELEMENT_NUMBER)
            atomicNumber = 0;

        // write out the element, coordinates, and any variables
        out << element_names[atomicNumber] << "\t";
        out << coord[0] << "\t";
        out << coord[1] << "\t";
        out << coord[2] << "\t";
        for (int j=0; j<nvars; j++)
            out << arrays[j]->GetTuple1((vtkIdType)a) << "\t";
        out << endl;
    }
}


// ****************************************************************************
//  Method: avtXYZWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  2, 2009
//
// ****************************************************************************

void
avtXYZWriter::CloseFile(void)
{
    out.close();
}
