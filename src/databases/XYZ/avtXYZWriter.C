// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtXYZWriter.C                               //
// ************************************************************************* //

#include <avtXYZWriter.h>

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <AtomicProperties.h>
#include <avtDatabaseMetaData.h>

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
// ****************************************************************************

void
avtXYZWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    int natoms = ds->GetNumberOfPoints();
    int ncells = ds->GetNumberOfCells();
    if (natoms == 0 || ncells == 0)
        return;

    std::string filename;
    if(writeContext.GroupSize() > 1)
    {
        char ext[20];
        snprintf(ext, 20, ".%d.xyz", writeContext.GroupRank());
        filename = stem + ext;
    }
    else
        filename = stem + ".xyz";

    std::ofstream  out;
    out.open(filename.c_str());
    out << std::scientific << std::setprecision(8);
    

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

