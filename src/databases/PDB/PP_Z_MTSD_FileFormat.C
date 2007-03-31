#include <PP_Z_MTSD_FileFormat.h>
#include <PP_ZFileReader.h>
#include <VisItException.h>
#include <avtMTSDFileFormatInterface.h>
#include <DebugStream.h>
#include <vectortypes.h>
#include <avtDatabaseMetaData.h>

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
            inter = new avtMTSDFileFormatInterface(ffl, 1);
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
//   
// ****************************************************************************

const char *
PP_Z_MTSD_FileFormat::GetType()
{
    return "PP_Z MTSD File Format";
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
    readers[0]->PopulateDatabaseMetaData(md);

    // Since some setup takes place when reading the metadata in the reader, 
    // make sure that all of the readers have a chance to do their setup.
    for(int i = 1; i < nReaders; ++i)
    {
        avtDatabaseMetaData tmp;
        readers[i]->PopulateDatabaseMetaData(&tmp);
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
PP_Z_MTSD_FileFormat::GetTimeVaryingInformation(int ts, avtDatabaseMetaData *md)
{
    // Put the cycles into the metadata.
    intVector c;
    GetCycles(c);
    if(md->GetCycles().size() < c.size())
    {
        // Set all of the cycles at once.
        md->SetCycles(c);
        md->SetCyclesAreAccurate(true);
    }

    // Put the times into the metadata.
    doubleVector t;
    GetTimes(t);
    if(md->GetTimes().size() < t.size())
    {
        // Set all of the times at once.
        md->SetTimes(t);
        md->SetTimesAreAccurate(true);

        if(t.size() > 0)
            md->SetTemporalExtents(t[0], t[t.size()-1]);
    }
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
        retval = readers[index]->GetAuxiliaryData(localTimeState, var, type, args, df);

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
//   
// ****************************************************************************

vtkDataSet *
PP_Z_MTSD_FileFormat::GetMesh(int ts, const char *var)
{
    vtkDataSet *retval = 0;

    int localTimeState = 0;
    int index = GetReaderIndexAndTimeStep(ts, localTimeState);

    if(index != -1)
        retval = readers[index]->GetMesh(localTimeState, var);

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
//   
// ****************************************************************************

vtkDataArray *
PP_Z_MTSD_FileFormat::GetVar(int ts, const char *var)
{
    vtkDataArray *retval = 0;

    int localTimeState = 0;
    int index = GetReaderIndexAndTimeStep(ts, localTimeState);

    if(index != -1)
        retval = readers[index]->GetVar(localTimeState, var);

    return retval;
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
