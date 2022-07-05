// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVTKMultiGroupFileReader.C
// ****************************************************************************


#include <avtVTKMultiGroupFileReader.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtVTKMultiGroupFileReader constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Kathleen Biagas
//  Creation:    June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

avtVTKMultiGroupFileReader::avtVTKMultiGroupFileReader(const char *fname,
    const DBOptionsAttributes *opts) : avtVTKMultiBlockFileReader(fname, opts)
{
}


// ****************************************************************************
//  Method: avtVTKMultiGroupFileReader destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
// ****************************************************************************

avtVTKMultiGroupFileReader::~avtVTKMultiGroupFileReader()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtVTKMultiGroupFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Kathleen Biagas
//  Creation:   Juny 24, 2022
//
//  Modifications:
//
// ****************************************************************************

void
avtVTKMultiGroupFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!haveReadFile)
    {
        ReadInFile();
    }

    if (pieceDatasets[0] == NULL)
    {
        ReadInDataset(0);
    }

    string useMeshName;
    if(vtk_meshname.empty())
    {
        useMeshName = MESHNAME;
    }
    else
    {
        useMeshName = vtk_meshname;
    }

    FillMeshMetaData(md, pieceDatasets[0], useMeshName,
        ngroups, groupPieceName, groupNames,
        groupIds, nblocks, blockPieceName, blockNames);

    FillVarsMetaData(md, pieceDatasets[0]->GetPointData(),
        useMeshName, AVT_NODECENT);
    FillVarsMetaData(md, pieceDatasets[0]->GetCellData(),
        useMeshName, AVT_ZONECENT, pieceDatasets[0]->GetFieldData());

    // Don't hang on to all the data we've read. We might not even need it
    // if we're in mdserver or of on non-zero mpi-rank.
    FreeUpResources();
}

