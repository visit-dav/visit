/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#include <avtFieldViewXDBFileFormat.h>
#include <avtDatabaseMetaData.h>

// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

avtFieldViewXDBFileFormat::avtFieldViewXDBFileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
    // INITIALIZE DATA MEMBERS
}

// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

avtFieldViewXDBFileFormat::~avtFieldViewXDBFileFormat()
{
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

int
avtFieldViewXDBFileFormat::GetNTimesteps(void)
{
    return 1;
}


// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

void
avtFieldViewXDBFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

void
avtFieldViewXDBFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    // Do nothing since we don't want to read XDB's.
}


// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

vtkDataSet *
avtFieldViewXDBFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    // Do nothing since we don't want to read XDB's.
    return NULL;
}


// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

vtkDataArray *
avtFieldViewXDBFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    // Do nothing since we don't want to read XDB's.
    return NULL;
}


// ****************************************************************************
//  Method: avtFieldViewXDBFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

vtkDataArray *
avtFieldViewXDBFileFormat::GetVectorVar(int timestate, int domain,const char *varname)
{
    // Do nothing since we don't want to read XDB's.
    return NULL;
}

