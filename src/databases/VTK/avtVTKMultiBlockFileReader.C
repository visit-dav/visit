// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//    avtVTKMultiBlockFileReader.C
// ****************************************************************************


#include <avtDatabaseMetaData.h>
#include <avtVTKMultiBlockFileReader.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


#include <map>
#include <string>
#include <vector>

using std::array;
using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader constructor
//
//  Arguments:
//      fname    The file name.
//
//  Notes:  Pulled from avtVTKFileReader.
//
//  Programmer:  Kathleen Biaga
//  Creation:    June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

avtVTKMultiBlockFileReader::avtVTKMultiBlockFileReader(const char *fname,
    const DBOptionsAttributes *) : avtVTKFileReaderBase(), filename(fname)
{
    nblocks = 1;
    haveReadDataset = false;
    haveReadFile = false;
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::FreeUpResource
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Child
//  Creation:   February 23, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Mark C. Miller, Wed Jul  2 17:27:35 PDT 2014
//    Delete everything even VTK datasets read.
//
//    Kathleen Biagas, Fri Aug 13 2021
//    pieceFileNames now a vector<string>.
//    pieceExtents now vector<array<int,6>>.
//
// ****************************************************************************
void
avtVTKMultiBlockFileReader::FreeUpResources(void)
{
    debug4 << "VTK file " << filename << " forced to free up resources." << endl;

    pieceFileNames.clear();
    if (!pieceDatasets.empty())
    {
        for (size_t i = 0; i < pieceDatasets.size(); i++)
        {
            if (pieceDatasets[i] != NULL)
                pieceDatasets[i]->Delete();
        }
        pieceDatasets.clear();
    }
    pieceExtents.clear();
    haveReadDataset = false;
    haveReadFile = false;
    avtVTKFileReaderBase::FreeUpResources();
}

// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader destructor
//
//  Programmer: Hank Child
//  Creation:   February 23, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
//    Brad Whitlock, Wed Oct 26 11:03:14 PDT 2011
//    Delete curves in vtkCurves.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Burlen Loring, Fri Jul 11 11:19:36 PDT 2014
//    fix alloc-dealloc-mismatch (operator new [] vs free)
//
// ****************************************************************************

avtVTKMultiBlockFileReader::~avtVTKMultiBlockFileReader()
{
    FreeUpResources();
}


// ****************************************************************************
// Method: avtVTKMultiBlockFileReader::GetNumberOfDomain
//
// Purpose:
//   Return the number of domains, reading the data file to figure it out.
//
// Returns:    The number of domains.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 22 17:08:06 PDT 2012
//
// Modifications:
//
// ****************************************************************************

int
avtVTKMultiBlockFileReader::GetNumberOfDomains()
{
    if (!haveReadFile)
    {
        ReadInFile();
    }

    return nblocks;
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

void
avtVTKMultiBlockFileReader::ReadInDataset(int domain)
{
    debug4 << "Reading in dataset from VTK file ";
    debug4 << pieceFileNames[domain];
    debug4 << " (domain = " << domain << ") " << endl;

    //
    // This shouldn't ever happen (since we would already have the dataset
    // we are trying to read from the file sitting in memory), but anything
    // to prevent leaks.
    //
    if (pieceDatasets[domain] != NULL)
    {
        pieceDatasets[domain]->Delete();
        pieceDatasets[domain] = NULL;
    }

    int *ext = nullptr;
    if(!pieceExtents.empty() && !pieceExtents[domain].empty())
       ext = pieceExtents[domain].data();
    pieceDatasets[domain] = ReadVTKDataset(pieceFileNames[domain], ext);
    pieceDatasets[domain]->Register(NULL);
    haveReadDataset = true;
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::GetAuxiliaryData
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

void *
avtVTKMultiBlockFileReader::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *, DestructorFunction &df)
{
    if (!haveReadFile)
    {
        ReadInFile(domain);
    }

    if (pieceDatasets[domain] == NULL)
    {
        ReadInDataset(domain);
    }

    return GetAuxiliaryDataFromDataset(pieceDatasets[domain], var, type, df);
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::GetMesh
//
//  Purpose:
//    Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//    domain   The domain of the dataset.
//    mesh     The desired meshname.
//
//  Returns:    The mesh as a VTK dataset.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtVTKMultiBlockFileReader::GetMesh(int domain, const char *mesh)
{
    if (!haveReadFile)
    {
        ReadInFile(domain);
    }

    debug5 << "Getting mesh from VTK file: " ;
    debug5 << pieceFileNames[domain];
    debug5 << " (domain = " << domain << ") " << endl;

    if (pieceDatasets[domain] == NULL)
    {
        ReadInDataset(domain);
    }

    vtkDataSet *meshDS = GetMeshFromDataset(pieceDatasets[domain], mesh);
    meshDS->Register(NULL);
    return meshDS;
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::GetVar
//
//  Purpose:
//    Gets the variable.
//
//  Arguments:
//    domain   The domain of the dataset.
//    var      The desired varname.
//
//  Returns:    The variable.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtVTKMultiBlockFileReader::GetVar(int domain, const char *real_name)
{
    if (!haveReadFile)
    {
        ReadInFile(domain);
    }

    debug5 << "Getting var from VTK file ";
    debug5 << pieceFileNames[domain];
    debug5 << " (domain = " << domain << ") " << endl;

    if (!haveReadDataset)
    {
        ReadInDataset(domain);
    }

    return GetVarFromDataset(pieceDatasets[domain], real_name);
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 24, 2022
//
//  Modifications:
//
// ****************************************************************************

void
avtVTKMultiBlockFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!haveReadFile)
    {
        ReadInFile();
    }

    if (!haveReadDataset)
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

    // Use only the first 'piece' as we are working under the assumption that
    // all 'pieces' will have the same topology, mesh type,
    // #, names, and types of variable
    vtkDataSet *dataset = pieceDatasets[0];

    // create some dummy vars for non-used mesh meta data (group info)
    string empty;
    vector<string> vs;
    vector<int> vi;
    FillMeshMetaData(md, dataset, useMeshName, 0, empty, vs, vi,
        nblocks, blockPieceName, blockNames);

    // Fill in the variable
    FillVarsMetaData(md, dataset->GetPointData(), useMeshName, AVT_NODECENT);
    FillVarsMetaData(md, dataset->GetCellData(),  useMeshName, AVT_ZONECENT,
                     dataset->GetFieldData());

    // Don't hang on to all the data we've read. We might not even need it
    // if we're in mdserver or of on non-zero mpi-rank.
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::IsEmpty
//
//  Purpose:
//      Returns a flag indicating if the file contains an empty dataset.
//
//  Notes: Originally part of avtVTKFileReader.
//
//  Programmer: Eric Brugger
//  Creation:   June 20, 2017
//
//  Modifications:
//    Kathleen Biagas, Fri August 13, 2021
//    Add call to ReadInDataset if pieceDataset[0] is NULL.
//
// ****************************************************************************

bool
avtVTKMultiBlockFileReader::IsEmpty()
{
    if (!haveReadFile)
    {
        ReadInFile();
    }

    if (pieceDatasets[0] == NULL)
    {
        ReadInDataset(0);
    }

    vtkDataSet *dataset = pieceDatasets[0];

    if (dataset->GetNumberOfCells() == 0 && dataset->GetNumberOfPoints() == 0)
    {
        FreeUpResources();
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::GetTime
//
//  Purpose: Return the time associated with this file
//
//  Notes: Originally part of avtVTKFileReader.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Jun 29, 2006
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul  9 18:14:24 PDT 2008
//    Call ReadInDataset if not done already.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

double
avtVTKMultiBlockFileReader::GetTime()
{
    if (INVALID_TIME == vtk_time && !haveReadFile)
        ReadInFile();
    return vtk_time;
}


// ****************************************************************************
//  Method: avtVTKMultiBlockFileReader::GetCycle
//
//  Purpose: Return the cycle associated with this file
//
//  Notes: Originally part of avtVTKFileReader.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2008
//
//  Modifications:
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

int
avtVTKMultiBlockFileReader::GetCycle()
{
    if (INVALID_CYCLE == vtk_cycle && !haveReadFile)
        ReadInFile();
    return vtk_cycle;
}

