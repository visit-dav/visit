#include <avtPDBFileFormat.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>

#include <PDBFactory.h>
#include <PDBReader.h>

#include <BadIndexException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>

#include <DebugStream.h>


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// Begin the file format reader.
///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: avtPDBFileFormat::avtPDBFileFormat
//
// Purpose: 
//   Constructor for the avtPDBFileFormat class.
//
// Arguments:
//   dbFileName : The name of the database file to open.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:36:30 PST 2002
//
// Modifications:
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Do not do so much work in the constructor.
//
//   Brad Whitlock, Mon Jun 30 11:04:17 PDT 2003
//   I added some of the initialization work back in because we have to at
//   least open the file in here or the reader will say that it can read a
//   Silo .pdb file and then fail later when actually trying to read the Silo
//   file and we DON'T want that.
//
//   Brad Whitlock, Thu Aug 7 16:48:29 PST 2003
//   I passed the database cache to the factory.
//
// ****************************************************************************

avtPDBFileFormat::avtPDBFileFormat(const char * const *names, int nNames,
    bool openImmediately) :
    avtMTSDFileFormat(names, nNames), factory()
{
    initializedFactory = false;

    //
    // Get the file. This will try to open the PDB file with various readers
    // and see if any of the readers can identify the format of the PDB file.
    // If no reader can identify the format, we throw an InvalidDBTypeException.
    // We have to throw the exception from here so we can try to open the
    // file with other database formats in avtDatabaseFactory::FileList.
    //
    if(openImmediately)
        factory.Open(filenames[0], cache);
}

// ****************************************************************************
// Method: avtPDBFileFormat::InitializeFactory
//
// Purpose:
//   Has the factory actually open the file.
//
// Arguments:
//   timeState : The time that we're interested in.
//
// Programmer: Hank Childs
// Creation:   April 7, 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:00:59 PDT 2003
//   I added the timeState argument.
//
//   Brad Whitlock, Thu Aug 7 16:48:29 PST 2003
//   I passed the database cache to the factory.
//
// ****************************************************************************

void
avtPDBFileFormat::InitializeFactory(int timeState)
{
    factory.Open(filenames[0], cache);
    factory.GetReader()->GetTimeVaryingInformation(timeState, metadata);
    initializedFactory = true;
}

// ****************************************************************************
// Method: avtPDBFileFormat::~avtPDBFileFormat
//
// Purpose: 
//   Destructor for the avtPDBFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:37:03 PST 2002
//
// Modifications:
//   
// ****************************************************************************

avtPDBFileFormat::~avtPDBFileFormat()
{
    factory.Close();
}
    
