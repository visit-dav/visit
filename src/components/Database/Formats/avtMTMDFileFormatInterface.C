// ************************************************************************* //
//                          avtMTMDFileFormatInterface.C                     //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>

#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtIOInformation.h>
#include <avtMTMDFileFormat.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using std::vector;


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface constructor
//
//  Arguments:
//      f        The MTMD file format that we are an interface for.
//
//  Notes:       The interface owns the file format after this call.
//
//  Programmer:  Hank Childs
//  Creation:    April 4, 2003
//
// ****************************************************************************

avtMTMDFileFormatInterface::avtMTMDFileFormatInterface(avtMTMDFileFormat *f)
{
    format = f;
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface destructor
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

avtMTMDFileFormatInterface::~avtMTMDFileFormatInterface()
{
    if (format != NULL)
    {
        delete format;
    }
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetMesh
//
//  Purpose:
//      Gets the mesh from the file format.
//
//  Arguments:
//      ts      The desired time step.
//      dom     The domain.
//      mesh    The name of the mesh.
//
//  Returns:    The vtk mesh.
//
//  Progrmamer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

vtkDataSet *
avtMTMDFileFormatInterface::GetMesh(int ts, int dom, const char *mesh)
{
    return format->GetMesh(ts, dom, mesh);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetVar
//
//  Purpose:
//      Gets the var from the file format.
//
//  Arguments:
//      ts      The desired time step.
//      dom     The domain.
//      var     The name of the var.
//
//  Returns:    The vtk variable (field).
//
//  Progrmamer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormatInterface::GetVar(int ts, int dom, const char *var)
{
    return format->GetVar(ts, dom, var);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetVectorVar
//
//  Purpose:
//      Gets the var from the file format.
//
//  Arguments:
//      ts      The desired time step.
//      dom     The domain.
//      var     The name of the var.
//
//  Returns:    The vtk variable (field).
//
//  Progrmamer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

vtkDataArray *
avtMTMDFileFormatInterface::GetVectorVar(int ts, int dom, const char *var)
{
    return format->GetVectorVar(ts, dom, var);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data from the file format.
//
//  Arguments:
//      var     The variable.
//      ts      The time step.
//      dom     The domain.
//      type    The type of auxiliary data.
//      args    Any additional arguments.
//
//  Returns:    A void * of the auxiliary data.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

void *
avtMTMDFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
                          const char *type, void *args, DestructorFunction &df)
{
    if (dom == -1)
    {
        debug5 << "Auxiliary data was requested of multiple timestep, "
               << "multiple domain file format.  Since the data was requested "
               << "for all domains, returning NULL." << endl;
        return NULL;
    }

    return format->GetAuxiliaryData(var, ts, dom, type, args, df);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::GetFilename
//
//  Purpose:
//      Gets the name of the file we are using.
//
//  Arguments:
//      <unused>   The timestep (does not really apply to this interface).
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

const char *
avtMTMDFileFormatInterface::GetFilename(int)
{
    return format->GetFilename();
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::SetDatabaseMetaData
//
//  Purpose:
//      Gets the database meta-data.  This is a tricky issue because we don't
//      want to read in the table of contents for every file.  Just read it
//      in for the first one and try to add some time step information.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:43:16 PDT 2003
//    Added int argument to conform to new method prototype.
//
//    Brad Whitlock, Mon Oct 13 13:54:06 PST 2003
//    Added code to set the times in the metadata.
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
    int)
{
    format->SetDatabaseMetaData(md);

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(format->GetNTimesteps());

    //
    // We are going to try and guess at the naming convention.  If we ever get
    // two consecutive domains that are not in increasing order, assume we
    // are guessing incorrectly and give up.
    //
    vector<int> cycles;
    format->GetCycles(cycles);
    md->SetCycles(cycles);
    md->SetCyclesAreAccurate(true);

    // Set the times in the metadata.
    vector<double> times;
    format->GetTimes(times);
    md->SetTimes(times);
    md->SetTimesAreAccurate(true);
    if(times.size() > 0)
        md->SetTemporalExtents(times[0], times[times.size() - 1]);
}


// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::FreeUpResources
//
//  Purpose:
//      Frees up the resources for the file (memory, file descriptors).
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::FreeUpResources(int, int)
{
    format->FreeUpResources();
}

// ****************************************************************************
//  Method: avtMTMDFileFormatInterface::ActivateTimestep
//
//  Purpose: Notify the format of our intention to read data for a given
//  timestep. This gives the format an opportunity to do whatever 
//  parallel collective work it might need to for the given timestep
//
//  Programmer: Mark C. Miller 
//  Creation:   February 23, 2004 
//
// ****************************************************************************

void
avtMTMDFileFormatInterface::ActivateTimestep(int ts)
{
    format->ActivateTimestep(ts);
}
