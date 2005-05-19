// ************************************************************************* //
//                          avtSTSDFileFormatInterface.C                     //
// ************************************************************************* //

#include <avtSTSDFileFormatInterface.h>

#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtIOInformation.h>
#include <avtSTSDFileFormat.h>

#include <vtkDataArray.h>

#include <BadIndexException.h>

using std::vector;


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface constructor
//
//  Arguments:
//      lst      A list of STSD file formats.  One for each timestep, block.
//      nT       The number of timesteps in list.
//      nB       The number of blocks in the list.
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
//    Tell each format what its timestep, domain is.
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
// ****************************************************************************

avtSTSDFileFormatInterface::avtSTSDFileFormatInterface(
                                      avtSTSDFileFormat ***lst, int nT, int nB)
{
    timesteps  = lst;
    nTimesteps = nT;
    nBlocks    = nB;
    for (int i = 0 ; i < nT ; i++)
    {
        for (int j = 0 ; j < nB ; j++)
        {
            timesteps[i][j]->SetTimestep(i);
            timesteps[i][j]->SetDomain(j);
        }
    }
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface destructor
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
// ****************************************************************************

avtSTSDFileFormatInterface::~avtSTSDFileFormatInterface()
{
    if (timesteps != NULL)
    {
        for (int i = 0 ; i < nTimesteps ; i++)
        {
            if (timesteps[i] != NULL)
            {
                for (int j = 0 ; j < nBlocks ; j++)
                {
                    if (timesteps[i][j] != NULL)
                    {
                        delete timesteps[i][j];
                    }
                }
                delete [] timesteps[i];
            }
        }
        delete [] timesteps;
        timesteps = NULL;
    }
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::GetMesh
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
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
// ****************************************************************************

vtkDataSet *
avtSTSDFileFormatInterface::GetMesh(int ts, int dom, const char *mesh)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    if (dom < 0 || dom >= nBlocks)
    {
        EXCEPTION2(BadIndexException, dom, nBlocks);
    }

    return timesteps[ts][dom]->GetMesh(mesh);
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::GetVar
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
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
//    Kathleen Bonnell, Mon Mar 18 17:22:30 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

vtkDataArray *
avtSTSDFileFormatInterface::GetVar(int ts, int dom, const char *var)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    if (dom < 0 || dom >= nBlocks)
    {
        EXCEPTION2(BadIndexException, dom, nBlocks);
    }

    return timesteps[ts][dom]->GetVar(var);
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::GetVectorVar
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
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
//    Kathleen Bonnell, Mon Mar 18 17:22:30 PST 2002  
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

vtkDataArray *
avtSTSDFileFormatInterface::GetVectorVar(int ts, int dom, const char *var)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    if (dom < 0 || dom >= nBlocks)
    {
        EXCEPTION2(BadIndexException, dom, nBlocks);
    }

    return timesteps[ts][dom]->GetVectorVar(var);
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data from the file format.
//
//  Arguments:
//      var     The variable.
//      ts      The time step.
//      dom     The domain.
//      type    The type of auxiliary data.
//      args    An additional arguments.
//
//  Returns:    A void * of the auxiliary data.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jun 18 10:14:18 PDT 2001
//    Account for domain == -1.
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
// ****************************************************************************

void *
avtSTSDFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
                          const char *type, void *args, DestructorFunction &df)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }

    //
    // dom == -1 is used to indicate something that is valid over all domains.
    // Since there is only a single domain, all domains can be just the one
    // domain.
    //
    if (dom == -1)
    {
        dom = 0;
    }

    if (dom < 0 || dom >= nBlocks)
    {
        EXCEPTION2(BadIndexException, dom, nBlocks);
    }

    return timesteps[ts][dom]->GetAuxiliaryData(var, type, args, df);
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::GetFilename
//
//  Purpose:
//      Gets the filename for a particular timestep.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

