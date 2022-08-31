// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtVTKFileFormat.h>

#include <avtVTKFileReader.h>

avtVTK_STSDFileFormat::avtVTK_STSDFileFormat(const char *filename, const DBOptionsAttributes *opts) :
    avtSTSDFileFormat(filename)
{
    GetCycleFromFilename(filename);
    reader = new avtVTKFileReader(filename, opts);
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
    return reader->GetMesh(name);
}

vtkDataArray *
avtVTK_STSDFileFormat::GetVar(const char *name)
{
    return reader->GetVar(name);
}

vtkDataArray *
avtVTK_STSDFileFormat::GetVectorVar(const char *name)
{
    // Vector retrieved same as other var.
    return reader->GetVar(name);
}

void *
avtVTK_STSDFileFormat::GetAuxiliaryData(const char *var,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, type, d, df);
}

bool
avtVTK_STSDFileFormat::IsEmpty()
{
    return reader->IsEmpty();
}

// ****************************************************************************
//   STMDFileFormat for .pvt? case.
// ****************************************************************************

#include <avtPVTKFileReader.h>

avtPVTKFileFormat::avtPVTKFileFormat(const char *filename, const DBOptionsAttributes *opts) :
    avtSTMDFileFormat(&filename, 1)
{
    GetCycleFromFilename(filename);
    reader = new avtPVTKFileReader(filename, opts);
}

avtPVTKFileFormat::~avtPVTKFileFormat()
{
    delete reader;
}

const char *
avtPVTKFileFormat::GetType(void)
{
    return "VTK MD File Format";
}

void
avtPVTKFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

// ****************************************************************************
//  Method: avtPVTKFileFormat::GetCycle
//
//  Modifications:
//    Kathleen Biagas, Tue Aug 18 11:38:07 PDT 2015
//    Check for INVALID_CYCLE instead of INVALID_TIME.
//
// ****************************************************************************

int
avtPVTKFileFormat::GetCycle(void)
{
    int cycle = reader->GetCycle();
    if( cycle == INVALID_CYCLE )
      return cycleFromFilename;
    else
      return cycle;
}

// ****************************************************************************
//  Method: avtPVTKFileFormat::GetTime
//
//  Modifications:
//    If cycleFromFilename is invalid, return INVALID_TIME.
//
// ****************************************************************************

double
avtPVTKFileFormat::GetTime(void)
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
avtPVTKFileFormat::GetCycleFromFilename(const char *f) const
{
    cycleFromFilename = GuessCycle(f);
    return cycleFromFilename;
}

void
avtPVTKFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtPVTKFileFormat::GetMesh(int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtPVTKFileFormat::GetVar(int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtPVTKFileFormat::GetVectorVar(int domain, const char *name)
{
    // Vector retrieved same as other var.
    return reader->GetVar(domain, name);
}

void *
avtPVTKFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, domain, type, d, df);
}

// ****************************************************************************
//   STMDFileFormat for .vtm case.
// ****************************************************************************

#include <avtVTMFileReader.h>

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

avtVTMFileFormat::avtVTMFileFormat(const char *filename, const DBOptionsAttributes *opts) :
    avtSTMDFileFormat(&filename, 1)
{
    GetCycleFromFilename(filename);
    reader = new avtVTMFileReader(filename, opts);
}

avtVTMFileFormat::~avtVTMFileFormat()
{
    delete reader;
}

const char *
avtVTMFileFormat::GetType(void)
{
    return "VTK MD File Format";
}

void
avtVTMFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

int
avtVTMFileFormat::GetCycle(void)
{
    int cycle = reader->GetCycle();
    if( cycle == INVALID_CYCLE )
      return cycleFromFilename;
    else
      return cycle;
}

double
avtVTMFileFormat::GetTime(void)
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

int
avtVTMFileFormat::GetCycleFromFilename(const char *f) const
{
    cycleFromFilename = GuessCycle(f);
    return cycleFromFilename;
}

void
avtVTMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtVTMFileFormat::GetMesh(int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtVTMFileFormat::GetVar(int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtVTMFileFormat::GetVectorVar(int domain, const char *name)
{
    // Vector retrieved same as other var.
    return reader->GetVar(domain, name);
}

void *
avtVTMFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, domain, type, d, df);
}


// ****************************************************************************
//   STMDFileFormat for GEOSX .vtm case.
// ****************************************************************************

#include <avtGEOSXFileReader.h>

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

avtGEOSXFileFormat::avtGEOSXFileFormat(const char *filename, const DBOptionsAttributes *opts) :
    avtSTMDFileFormat(&filename, 1)
{
    GetCycleFromFilename(filename);
    reader = new avtGEOSXFileReader(filename, opts);
}

avtGEOSXFileFormat::~avtGEOSXFileFormat()
{
    delete reader;
}

const char *
avtGEOSXFileFormat::GetType(void)
{
    return "GEOSX";
}

void
avtGEOSXFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

int
avtGEOSXFileFormat::GetCycle(void)
{
    int cycle = reader->GetCycle();
    if( cycle == INVALID_CYCLE )
      return cycleFromFilename;
    else
      return cycle;
}

double
avtGEOSXFileFormat::GetTime(void)
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

int
avtGEOSXFileFormat::GetCycleFromFilename(const char *f) const
{
    cycleFromFilename = GuessCycle(f);
    return cycleFromFilename;
}

void
avtGEOSXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtGEOSXFileFormat::GetMesh(int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtGEOSXFileFormat::GetVar(int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtGEOSXFileFormat::GetVectorVar(int domain, const char *name)
{
    // Vector retrieved same as other var.
    return reader->GetVar(domain, name);
}


// ****************************************************************************
//   MTMDFileFormat for PVD case.
// ****************************************************************************

#include <avtPVDFileReader.h>

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************


avtPVDFileFormat::avtPVDFileFormat(const char *filename,
    const DBOptionsAttributes *rdOpts) : avtMTMDFileFormat(filename)
{
    reader = new avtPVDFileReader(filename, rdOpts);
}

avtPVDFileFormat::~avtPVDFileFormat()
{
    delete reader;
}

const char *
avtPVDFileFormat::GetType(void)
{
    return "VTK PVD MD File Format";
}

void
avtPVDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

void
avtPVDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int /*ts */)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtPVDFileFormat::GetMesh(int /*ts*/, int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtPVDFileFormat::GetVar(int /*ts*/, int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtPVDFileFormat::GetVectorVar(int /*ts*/, int domain, const char *name)
{
    // Vector retrieved same as other var.
    return reader->GetVar(domain, name);
}

void *
avtPVDFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, domain, type, d, df);
}

void
avtPVDFileFormat::ActivateTimestep(int ts)
{
    reader->ActivateTimestep(ts);
}

int
avtPVDFileFormat::GetNTimesteps()
{
    return reader->GetNTimes();
}

void
avtPVDFileFormat::GetTimes(std::vector<double> &_times)
{
    reader->GetTimes(_times);
}

