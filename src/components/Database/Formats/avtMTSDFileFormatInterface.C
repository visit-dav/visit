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
//  Modifications:
//
//    Hank Childs, Mon Aug 16 16:22:56 PDT 2004
//    Tell each file format what its domain is.
//
// ****************************************************************************

avtMTSDFileFormatInterface::avtMTSDFileFormatInterface(avtMTSDFileFormat **lst,
                                                       int nLst)
{
    domains  = lst;
    nDomains = nLst;

    for (int i = 0 ; i < nDomains ; i++)
    {
        if (lst[i] != NULL)
        {
            lst[i]->SetDomain(i);
        }
    }
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
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Revamped the code to deal with getting cycles/times to make it more
//    consistent with other plugin types. Added anonymous bool for force read
//    all cycles/times to satisfy interface. Here, that bool can be ignored
//    because issues with opening many files, etc. to obtain that information
//    are not VisIt's concern. It is the plugin developers. Even if VisIt
//    should be concerned with it, there isn't much VisIt can do except pass
//    the request down to the plugin
//
// ****************************************************************************

void
avtMTSDFileFormatInterface::SetDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState, bool)
{
    int i, j;

    //
    // Throw an exception if an invalid time state was requested.
    //
    int nTimesteps = domains[0]->GetNTimesteps();
    if (timeState < 0 || timeState >= nTimesteps)
    {
        EXCEPTION2(BadIndexException, timeState, nTimesteps);
    }

    //
    // We know for sure that the number of states is the number of timesteps.
    //
    md->SetNumStates(nTimesteps);

    //
    // Have a _domain timestep populate what variables, meshes, materials, etc
    // we can deal with for the whole database.  This is bad, but the
    // alternative (to read in all the files) is unattractive and we may not
    // know what to do with a variable that is in some domains and not others
    // anyways. This may wind up setting cycles/times, too.
    //
    domains[0]->SetDatabaseMetaData(md, timeState);

    //
    // Note: In an MTXX format, a single file has multiple time steps in it
    // So, we don't have the same kinds of semantics we do with STXX databases
    // in, for example, trying to guess cycle numbers from file names
    //
    if (md->AreAllCyclesAccurateAndValid(nTimesteps) != true)
    {
        vector<int> cycles;
        domains[0]->FormatGetCycles(cycles);
        bool cyclesLookGood = true;
        for (i = 0; i < cycles.size(); i++)
        {
            if ((i != 0) && (cycles[i] <= cycles[i-1]))
            {
                cyclesLookGood = false;
                break;
            }
        }
        if (cycles.size() != nTimesteps)
            cyclesLookGood = false;
        if (cyclesLookGood == false)
        {
            cycles.clear();
            for (i = 0; i < nTimesteps; i++)
            {
                int c = domains[0]->FormatGetCycle(i);

                cycles.push_back(c);

                if ((c == -INT_MAX) || ((i != 0) && (cycles[i] <= cycles[i-1])))
                {
                    cyclesLookGood = false;
                    break;
                }
            }
        }

        //
        // Ok, now put cycles into the metadata
        //
        if (cyclesLookGood)
        {
            md->SetCycles(cycles);
            md->SetCyclesAreAccurate(true);
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
        // Set the times in the metadata.
        vector<double> times;
        domains[0]->FormatGetTimes(times);
        bool timesLookGood = true;
        for (i = 0; i < times.size(); i++)
        {
            if ((i != 0) && (times[i] <= times[i-1]))
            {
                timesLookGood = false;
                break;
            }
        }
        if (times.size() != nTimesteps)
            timesLookGood = false;
        if (timesLookGood == false)
        {
            times.clear();
            for (i = 0; i < nTimesteps; i++)
            {
                double t = domains[0]->FormatGetTime(i);

                times.push_back(t);

                if ((t == -DBL_MAX) || ((i != 0) && (times[i] <= times[i-1])))
                {
                    timesLookGood = false;
                    break;
                }
            }
        }

        //
        // Ok, now put times into the metadata
        //
        if (timesLookGood)
        {
            md->SetTimes(times);
            md->SetTimesAreAccurate(true);
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

// ****************************************************************************
//  Method: avtMTSDFileFormatInterface::PopulateIOInformation
//
//  Purpose: Populate information regarding domain's assignment to files 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 16, 2004 
//
// ****************************************************************************
void
avtMTSDFileFormatInterface::PopulateIOInformation(int ts, avtIOInformation &ioInfo)
{
    for (int i = 0 ; i < nDomains ; i++)
    {
        domains[i]->PopulateIOInformation(ts, ioInfo);
    }
}
