// ************************************************************************* //
//                          avtMTSDFileFormatInterface.C                     //
// ************************************************************************* //

#include <avtMTSDFileFormatInterface.h>

#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtIOInformation.h>
#include <avtMTSDFileFormat.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using std::vector;


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface constructor
//
//  Arguments:
//      lst      A list of MTSD file formats.  One for each domain.
//      nLst     The number of elements in list.
//
//  Notes:       The interface owns the file formats and the array they sit on
//               after this call.
//
//  Programmer:  Hank Childs
//  Creation:    October 8, 2001
//
// ****************************************************************************

avtMTSDFileFormatInterface::avtMTSDFileFormatInterface(avtMTSDFileFormat **lst,
                                                       int nLst)
{
    domains  = lst;
    nDomains = nLst;
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface destructor
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

avtMTSDFileFormatInterface::~avtMTSDFileFormatInterface()
{
    if (domains != NULL)
    {
        for (int i = 0 ; i < nDomains ; i++)
        {
            if (domains[i] != NULL)
            {
                delete domains[i];
                domains[i] = NULL;
            }
        }
        delete [] domains;
        domains = NULL;
    }
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::GetMesh
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
//  Creation:   October 8, 2001
//
// ****************************************************************************

vtkDataSet *
avtMTSDFileFormatInterface::GetMesh(int ts, int dom, const char *mesh)
{
    if (dom < 0 || dom >= nDomains)
    {
        EXCEPTION2(BadIndexException, dom, nDomains);
    }

    return domains[dom]->GetMesh(ts, mesh);
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::GetVar
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
//  Creation:   October 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

vtkDataArray *
avtMTSDFileFormatInterface::GetVar(int ts, int dom, const char *var)
{
    if (dom < 0 || dom >= nDomains)
    {
        EXCEPTION2(BadIndexException, dom, nDomains);
    }

    return domains[dom]->GetVar(ts, var);
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::GetVectorVar
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
//  Creation:   October 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

vtkDataArray *
avtMTSDFileFormatInterface::GetVectorVar(int ts, int dom, const char *var)
{
    if (dom < 0 || dom >= nDomains)
    {
        EXCEPTION2(BadIndexException, dom, nDomains);
    }

    return domains[dom]->GetVectorVar(ts, var);
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::GetAuxiliaryData
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
//  Creation:   October 8, 2001
//
// ****************************************************************************

void *
avtMTSDFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
                          const char *type, void *args, DestructorFunction &df)
{
    if (dom == -1)
    {
        debug5 << "Auxiliary data was requested of multiple timestep, "
               << "single domain file format.  Since the data was requested "
               << "for all domains, returning NULL." << endl;
        return NULL;
    }

    if (dom < 0 || dom >= nDomains)
    {
        EXCEPTION2(BadIndexException, dom, nDomains);
    }

    return domains[dom]->GetAuxiliaryData(var, ts, type, args, df);
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::GetFilename
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
avtMTSDFileFormatInterface::GetFilename(int)
{
    return domains[0]->GetFilename();
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::SetDatabaseMetaData
//
//  Purpose:
//      Gets the database meta-data.  This is a tricky issue because we don't
//      want to read in the table of contents for every file.  Just read it
//      in for the first one and try to add some time step information.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Mar  6 10:19:31 PST 2002
//    Do not honor extents from file formats if we know we have multiple
//    blocks.
//
//    Hank Childs, Mon Mar 11 09:42:39 PST 2002
//    Indicate our confidence in the cycle numbers.
//
//    Brad Whitlock, Wed May 14 09:43:16 PDT 2003
//    Added int argument to conform to new method prototype.
//
//    Brad Whitlock, Mon Oct 13 13:46:43 PST 2003
//    Added code to make sure that the times are also populated.
//
// ****************************************************************************

void
avtMTSDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md, int)
{
    //
    // Have a _domain timestep populate what variables, meshes, materials, etc
    // we can deal with for the whole database.  This is bad, but the
    // alternative (to read in all the files) is unattractive and we may not
    // know what to do with a variable that is in some domains and not others
    // anyways.
    //
    domains[0]->SetDatabaseMetaData(md);

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(domains[0]->GetNTimesteps());

    //
    // We are going to try and guess at the naming convention.  If we ever get
    // two consecutive domains that are not in increasing order, assume we
    // are guessing incorrectly and give up.
    //
    vector<int> cycles;
    domains[0]->GetCycles(cycles);
    bool guessLooksGood = true;
    int i;
    for (i = 0 ; i < nDomains ; i++)
    {
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
        for (int j = 0 ; j < nDomains ; j++)
        {
            cycles.push_back(j);
        }
        md->SetCycles(cycles);
    }
    md->SetCyclesAreAccurate(guessLooksGood);

    //
    // Get the times for the file format and make aure that they are in
    // ascending order or assume that they are not valid.
    //
    vector<double> times;
    domains[0]->GetTimes(times);
    guessLooksGood = true;
    for (i = 0 ; i < times.size() ; i++)
    {
        if (i != 0)
        {
            if (times[i] <= times[i-1])
            {
                guessLooksGood = false;
                break;
            }
        }
    }
    if (guessLooksGood)
    {
        md->SetTimes(times);
    }
    else
    {
        int nTimes = times.size();
        times.clear();
        for (int j = 0 ; j < nTimes ; j++)
            times.push_back(double(j));
        md->SetTimes(times);
    }
    md->SetTimesAreAccurate(guessLooksGood);
    if(times.size() > 0)
        md->SetTemporalExtents(times[0], times[times.size() - 1]);

    //
    // Each one of these domains thinks that it only has one domain.  Overwrite
    // that with the true number of domains.
    //
    int nm = md->GetNumMeshes();
    for (int j = 0 ; j < nm ; j++)
    {
        md->SetBlocksForMesh(j, nDomains);
    }

    if (nDomains > 1)
    {
        md->UnsetExtents();
    }
}


// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::FreeUpResources
//
//  Purpose:
//      Frees up the resources for the file (memory, file descriptors).
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
// ****************************************************************************

void
avtMTSDFileFormatInterface::FreeUpResources(int ts, int)
{
    if (ts == -1)
    {
        for (int i = 0 ; i < nDomains ; i++)
        {
            domains[i]->FreeUpResources();
        }
    }
}

// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::ActivateTimestep
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
avtMTSDFileFormatInterface::ActivateTimestep(int ts)
{
    for (int i = 0 ; i < nDomains ; i++)
    {
        domains[i]->ActivateTimestep(ts);
    }
}
