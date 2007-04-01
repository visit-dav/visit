// ************************************************************************* //
//                          avtSTMDFileFormatInterface.C                     //
// ************************************************************************* //

#include <avtSTMDFileFormatInterface.h>

#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtIOInformation.h>
#include <avtSTMDFileFormat.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using std::vector;


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface constructor
//
//  Arguments:
//      lst      A list of STMD file formats.  One for each timestep.
//      nLst     The number of elements in list.
//
//  Notes:       The interface owns the file formats and the array they sit on
//               after this call.
//
//  Programmer:  Hank Childs
//  Creation:    February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:19:34 PDT 2001
//    Tell each format what its timestep is.
//
//    Hank Childs, Thu Oct 18 14:21:20 PDT 2001
//    Tell each file format how many timesteps there are.
//
// ****************************************************************************

avtSTMDFileFormatInterface::avtSTMDFileFormatInterface(avtSTMDFileFormat **lst,
                                                       int nLst)
{
    timesteps  = lst;
    nTimesteps = nLst;
    for (int i = 0 ; i < nLst ; i++)
    {
        timesteps[i]->SetTimestep(i, nTimesteps);
    }
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface destructor
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
// ****************************************************************************

avtSTMDFileFormatInterface::~avtSTMDFileFormatInterface()
{
    if (timesteps != NULL)
    {
        for (int i = 0 ; i < nTimesteps ; i++)
        {
            if (timesteps[i] != NULL)
            {
                delete timesteps[i];
                timesteps[i] = NULL;
            }
        }
        delete [] timesteps;
        timesteps = NULL;
    }
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::GetMesh
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
//  Creation:   February 22, 2001
//
//  Modifications:
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//.
// ****************************************************************************

vtkDataSet *
avtSTMDFileFormatInterface::GetMesh(int ts, int dom, const char *mesh)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    return timesteps[ts]->GetMesh(dom, mesh);
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::GetVar
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
//  Creation:   February 22, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

vtkDataArray *
avtSTMDFileFormatInterface::GetVar(int ts, int dom, const char *var)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    return timesteps[ts]->GetVar(dom, var);
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::GetVectorVar
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
//  Creation:   March 19, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

vtkDataArray *
avtSTMDFileFormatInterface::GetVectorVar(int ts, int dom, const char *var)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    return timesteps[ts]->GetVectorVar(dom, var);
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::GetAuxiliaryData
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
//  Craetion:   February 22, 2001
//
//  Modifications:
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

void *
avtSTMDFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
                                             const char *type, void *args,
                                             DestructorFunction &df)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    return timesteps[ts]->GetAuxiliaryData(var, dom, type, args, df);
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::GetFilename
//
//  Purpose:
//      Gets the filename for a particular timestep.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

const char *
avtSTMDFileFormatInterface::GetFilename(int ts)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    return timesteps[ts]->GetFilename();
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::SetDatabaseMetaData
//
//  Purpose:
//      Gets the database meta-data.  This is a tricky issue because we don't
//      want to read in the table of contents for every file.  Just read it
//      in for the first one and try to add some time step information.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 09:42:39 PST 2002
//    Indicate our confidence in the cycle numbers.
//
//    Brad Whitlock, Wed May 14 09:31:21 PDT 2003
//    Added timeState so we can get the metadata for a certain time state.
//
//    Hank Childs, Tue Jul 29 21:39:39 PDT 2003
//    Do not declare the cycle number accurate -- since we are still guessing.
//
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

void
avtSTMDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    //
    // Throw an exception if an invalid time state was requested.
    //
    if(timeState < 0 || timeState >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTimesteps);
    }

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(nTimesteps);

    //
    // We are going to try and guess at the naming convention.  If we ever get
    // two consecutive timesteps that are not in increasing order, assume we
    // are guessing incorrectly and give up.
    //
    vector<int> cycles;
    bool guessLooksGood = true;
    int i;
    for (i = 0 ; i < nTimesteps ; i++)
    {
        int c = timesteps[i]->GetCycle();
        cycles.push_back(c);
        if (i != 0)
        {
            if (cycles[i] <= cycles[i-1])
            {
                guessLooksGood = false;
                break;
            }
        }
    }
    if (guessLooksGood)
    {
        md->SetCycles(cycles);
    }
    else
    {
        cycles.clear();
        for (int j = 0 ; j < nTimesteps ; j++)
        {
            cycles.push_back(j);
        }
        md->SetCycles(cycles);
    }
    md->SetCyclesAreAccurate(false);

    //
    // Have a _single_ timestep populate what variables, meshes, materials, etc
    // we can deal with for the whole database.  This is bad, but the
    // alternative (to read in all the files) is unattractive and we may not
    // know what to do with a variable that is in some timesteps and not others
    // anyways.
    //
    timesteps[timeState]->SetDatabaseMetaData(md);
    for (i = 0; i < nTimesteps ; i++)
    {
        if(i != timeState)
            timesteps[i]->RegisterDatabaseMetaData(md);
    }
}


// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::FreeUpResources
//
//  Purpose:
//      Frees up the resources for the file (memory, file descriptors).
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Dec  5 11:08:54 PST 2003
//    Try to handle bad timesteps, since the viewer's state is out-of-whack
//    and a fix cannot get in by 1.2.5.
//
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

void
avtSTMDFileFormatInterface::FreeUpResources(int ts, int)
{
    if (ts == -1)
    {
        for (int i = 0 ; i < nTimesteps ; i++)
        {
            timesteps[i]->FreeUpResources();
        }
    }
    else
    {
        if (ts < 0 || ts >= nTimesteps)
        {
            EXCEPTION2(BadIndexException, ts, nTimesteps);
        }
        timesteps[ts]->FreeUpResources();
    }
}

// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::ActivateTimestep
//
//  Purpose: Notify the format of our intention to read data for a given
//  timestep. This gives the format an opportunity to do whatever 
//  parallel collective work it might need to for the given timestep
//
//  Programmer: Mark C. Miller 
//  Creation:   February 23, 2004 
//
//  Modifications:
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

void
avtSTMDFileFormatInterface::ActivateTimestep(int ts)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }
    timesteps[ts]->ActivateTimestep();
}

// ****************************************************************************
//  Method: avtSTMDFileFormatInterface::PopulateIOInformation
//
//  Purpose: Populate information regarding domain's assignment to files 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 16, 2004 
//
//  Modifications:
//    Brad Whitlock, Tue May 4 13:47:45 PST 2004
//    Reenabled exception.
//
// ****************************************************************************

void
avtSTMDFileFormatInterface::PopulateIOInformation(int ts, avtIOInformation& ioInfo)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }
    timesteps[ts]->PopulateIOInformation(ioInfo);
}
