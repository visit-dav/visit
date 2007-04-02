#include <PP_Z_MTSD_FileFormat.h>
#include <PP_ZFileReader.h>
#include <VisItException.h>
#include <avtMTSDFileFormatInterface.h>
#include <DebugStream.h>
#include <vectortypes.h>
#include <avtDatabaseMetaData.h>

// ****************************************************************************
// Class: PP_Z_MTSD_FileFormatInterface
//
// Purpose:
//   Custom file format interface that allows the PP_Z MTSD file format to
//   free up its resources as it sees fit.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 1 23:28:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

class PP_Z_MTSD_FileFormatInterface : public avtMTSDFileFormatInterface
{
public:
    PP_Z_MTSD_FileFormatInterface(avtMTSDFileFormat **fileFormats,
        int nFileFormats) :
        avtMTSDFileFormatInterface(fileFormats, nFileFormats)
    {
    }

    virtual ~PP_Z_MTSD_FileFormatInterface()
    {
    }

    virtual void FreeUpResources(int ts, int)
    {
        // This file format interface only ever has 1 "domain". That "domain"
        // is a PP_Z_MTSD_FileFormat, which manages its own set of files so
        // we need to pass the time state on to it and let it free up
        // resources on the time state according to its own rules.
        PP_Z_MTSD_FileFormat *fmt = (PP_Z_MTSD_FileFormat *)domains[0];
        fmt->FreeUpResourcesForTimeStep(ts);
    }
};

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::CreateInterface
//
// Purpose: 
//   Creates a single MTSD file format interface using the files in the list.
//
// Arguments:
//   pdb   : the open PDB file object.
//   list  : The number of files in the list.
//   nList : The number of files in the list.
//
// Returns:    A file format interface or 0 if one was not created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:36:05 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 1 23:29:39 PST 2004
//   I made it use the PP_Z_MTSD_FileFormatInterface.
//
// ****************************************************************************