const char *
avtSTSDFileFormatInterface::GetFilename(int ts)
{
    return timesteps[ts][0]->GetFilename();
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::SetDatabaseMetaData
//
//  Purpose:
//      Gets the database meta-data.  This is a tricky issue because we don't
//      want to read in the table of contents for every file.  Just read it
//      in for the first one and try to add some time step information.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Reflect that we now have multiple blocks in our pointer list.
//
//    Hank Childs, Wed Mar  6 10:19:31 PST 2002
//    Do not honor extents from file formats if we know we have multiple
//    blocks.
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
//    Brad Whitlock, Mon Oct 13 14:55:11 PST 2003
//    Added code that lets the format determine whether or not the cycle is
//    accurate. I also added support for setting the times in a similar way.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool to forceReadAllCyclesTimes. Added logic to correctly
//    deal with reading cycles/times from formats. Unified it so it behaves
//    consistently with other format types
// ****************************************************************************

void
avtSTSDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState, bool forceReadAllCyclesTimes)
{
    int i,j;

    //
    // Throw an exception if an invalid time state was requested.
    //
    if (timeState < 0 || timeState >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTimesteps);
    }

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(nTimesteps);

    //
    // Let the format plugin fill in whatever in can in avtDatabaseMetaData,
    // first
    //
    timesteps[timeState][0]->SetDatabaseMetaData(md);

    //
    // We are going to try and guess at the naming convention.  If we ever get
    // two consecutive timesteps that are not in increasing order, assume we
    // are guessing incorrectly and give up.
    //
    if (md->AreAllCyclesAccurateAndValid(nTimesteps) != true ||
        md->AreAllTimesAccurateAndValid(nTimesteps) != true)
    {
        //
        // Probe the format to see if we're using the default implementation of
        // FormatGetCycleFromFilename or one that the plugin has overridden. The default
        // implementation which will return -XXX_MAX+1 cannot be trusted.
        //
        bool canGetGoodCycleFromFilename =
                 timesteps[0][0]->FormatGetCycleFromFilename("") != -INT_MAX+1;
        bool canGetGoodTimeFromFilename =
                 timesteps[0][0]->FormatGetTimeFromFilename("") != -DBL_MAX+1;

        //
        // We are going to obtain cycle information from Filenames or from
        // the format itself depending on whether forceReadAllCyclesTimes is true.
        // If we ever get two consecutive timesteps that are not in increasing order,
        // either from the Filename or from the format itself, or if we get "invalid"
        // values back from the plugin, we give up.
        //
        bool cyclesLookGood = true;
        bool timesLookGood = true;
        bool fallBackToCyclesFromFilename = false;
        bool fallBackToTimesFromFilename = false;
        vector<int> cycles;
        vector<double> times;
        vector<bool> cycleIsAccurate;
        vector<bool> timeIsAccurate;
        for (i = 0 ; i < nTimesteps; i++)
        {

            if (md->IsCycleAccurate(i) != true && cyclesLookGood)
            {
                int c = -INT_MAX;

                if (forceReadAllCyclesTimes && !fallBackToCyclesFromFilename &&
                    !canGetGoodCycleFromFilename)
                {
                    c = timesteps[i][0]->FormatGetCycle();

                    // If we get back -INT_MAX from FormatGetCycle, this indicates the
                    // format was unable to return a valid cycle. So, we fall back
                    // to using the filenames.
                    if (c == -INT_MAX)
                    {
                        fallBackToCyclesFromFilename = true;
                        c = timesteps[i][0]->FormatGetCycleFromFilename(timesteps[i][0]->GetFilename());
                        cycleIsAccurate.push_back(false);
                    }
                    else
                    {
                        cycleIsAccurate.push_back(true);
                    }
                }
                else
                {
                    c = timesteps[i][0]->FormatGetCycleFromFilename(timesteps[i][0]->GetFilename());
                    cycleIsAccurate.push_back(true);
                }

                cycles.push_back(c);

                if ((c == -INT_MAX) || ((i != 0) && (cycles[i] <= cycles[i-1])))
                    cyclesLookGood = false;
            }
            else
            {
                if (cyclesLookGood)
                {
                    cycles.push_back(md->GetCycles()[i]);
                    cycleIsAccurate.push_back(true);
                }
            }

            if (md->IsTimeAccurate(i) != true && timesLookGood)
            {
                double t = -DBL_MAX;

                if (forceReadAllCyclesTimes && !fallBackToTimesFromFilename &&
                    !canGetGoodTimeFromFilename)
                {
                    t = timesteps[i][0]->FormatGetTime();

                    // If we get back -DBL_MAX from FormatGetTime, this indicates the
                    // format was unable to return a valid time. So, we fall back
                    // to using the filenames.
                    if (t == -DBL_MAX)
                    {
                        fallBackToTimesFromFilename = true;
                        t = timesteps[i][0]->FormatGetTimeFromFilename(timesteps[i][0]->GetFilename());
                        timeIsAccurate.push_back(false);
                    }
                    else
                    {
                        timeIsAccurate.push_back(true);
                    }
                }
                else
                {
                    t = timesteps[i][0]->FormatGetTimeFromFilename(timesteps[i][0]->GetFilename());
                    timeIsAccurate.push_back(true);
                }

                times.push_back(t);

                if ((t == -DBL_MAX) || ((i != 0) && (times[i] <= times[i-1])))
                    timesLookGood = false;
            }
            else
            {
                if (timesLookGood)
                {
                    times.push_back(md->GetTimes()[i]);
                    timeIsAccurate.push_back(true);
                }
            }

            if (cyclesLookGood == false && timesLookGood == false)
                break;
        }

        //
        // Ok, now put cycles/times into the metadata
        //
        if (md->AreAllCyclesAccurateAndValid(nTimesteps) != true)
        {
            if (cyclesLookGood)
            {
                md->SetCycles(cycles);
                for (i = 0 ; i < nTimesteps; i++)
                    md->SetCycleIsAccurate(cycleIsAccurate[i],i);
            }
            else
            {
                cycles.clear();
                for (j = 0 ; j < nTimesteps ; j++)
                {
                    cycles.push_back(j);
                }
                md->SetCycles(cycles);
                md->SetCyclesAreAccurate(false);
            }
        }

        if (md->AreAllTimesAccurateAndValid(nTimesteps) != true)
        {
            if (timesLookGood)
            {
                md->SetTimes(times);
                for (i = 0 ; i < nTimesteps; i++)
                    md->SetTimeIsAccurate(timeIsAccurate[i],i);
                md->SetTemporalExtents(times[0], times[times.size() - 1]);
            }
            else
            {
                times.clear();
                for (j = 0 ; j < nTimesteps ; j++)
                {
                    times.push_back((double)j);
                }
                md->SetTimes(times);
                md->SetTimesAreAccurate(false);
            }
        }
    }

    //
    // Have a _single_ timestep populate what variables, meshes, materials, etc
    // we can deal with for the whole database.  This is bad, but the
    // alternative (to read in all the files) is unattractive and we may not
    // know what to do with a variable that is in some timesteps and not others
    // anyways.
    //
    for (i = 0 ; i < nTimesteps ; i++)
    {
        for (j = 0 ; j < nBlocks ; j++)
        {
            if (i == timeState && j == 0)
            {
                continue;
            }
            timesteps[i][j]->RegisterDatabaseMetaData(md);
        }
    }

    //
    // Each one of these domains thinks that it only has one domain.  Overwrite
    // that with the true number of domains.
    //
    int nm = md->GetNumMeshes();
    for (j = 0 ; j < nm ; j++)
    {
        md->SetBlocksForMesh(j, nBlocks);
    }

    if (nBlocks > 1)
    {
        md->UnsetExtents();
    }
}


// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::FreeUpResources
//
//  Purpose:
//      Frees up the resources for the file (memory, file descriptors).
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb 14 10:08:01 PST 2002
//    Add better support for multiple blocks.
//
// ****************************************************************************

void
avtSTSDFileFormatInterface::FreeUpResources(int ts, int)
{
    if (ts == -1)
    {
        for (int i = 0 ; i < nTimesteps ; i++)
        {
            for (int j = 0 ; j < nBlocks ; j++)
            {
                timesteps[i][j]->FreeUpResources();
            }
        }
    }
    else
    {
        if (ts < 0 || ts >= nTimesteps)
        {
            EXCEPTION2(BadIndexException, ts, nTimesteps);
        }
        for (int j = 0 ; j < nBlocks ; j++)
        {
            timesteps[ts][j]->FreeUpResources();
        }
    }
}

// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::ActivateTimestep
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
avtSTSDFileFormatInterface::ActivateTimestep(int ts)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }
    for (int j = 0 ; j < nBlocks ; j++)
    {
        timesteps[ts][j]->ActivateTimestep();
    }
}

// ****************************************************************************
//  Method: avtSTSDFileFormatInterface::PopulateIOInformation
//
//  Purpose: Populate information regarding domain's assignment to files 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 16, 2004 
//
// ****************************************************************************

void
avtSTSDFileFormatInterface::PopulateIOInformation(int ts, avtIOInformation& ioInfo)
{
    if (ts < 0 || ts >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, ts, nTimesteps);
    }
    for (int j = 0 ; j < nBlocks ; j++)
    {
        timesteps[ts][j]->PopulateIOInformation(ioInfo);
    }
}
