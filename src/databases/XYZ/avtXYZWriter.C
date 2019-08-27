/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                              avtXYZWriter.C                               //
// ************************************************************************* //

#include <avtXYZWriter.h>

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <AtomicProperties.h>
#include <avtDatabaseMetaData.h>
#include <DebugStream.h>

#include <InvalidDBTypeException.h>

#include <fstream>
#include <vector>

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
                           const vector<string> &scalars,
                           const vector<string> &vectors,
                           const vector<string> &materials)
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
//
//    Mark C. Miller, Wed Jan 24 12:26:30 PST 2018
//    Added logic to output only points referenced by cells of non-zero size.    
//
//    Eddie Rusu, Fri Aug 23 11:21:05 PDT 2019
//    Writer can now write cell-centered data if the cell data is exclusively
//    composed of VTK_VERTEX cells. Warning issued if not.
// ****************************************************************************

void
avtXYZWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    debug5 << "Entering avtXYZWriter::WriteChunk(vtkDataSet*, int)"
           << std::endl;
    int natoms = ds->GetNumberOfPoints();
    int ncells = ds->GetNumberOfCells();
    if (natoms == 0 || ncells == 0)
        return;

    std::string filename;
    if(writeContext.GroupSize() > 1)
    {
        char ext[20];
        SNPRINTF(ext, 20, ".%d.xyz", writeContext.GroupRank());
        filename = stem + ext;
    }
    else
        filename = stem + ".xyz";

    std::ofstream  out;
    out.open(filename.c_str());
    out << std::scientific << std::setprecision(8);

    // XYZ plugin is designed to work with point data. However, in the VTK
    // universe, a VTK_VERTEX can be treated as a point. So, we use this logic:
    // If the data exists over point arrays, then just use those.
    // Otherwise, if the data exists over cell arrays and all the cells are
    // VTK_VERTEX, then treat those as points
    // Otherwise, produce a warning and proceed as with points.
    vtkDataSetAttributes *dsa;
    if (ds->GetPointData()->GetNumberOfArrays() == 0)
    {
        // There is no point data, so look for VTK_VERTEX only cells
        vtkCellTypes *cellTypes = vtkCellTypes::New();
        ds->GetCellTypes(cellTypes);
        if (cellTypes->GetNumberOfTypes() == 1 && cellTypes->IsType(VTK_VERTEX) == 1)
        {
            // There is only one type of cell and that is VTK_VERTEX, so
            // treat it like points
            dsa = ds->GetCellData();
        }
        else
        {
            // Data not over points and does not satisfy cell requirements.
            // Throw an exception and don't write because written output
            // would be meaningless.
            EXCEPTION1(InvalidDBTypeException, "XYZ Writer can only write "
                "node-centered variables. Recenter the variable(s) you want "
                "to include to the nodes.")
        }
        cellTypes->Delete();
    }
    else
    {
        // There is point data, so treat it like points
        dsa = ds->GetPointData();
    }
    

    // Collect up the data arrays, and find the atomic number one
#define MAX_XYZ_VARS 6
    int nvars = 0;
    vtkDataArray *arrays[MAX_XYZ_VARS];
    vtkDataArray *element = NULL;

    for (int i=0; i<dsa->GetNumberOfArrays(); i++)
    {
        vtkDataArray *arr = dsa->GetArray(i);
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

    // Tag which points are used by cells
    vector<bool> usedpoints(natoms, false);
    for (int c = 0; c < ncells; c++)
    {
        vtkCell *cell = ds->GetCell(c);
        for (int j = 0 ; j < cell->GetNumberOfPoints() ; j++)
            usedpoints[cell->GetPointId(j)] = true;
    }

    int nused = 0;
    for (int a = 0; a < natoms; a++)
        if (usedpoints[a]) nused++;

    // Write out the atoms.
    out << "  "<< nused << endl;
    out << "visit export chunk "<<chunk<<endl;
    for (int a = 0; a < natoms; a++)
    {
        if (!usedpoints[a]) continue;

        double *coord = ds->GetPoint((vtkIdType)a);

        // Get a viable atomic number
        int atomicNumber = -1;
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

    out.close();
    debug5 << "Exiting  avtXYZWriter::WriteChunk(vtkDataSet*, int)"
           << std::endl;
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
}

// ****************************************************************************
//  Method: avtXYZWriter::GetCombineMode
//
//  Purpose:
//     Provides a hint to the export mechanism to tell it how to combine data.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Sep  8 15:36:45 PDT 2015
//
// ****************************************************************************

avtDatabaseWriter::CombineMode
avtXYZWriter::GetCombineMode(const std::string &) const
{
    return CombineAll;
}

