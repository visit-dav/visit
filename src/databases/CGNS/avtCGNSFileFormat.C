// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCGNSFileFormat.C                            //
// ************************************************************************* //

#include <avtCGNSFileFormat.h>

#include <avtCGNSFileReader.h>
#include <avtDatabaseMetaData.h>

#include <InvalidFilesException.h>

#include <string.h>

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader constructor
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//    Eric Brugger, Thu Jul  2 10:56:36 PDT 2020
//    Corrected a bug that caused a crash when doing a Subset plot of "zones"
//    when reading data decomposed across multiple CGNS files.
//
// ****************************************************************************

avtCGNS_MTMDFileFormat::avtCGNS_MTMDFileFormat(const char *filename) :
    avtMTMDFileFormat(filename)
{
    reader = new avtCGNSFileReader(filename, true);
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::~avtCGNS_MTMDFileReader
//
//  Purpose:
//    Destructor for the avtCGNS_MTMDFileReader class.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

avtCGNS_MTMDFileFormat::~avtCGNS_MTMDFileFormat()
{
    delete reader;
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::GetCycles
//
//  Purpose:
//    Returns the cycles for the database.
//
// Arguments:
//   c : The return vector for the cycles.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTMDFileFormat::GetCycles(std::vector<int> &c)
{
    reader->GetCycles(c);
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::GetTimes
//
//  Purpose:
//    Returns the times for the database.
//
// Arguments:
//   t : The return vector for the times.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTMDFileFormat::GetTimes(std::vector<double> &t)
{
    reader->GetTimes(t);
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::GetNTimesteps
//
//  Purpose:
//    Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

int
avtCGNS_MTMDFileFormat::GetNTimesteps(void)
{
    return reader->GetNTimesteps();
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::FreeUpResources
//
//  Purpose:
//    When VisIt is done focusing on a particular timestep, it asks that
//    timestep to free up any resources (memory, file descriptors) that
//    it has associated with it. This method is the mechanism for doing
//    that.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTMDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::GetMesh
//
//  Purpose:
//    Gets the mesh associated with this file. The mesh is returned as a
//    derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//    vtkUnstructuredGrid, etc).
//
//  Arguments:
//    timestate   The index of the timestate. If GetNTimesteps returned
//                'N' time steps, this is guaranteed to be between 0 and N-1.
//    domain      The index of the domain. If there are NDomains, this
//                value is guaranteed to be between 0 and NDomains-1,
//                regardless of block origin.
//    meshname    The name of the mesh of interest. This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCGNS_MTMDFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    return reader->GetMesh(timestate, domain, meshname);
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::GetVar
//
//  Purpose:
//    Gets a scalar variable associated with this file. The variable is
//    returned as a vtkDataArray.
//
//  Arguments:
//    timestate   The index of the timestate. If GetNTimesteps returned
//                'N' time steps, this is guaranteed to be between 0 and N-1.
//    domain      The index of the domain. If there are NDomains, this
//                value is guaranteed to be between 0 and NDomains-1,
//                regardless of block origin.
//    meshname    The name of the mesh of interest. This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtCGNS_MTMDFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    return reader->GetVar(timestate, domain, varname);
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::GetVectorVar
//
//  Purpose:
//    Gets a vector variable associated with this file. The variable is
//    returned as a vtkDataArray.
//
//  Arguments:
//    timestate   The index of the timestate. If GetNTimesteps returned
//                'N' time steps, this is guaranteed to be between 0 and N-1.
//    domain      The index of the domain. If there are NDomains, this
//                value is guaranteed to be between 0 and NDomains-1,
//                regardless of block origin.
//    meshname    The name of the mesh of interest. This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtCGNS_MTMDFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
    return reader->GetVectorVar(timestate, domain, varname);
}

// ****************************************************************************
//  Method: avtCGNS_MTMDFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//    This database meta-data object is like a table of contents for the
//    file. By populating it, you are telling the rest of VisIt what
//    information it can request from you.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timestate)
{
    reader->PopulateDatabaseMetaData(md, timestate);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader constructor
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//    Eric Brugger, Thu Jul  2 10:56:36 PDT 2020
//    Corrected a bug that caused a crash when doing a Subset plot of "zones"
//    when reading data decomposed across multiple CGNS files.
//
// ****************************************************************************

avtCGNS_MTSDFileFormat::avtCGNS_MTSDFileFormat(const char *filename) :
    avtMTSDFileFormat(&filename, 1)
{
    cgnsFileName = new char[strlen(filename) + 1];
    strcpy(cgnsFileName, filename);

    reader = new avtCGNSFileReader(filename, false);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::~avtCGNS_MTMDFileReader
//
//  Purpose:
//    Destructor for the avtCGNS_MTMDFileReader class.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

avtCGNS_MTSDFileFormat::~avtCGNS_MTSDFileFormat()
{
    delete [] cgnsFileName;

    delete reader;
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::GetCycles
//
//  Purpose:
//    Returns the cycles for the database.
//
// Arguments:
//   c : The return vector for the cycles.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTSDFileFormat::GetCycles(std::vector<int> &c)
{
    reader->GetCycles(c);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::GetTimes
//
//  Purpose:
//    Returns the times for the database.
//
// Arguments:
//   t : The return vector for the times.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTSDFileFormat::GetTimes(std::vector<double> &t)
{
    reader->GetTimes(t);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::GetNTimesteps
//
//  Purpose:
//    Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

int
avtCGNS_MTSDFileFormat::GetNTimesteps(void)
{
    return reader->GetNTimesteps();
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::FreeUpResources
//
//  Purpose:
//    When VisIt is done focusing on a particular timestep, it asks that
//    timestep to free up any resources (memory, file descriptors) that
//    it has associated with it. This method is the mechanism for doing
//    that.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTSDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::GetMesh
//
//  Purpose:
//    Gets the mesh associated with this file. The mesh is returned as a
//    derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//    vtkUnstructuredGrid, etc).
//
//  Arguments:
//    timestate   The index of the timestate. If GetNTimesteps returned
//                'N' time steps, this is guaranteed to be between 0 and N-1.
//    meshname    The name of the mesh of interest. This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCGNS_MTSDFileFormat::GetMesh(int timestate, const char *meshname)
{
    return reader->GetMesh(timestate, 0, meshname);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::GetVar
//
//  Purpose:
//    Gets a scalar variable associated with this file. The variable is
//    returned as a vtkDataArray.
//
//  Arguments:
//    timestate   The index of the timestate. If GetNTimesteps returned
//                'N' time steps, this is guaranteed to be between 0 and N-1.
//    meshname    The name of the mesh of interest. This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtCGNS_MTSDFileFormat::GetVar(int timestate, const char *varname)
{
    return reader->GetVar(timestate, 0, varname);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::GetVectorVar
//
//  Purpose:
//    Gets a vector variable associated with this file. The variable is
//    returned as a vtkDataArray.
//
//  Arguments:
//    timestate   The index of the timestate. If GetNTimesteps returned
//                'N' time steps, this is guaranteed to be between 0 and N-1.
//    meshname    The name of the mesh of interest. This can be ignored if
//                there is only one mesh.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtCGNS_MTSDFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return reader->GetVectorVar(timestate, 0, varname);
}

// ****************************************************************************
//  Method: avtCGNS_MTSDFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//    This database meta-data object is like a table of contents for the
//    file. By populating it, you are telling the rest of VisIt what
//    information it can request from you.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Feb 28 13:48:04 PST 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtCGNS_MTSDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timestate)
{
    reader->PopulateDatabaseMetaData(md, timestate);

    //
    // Check that there is a single mesh.
    //
    stringVector names = md->GetAllMeshNames();
    if (names.size() != 1)
        EXCEPTION1(InvalidFilesException, cgnsFileName);
    if (md->GetNDomains(names[0]) != 1)
        EXCEPTION1(InvalidFilesException, cgnsFileName);
}
