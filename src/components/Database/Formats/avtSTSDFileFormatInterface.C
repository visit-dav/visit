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
        EXCEPTION2(BadIndexException, 2, nTimesteps);
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
        EXCEPTION2(BadIndexException, 2, nTimesteps);
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
        EXCEPTION2(BadIndexException, 2, nTimesteps);
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
        EXCEPTION2(BadIndexException, 2, nTimesteps);
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
// ****************************************************************************

void
avtSTSDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
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
    bool formatReturnsValidCycles = false;
    int i, j;
    for (i = 0 ; i < nTimesteps ; i++)
    {
        cycles.push_back(timesteps[i][0]->GetCycle());
        if (i != 0)
        {
            if (cycles[i] <= cycles[i-1])
            {
                guessLooksGood = false;
                break;
            }
        }
        else
        {
            // Some formats can return valid cycles. Most formats return
            // guesses but for those formats that return valid cycles, we
            // don't want to throw them away.
            formatReturnsValidCycles = timesteps[i][0]->ReturnsValidCycle();
        }
    }
    if (guessLooksGood)
    {
        md->SetCycles(cycles);
    }
    else
    {
        cycles.clear();
        for (j = 0 ; j < nTimesteps ; j++)
        {
            cycles.push_back(j);
        }
        md->SetCycles(cycles);
    }
    md->SetCyclesAreAccurate(guessLooksGood && formatReturnsValidCycles);

    //
    // Set the times in the metadata.
    //
    if(nTimesteps > 0)
    {
        vector<double> times;
        guessLooksGood = true;
        bool formatReturnsValidTimes = false;
        for (i = 0 ; i < nTimesteps ; i++)
        {
            times.push_back(timesteps[i][0]->GetTime());

            if (i != 0)
            {
                if (times[i] <= times[i-1])
                {
                    guessLooksGood = false;
                    break;
                }
            }
            else
            {
                // Some formats can return valid times. Most formats return
                // guesses but for those formats that return valid cycles, we
                // don't want to throw them away.
                formatReturnsValidTimes = timesteps[i][0]->ReturnsValidTime();
            }
        }

        if (guessLooksGood && formatReturnsValidTimes)
        {
            md->SetTimes(times);
            md->SetTimesAreAccurate(true);
            if(times.size() > 0)
                md->SetTemporalExtents(times[0], times[times.size() - 1]);
        }
    }

    //
    // Have a _single_ timestep populate what variables, meshes, materials, etc
    // we can deal with for the whole database.  This is bad, but the
    // alternative (to read in all the files) is unattractive and we may not
    // know what to do with a variable that is in some timesteps and not others
    // anyways.
    //
    timesteps[timeState][0]->SetDatabaseMetaData(md);
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
            EXCEPTION2(BadIndexException, 2, nTimesteps);
        }
        for (int j = 0 ; j < nBlocks ; j++)
        {
            timesteps[ts][j]->FreeUpResources();
        }
    }
}