avtFileFormatInterface *
PP_Z_MTSD_FileFormat::CreateInterface(PDBFileObject *pdb,
    const char *const *list, int nList)
{
    avtFileFormatInterface *inter = 0;

    // Create a PPZ file that uses the pdb file but does not own it.
    PP_Z_MTSD_FileFormat *ff = new PP_Z_MTSD_FileFormat(pdb, list, nList);

    // If the file format is a PPZ file then create a file format interface.
    if(ff->Identify())
    {
        TRY
        {
            //
            // Try to create a file format interface compatible with the 
            // PPZ MTSD file format.
            //
            avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[1];
            ffl[0] = ff;
            inter = new PP_Z_MTSD_FileFormatInterface(ffl, 1);
        }
        CATCH(VisItException)
        {
            delete inter;
            RETHROW;
        }
        ENDTRY

        //
        // Since at this point, we successfully created a file format interface, we
        // can let the first file format keep the PDB file object.
        //
        ff->SetOwnsPDBFile(true);
    }
    else
        delete ff;

    return inter;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::PP_Z_MTSD_FileFormat
//
// Purpose: 
//   Constructor for the PP_Z_MTSD_FileFormat class.
//
// Arguments:
//   filename : The name of the first file in the list.
//   list     : The filenames that we're trying to use.
//   nList    : The number of files in the list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:37:37 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 1 23:30:06 PST 2004
//   Initialized the lastTimeState and timeFlowsForward members.
//
// ****************************************************************************

PP_Z_MTSD_FileFormat::PP_Z_MTSD_FileFormat(const char *filename,
    const char * const *list, int nList) : avtMTSDFileFormat(list, 1)
{
    int i;
    nReaders = nList;
    readers = new PP_ZFileReader*[nList];
    for(i = 0; i < nList; ++i)
        readers[i] = 0;
    for(i = 0; i < nList; ++i)
        readers[i] = new PP_ZFileReader(list[i]);

    nTotalTimeSteps = -1;
    lastTimeState = -1;
    timeFlowsForward = true;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::PP_Z_MTSD_FileFormat
//
// Purpose: 
//   Constructor for the PP_Z_MTSD_FileFormat class.
//
// Arguments:
//   p     : The open PDB file object.
//   list  : The filenames that we're trying to use.
//   nList : The number of files in the list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:37:37 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 1 23:30:06 PST 2004
//   Initialized the lastTimeState and timeFlowsForward members.
//   
// ****************************************************************************

PP_Z_MTSD_FileFormat::PP_Z_MTSD_FileFormat(PDBFileObject *p,
    const char * const *list, int nList) : avtMTSDFileFormat(list, 1)
{
    int i;
    nReaders = nList;
    readers = new PP_ZFileReader*[nList];
    for(i = 0; i < nList; ++i)
        readers[i] = 0;
    readers[0] = new PP_ZFileReader(p);
    for(i = 1; i < nList; ++i)
        readers[i] = new PP_ZFileReader(list[i]);

    nTotalTimeSteps = -1;
    lastTimeState = -1;
    timeFlowsForward = true;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::~PP_Z_MTSD_FileFormat
//
// Purpose: 
//   Destructor for the PP_Z_MTSD_FileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:39:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PP_Z_MTSD_FileFormat::~PP_Z_MTSD_FileFormat()
{
    for(int i = 0; i < nReaders; ++i)
        delete readers[i];
    delete [] readers;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::Identify
//
// Purpose: 
//   Determines whether or not the file is a MTSD PPZ file.
//
// Returns:    True if the file is the right type; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:39:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
PP_Z_MTSD_FileFormat::Identify()
{
    bool retval = false;

    //
    // Check to see if it's a PPZ file using the reader's Identify method.
    //
    if(readers[0]->Identify())
    {
        // Does it have multiple time states?
        retval = readers[0]->GetNumTimeSteps() > 0;
    }

    return retval;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::SetOwnsPDBFile
//
// Purpose: 
//   Tells the first reader, who by definition gets the PDB file object from
//   the factory, that it owns the PDB file object and can delete it later.
//
// Arguments:
//   val : The new ownership flag.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:40:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::SetOwnsPDBFile(bool val)
{
    readers[0]->SetOwnsPDBFile(val);
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetType
//
// Purpose: 
//   Returns the name of the file format.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:41:30 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Mar 3 15:22:41 PST 2004
//   Made it return Flash.
//
// ****************************************************************************

const char *
PP_Z_MTSD_FileFormat::GetType()
{
    return "Flash File Format";
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetCycles
//
// Purpose: 
//   Adds the cycle numbers to the specified vector of ints.
//
// Arguments:
//   c : The int vector to which the cycles are added.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:51:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Sep 16 16:45:21 PST 2003
//   I made it use multiple readers so we can have a single set of cycles
//   with a bunch of files.
//
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::GetCycles(std::vector<int> &c)
{
    c.clear();

    debug4 << "PP_Z_MTSD_FileFormat::GetCycles: cycles=(";
    for(int i = 0; i < nReaders; ++i)
    {
        int        nts = readers[i]->GetNumTimeSteps();
        const int *cycles = readers[i]->GetCycles();
        if(nts > 0 && cycles != 0)
        {
            for(int j = 0; j < nts; ++j)
            {
                c.push_back(cycles[j]);
                debug4 << ", " << cycles[j];
            }
        }
    }
    debug4 << ")" << endl;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetTimes
//
// Purpose: 
//   Adds the time numbers to the specified vector of doubles.
//
// Arguments:
//   t : The double vector to which the cycles are added.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 16:50:58 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::GetTimes(std::vector<double> &c)
{
    c.clear();

    debug4 << "PP_Z_MTSD_FileFormat::GetTimes: times=(";
    for(int i = 0; i < nReaders; ++i)
    {
        int        nts = readers[i]->GetNumTimeSteps();
        const double *times = readers[i]->GetTimes();
        if(nts > 0 && times != 0)
        {
            for(int j = 0; j < nts; ++j)
            {
                c.push_back(times[j]);
                debug4 << ", " << times[j];
            }
        }
    }
    debug4 << ")" << endl;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetNTimesteps
//
// Purpose: 
//   Returns the number of timesteps in the database.
//
// Returns:    The number of timesteps.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 26 15:52:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PP_Z_MTSD_FileFormat::GetNTimesteps()
{
    if(nTotalTimeSteps == -1)
    {
        //
        // Read the each file to get the total number of time steps.
        //
        nTotalTimeSteps = 0;
        for(int i = 0; i < nReaders; ++i)
        {
            nTotalTimeSteps += readers[i]->GetNumTimeSteps();
        }
    }

    return nTotalTimeSteps;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata using the first reader.
//
// Arguments:
//   md : A pointer to the metadata object to populate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:41:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    readers[0]->PopulateDatabaseMetaData(0, md);

    GetTimeVaryingInformation(md);

    // Since some setup takes place when reading the metadata in the reader, 
    // make sure that all of the readers have a chance to do their setup.
    for(int i = 1; i < nReaders; ++i)
    {
        avtDatabaseMetaData tmp;
        readers[i]->PopulateDatabaseMetaData(-1, &tmp);
    }
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetTimeVaryingInformation
//
// Purpose: 
//   Stores the cycles and times into the metadata object.
//
// Arguments:
//   md : The metadata object to populate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:42:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::GetTimeVaryingInformation(avtDatabaseMetaData *md)
{
    // Put the cycles into the metadata.
    intVector c;
    GetCycles(c);
    md->SetCycles(c);
    md->SetCyclesAreAccurate(true);

    // Put the times into the metadata.
    doubleVector t;
    GetTimes(t);
    md->SetTimes(t);
    md->SetTimesAreAccurate(true);
    if(t.size() > 0)
        md->SetTemporalExtents(t[0], t[t.size()-1]);
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::DetermineTimeFlow
//
// Purpose: 
//   Tries to determine whether we're accessing timesteps in a forward or
//   backward fashion so we can more accurately guess when we need to 
//   clear the cache.
//
// Arguments:
//   ts : The time state that we're accessing.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 2 09:43:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::DetermineTimeFlow(int ts)
{
    if(lastTimeState == -1)
        timeFlowsForward = true;
    else if(lastTimeState != ts)
    {
        if(lastTimeState == 0)
            timeFlowsForward = (ts != nTotalTimeSteps-1);
        else if(lastTimeState == nTotalTimeSteps-1)
            timeFlowsForward = (ts == 0);
        else
            timeFlowsForward = (ts > lastTimeState);
    }

    lastTimeState = ts;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets the auxiliary data.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:43:15 PST 2003
//
// Modifications:
//    Brad Whitlock, Thu Sep 2 09:39:32 PDT 2004
//    Added a call to DetermineTimeFlow.
//
//    Brad Whitlock, Mon Dec 6 16:50:27 PST 2004
//    Added code to set the reader's cache pointer.
//
// ****************************************************************************

void *
PP_Z_MTSD_FileFormat::GetAuxiliaryData(const char *var, int ts, 
    const char *type, void *args, DestructorFunction &df)
{
    void *retval = 0;

    int localTimeState = 0;
    int index = GetReaderIndexAndTimeStep(ts, localTimeState);

    if(index != -1)
    {
        DetermineTimeFlow(ts);
        readers[index]->SetCache(cache);
        retval = readers[index]->GetAuxiliaryData(localTimeState, var, type, args, df);
    }

    return retval;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetMesh
//
// Purpose: 
//   Gets the mesh for the specified time state.
//
// Arguments:
//   ts : The time state for which we're getting the mesh.
//
// Returns:    A mesh object in a vtkDataSet.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:43:39 PST 2003
//
// Modifications:
//    Brad Whitlock, Thu Sep 2 09:39:32 PDT 2004
//    Added a call to DetermineTimeFlow.
// 
//    Brad Whitlock, Mon Dec 6 16:50:27 PST 2004
//    Added code to set the reader's cache pointer.
//
// ****************************************************************************

vtkDataSet *
PP_Z_MTSD_FileFormat::GetMesh(int ts, const char *var)
{
    vtkDataSet *retval = 0;

    int localTimeState = 0;
    int index = GetReaderIndexAndTimeStep(ts, localTimeState);

    if(index != -1)
    {
        DetermineTimeFlow(ts);
        readers[index]->SetCache(cache);
        retval = readers[index]->GetMesh(localTimeState, var);
    }

    return retval;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetVar
//
// Purpose: 
//   Gets the variable for the specified time state.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:44:28 PST 2003
//
// Modifications:
//    Brad Whitlock, Thu Sep 2 09:39:32 PDT 2004
//    Added a call to DetermineTimeFlow.
//
//    Brad Whitlock, Mon Dec 6 16:50:27 PST 2004
//    Added code to set the reader's cache pointer.
//
// ****************************************************************************

vtkDataArray *
PP_Z_MTSD_FileFormat::GetVar(int ts, const char *var)
{
    vtkDataArray *retval = 0;

    int localTimeState = 0;
    int index = GetReaderIndexAndTimeStep(ts, localTimeState);

    if(index != -1)
    {
        DetermineTimeFlow(ts);
        readers[index]->SetCache(cache);
        retval = readers[index]->GetVar(localTimeState, var);
    }

    return retval;
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::FreeUpResourcesForTimeStep
//
// Purpose: 
//   This is a special method that is called by PP_Z_MTSD_FileFormatInterface
//   when the generic database needs to free resources for a specific time
//   step. That mechanism ends up calling this method instead of 
//   the FreeUpResources method since FreeUpResources does not allow us to
//   figure out which time state we need to clear, which translates to a
//   specific file. We don't want to just clear the file because we want to
//   wait for more assurance that we're done with it before clearing its data.
//
// Arguments:
//   ts : The time step to clear.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 2 00:34:09 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PP_Z_MTSD_FileFormat::FreeUpResourcesForTimeStep(int ts)
{
    if(ts == -1)
    {
        debug4 << "PP_Z_MTSD_FileFormat::FreeUpResourcesForTimeStep: "
                  "freeing data for all files." << endl;
        for(int i = 0; i < nReaders; ++i)
            readers[i]->FreeUpResources();
    }
    else
    {
        // Try and predict what the next time step will be based on previous
        // accesses.
        int nextTimeStep;
        if(timeFlowsForward)
        {
            nextTimeStep = ts + 1;
            if(nextTimeStep >= nTotalTimeSteps)
                nextTimeStep = 0;
        }
        else
        {
            nextTimeStep = ts - 1;
            if(nextTimeStep < 0)
                nextTimeStep = nTotalTimeSteps - 1;
        }

        // Determine the file that was used for ts (the time state for which
        // we're freeing up resources) and the "next" time step.
        int tmp;
        int index = GetReaderIndexAndTimeStep(ts, tmp);
        int nextIndex = GetReaderIndexAndTimeStep(nextTimeStep, tmp);

        // If the index and nextIndex are both valid but not equal then
        // we will be transitioning to a new PDB file. Tell the current
        // file to clear out its cached data.
        if(index != -1 && nextIndex != -1 && index != nextIndex)
        {
            debug4 << "PP_Z_MTSD_FileFormat::FreeUpResourcesForTimeStep: "
                      "freeing data for all timesteps in file " << index
                   << " because we think the next timestep will be "
                   << nextTimeStep << ", which is in file " << nextIndex
                   << endl;
            readers[index]->FreeUpResources();
        }
    }
}

// ****************************************************************************
// Method: PP_Z_MTSD_FileFormat::GetReaderIndexAndTimeStep
//
// Purpose: 
//   Translates a global timestate into a reader index and a local timestate
//   for that reader.
//
// Arguments:
//   ts             : The global time state.
//   localTimeState : The local timestate to use with the reader.
//
// Returns:    The reader index or -1 if the time state is invalid.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 17:34:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PP_Z_MTSD_FileFormat::GetReaderIndexAndTimeStep(int ts, int &localTimeState)
{
    int begin = 0;
    for(int i = 0; i < nReaders; ++i)
    {
        int nts = readers[i]->GetNumTimeSteps();

        if(ts >= begin && ts < begin + nts)
        {
            localTimeState = ts - begin;
            return i;
        }
        else
            begin += nts;
    }

    return -1;
}