// ****************************************************************************
// Method: avtPDBFileFormat::FreeUpResources
//
// Purpose: 
//   Frees up resources.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:37:36 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtPDBFileFormat::FreeUpResources(void)
{
    debug4 << "PDB File Format instructed to free up resources." << endl;
    factory.Close();
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets auxiliary data from the file.
//
// Arguments:
//   var   : The variable for which to get aux data.
//   state : The start at which to get aux data.
//   type  : The type of aux data to get.
//   args  : ?
//   df    : A function to delete the aux data.
//
// Returns:    Auxiliary data like materials.
//
// Note:       This is not yet implemented.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:37:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Aug 5 17:29:35 PST 2003
//   I made the implementation do something.
//
// ****************************************************************************

void *
avtPDBFileFormat::GetAuxiliaryData(const char *var, int state, const char *type,
    void *args, DestructorFunction &df)
{
    if (!initializedFactory)
    {
        InitializeFactory(state);
    }

    return factory.GetReader()->GetAuxiliaryData(var, state, type, args, df);
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetCycles
//
// Purpose: 
//   Reads the cycles.
//
// Arguments:
//   cycles : the list of cycles.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 13:32:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtPDBFileFormat::GetCycles(vector<int> &cycles)
{
    if (!initializedFactory)
    {
        InitializeFactory(0);
    }

    // Read the cycles.
    factory.GetReader()->GetCycles(cycles);
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetNTimesteps
//
// Purpose: 
//   Returns the number of timesteps in the database.
//
// Returns:    The number of timesteps.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 13:41:05 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
avtPDBFileFormat::GetNTimesteps()
{
    if (!initializedFactory)
    {
        InitializeFactory(0);
    }

    // Read the cycles.
    return factory.GetReader()->GetNTimesteps();
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetMesh
//
// Purpose: 
//   Gets the named mesh from the PDB file.
//
// Arguments:
//   timeState : The timeState that we want.
//   m         : The name of the mesh.
//
// Returns:    A dataset containing the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:38:17 PST 2002
//
// Modifications:
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
//   Brad Whitlock, Tue Apr 29 10:59:03 PDT 2003
//   Added timeState argument because the format is now MTSD.
//
// ****************************************************************************

vtkDataSet *
avtPDBFileFormat::GetMesh(int timeState, const char *m)
{
    debug5 << "Reading mesh " << m << " for state " << timeState << endl;

    if (!initializedFactory)
    {
        InitializeFactory(timeState);
    }

    vtkDataSet *mesh = factory.GetReader()->GetMesh(timeState, m);

    return mesh;
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetVar
//
// Purpose: 
//   Gets the named scalar variable from the PDB file.
//
// Arguments:
//   timeState : The timeState that we want.
//   s         : The name of the scalar variable.
//
// Returns:    A data array containing the scalar.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:39:00 PST 2002
//
// Modifications:
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
//   Brad Whitlock, Tue Apr 29 10:59:03 PDT 2003
//   Added timeState argument because the format is now MTSD.
//
// ****************************************************************************

vtkDataArray *
avtPDBFileFormat::GetVar(int timeState, const char *s)
{
    debug5 << "Reading scalar " << s << " for state " << timeState << endl;

    if (!initializedFactory)
    {
        InitializeFactory(timeState);
    }

    vtkDataArray *scalar = factory.GetReader()->GetVar(timeState, s);

    return scalar;
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetVectorVar
//
// Purpose: 
//   Gets the named vector variable from the PDB file.
//
// Arguments:
//   timeState : The timeState that we want.
//   v         : The name of the vector variable.
//
// Returns:    A data array containing the vector.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:39:00 PST 2002
//
// Modifications:
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
//   Brad Whitlock, Tue Apr 29 10:59:03 PDT 2003
//   Added timeState argument because the format is now MTSD.
//
// ****************************************************************************

vtkDataArray *
avtPDBFileFormat::GetVectorVar(int timeState, const char *v)
{
    debug5 << "Reading in vector " << v << " for state " << timeState << endl;

    if (!initializedFactory)
    {
        InitializeFactory(timeState);
    }

    vtkDataArray *vec = factory.GetReader()->GetVectorVar(timeState, v);

    return vec;
}

// ****************************************************************************
// Method: avtPDBFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Reads the file metadata and uses it to populate the avtDatabaseMetaData.
//
// Arguments:
//   md : The metadata object that we're populating.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 9 08:30:08 PDT 2002
//
// Modifications:
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

void
avtPDBFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!initializedFactory)
    {
        InitializeFactory(0);
    }

    factory.GetReader()->PopulateDatabaseMetaData(md);
}

// ****************************************************************************
// Method: avtPDBFileFormat::PopulateIOInformation
//
// Purpose: 
//   Populates the IO information.
//
// Arguments:
//   ioInfo : A reference to the IO information that is being populated.
//
// Returns:    
//
// Note:       This doesn't really do much yet.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:40:56 PST 2002
//
// Modifications:
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

void
avtPDBFileFormat::PopulateIOInformation(avtIOInformation &ioInfo)
{
    if (!initializedFactory)
    {
        InitializeFactory(0);
    }

    int nMeshes = metadata->GetNumMeshes();

    if (nMeshes < 1)
    {
        debug1 << "Cannot populate I/O info since there are no meshes" << endl;
        ioInfo.SetNDomains(0);
        return;
    }
}


