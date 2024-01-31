// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtVTKFileFormat.h>

avtVTK_STSDFileFormat::avtVTK_STSDFileFormat(const char *filename, const DBOptionsAttributes *opts) :
    avtSTSDFileFormat(filename)
{
    GetCycleFromFilename(filename);
    reader = new avtVTKFileReader(filename, opts);
}

avtVTK_STSDFileFormat::avtVTK_STSDFileFormat(const char *filename,
    const DBOptionsAttributes *opts, avtVTKFileReader *r) :
    avtSTSDFileFormat(filename)
{
    GetCycleFromFilename(filename);
    reader = r;
}

avtVTK_STSDFileFormat::~avtVTK_STSDFileFormat()
{
    delete reader;
}

const char *
avtVTK_STSDFileFormat::GetType(void)
{
    return "VTK File Format";
}

void
avtVTK_STSDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycle
//
//  Modifications:
//    Kathleen Biagas, Tue Aug 18 11:38:07 PDT 2015
//    Check for INVALID_CYCLE instead of INVALID_TIME.
//
// ****************************************************************************

int
avtVTK_STSDFileFormat::GetCycle(void)
{
    int cycle = reader->GetCycle();
    if( cycle == INVALID_CYCLE )
      return cycleFromFilename;
    else
      return cycle;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetTime
//
//  Modifications:
//    If cycleFromFilename is invalid, return INVALID_TIME.
//
// ****************************************************************************

double
avtVTK_STSDFileFormat::GetTime(void)
{
    double time = reader->GetTime();
    if( time == INVALID_TIME )
    {
        if (cycleFromFilename == INVALID_CYCLE)
           return INVALID_TIME;
      return cycleFromFilename;
    }
    else
      return time;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name
//
//  Notes: Although all this method does is simply call the format's base
//         class implementation of GuessCycle, doing this is a way for the
//         VTK format to "bless" the guesses that that method makes.
//         Otherwise, VisIt wouldn't know that VTK thinks those guesses are
//         good. See notes in avtSTXXFileFormatInterface::SetDatabaseMetaData
//         for further explanation.
//
//  Programmer: Eric Brugger
//  Creation:   August 12, 2005
//
// ****************************************************************************

int
avtVTK_STSDFileFormat::GetCycleFromFilename(const char *f) const
{
    cycleFromFilename = GuessCycle(f);
    return cycleFromFilename;
}

void
avtVTK_STSDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtVTK_STSDFileFormat::GetMesh(const char *name)
{
    return reader->GetMesh(0, name);
}

vtkDataArray *
avtVTK_STSDFileFormat::GetVar(const char *name)
{
    return reader->GetVar(0, name);
}

vtkDataArray *
avtVTK_STSDFileFormat::GetVectorVar(const char *name)
{
    return reader->GetVectorVar(0, name);
}

void *
avtVTK_STSDFileFormat::GetAuxiliaryData(const char *var,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, 0, type, d, df);
}

bool
avtVTK_STSDFileFormat::IsEmpty()
{
    return reader->IsEmpty();
}

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

avtVTK_STMDFileFormat::avtVTK_STMDFileFormat(const char *filename, const DBOptionsAttributes *opts) :
    avtSTMDFileFormat(&filename, 1)
{
    GetCycleFromFilename(filename);
    reader = new avtVTKFileReader(filename, opts);
}

avtVTK_STMDFileFormat::avtVTK_STMDFileFormat(const char *filename,
    const DBOptionsAttributes *opts, avtVTKFileReader *r) :
    avtSTMDFileFormat(&filename, 1)
{
    GetCycleFromFilename(filename);
    reader = r;
}

avtVTK_STMDFileFormat::~avtVTK_STMDFileFormat()
{
    delete reader;
}

const char *
avtVTK_STMDFileFormat::GetType(void)
{
    return "VTK MD File Format";
}

void
avtVTK_STMDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycle
//
//  Modifications:
//    Kathleen Biagas, Tue Aug 18 11:38:07 PDT 2015
//    Check for INVALID_CYCLE instead of INVALID_TIME.
//
// ****************************************************************************

int
avtVTK_STMDFileFormat::GetCycle(void)
{
    int cycle = reader->GetCycle();
    if( cycle == INVALID_CYCLE )
      return cycleFromFilename;
    else
      return cycle;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetTime
//
//  Modifications:
//    If cycleFromFilename is invalid, return INVALID_TIME.
//
// ****************************************************************************

double
avtVTK_STMDFileFormat::GetTime(void)
{
    double time = reader->GetTime();
    if( time == INVALID_TIME )
    {
      if (cycleFromFilename == INVALID_CYCLE)
          return INVALID_TIME;
      return cycleFromFilename;
    }
    else
      return time;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name
//
//  Notes: Although all this method does is simply call the format's base
//         class implementation of GuessCycle, doing this is a way for the
//         VTK format to "bless" the guesses that that method makes.
//         Otherwise, VisIt wouldn't know that VTK thinks those guesses are
//         good. See notes in avtSTXXFileFormatInterface::SetDatabaseMetaData
//         for further explanation.
//
//  Programmer: Eric Brugger
//  Creation:   August 12, 2005
//
// ****************************************************************************

int
avtVTK_STMDFileFormat::GetCycleFromFilename(const char *f) const
{
    cycleFromFilename = GuessCycle(f);
    return cycleFromFilename;
}

void
avtVTK_STMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtVTK_STMDFileFormat::GetMesh(int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtVTK_STMDFileFormat::GetVar(int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtVTK_STMDFileFormat::GetVectorVar(int domain, const char *name)
{
    return reader->GetVectorVar(domain, name);
}

void *
avtVTK_STMDFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, domain, type, d, df);
}

// ****************************************************************************
//   MTMDFileFormat for PVD case.
// ****************************************************************************

#include <avtPVDFileReader.h>

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************


avtPVD_MTMDFileFormat::avtPVD_MTMDFileFormat(const char *filename,
    const DBOptionsAttributes *rdOpts) : avtMTMDFileFormat(filename)
{
    reader = new avtPVDFileReader(filename, rdOpts);
}

avtPVD_MTMDFileFormat::~avtPVD_MTMDFileFormat()
{
    delete reader;
}

const char *
avtPVD_MTMDFileFormat::GetType(void)
{
    return "VTK PVD MD File Format";
}

void
avtPVD_MTMDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

void
avtPVD_MTMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int /*ts */)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtPVD_MTMDFileFormat::GetMesh(int /*ts*/, int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtPVD_MTMDFileFormat::GetVar(int /*ts*/, int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtPVD_MTMDFileFormat::GetVectorVar(int /*ts*/, int domain, const char *name)
{
    return reader->GetVectorVar(domain, name);
}

void *
avtPVD_MTMDFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, domain, type, d, df);
}

void
avtPVD_MTMDFileFormat::ActivateTimestep(int ts)
{
    reader->ActivateTimestep(ts);
}

int
avtPVD_MTMDFileFormat::GetNTimesteps()
{
    return reader->GetNTimes();
}

void
avtPVD_MTMDFileFormat::GetTimes(std::vector<double> &_times)
{
    reader->GetTimes(_times);
}
