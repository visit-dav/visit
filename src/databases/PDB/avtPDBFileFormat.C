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
//   
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Do not do so much work in the constructor.
//
// ****************************************************************************

avtPDBFileFormat::avtPDBFileFormat(const char *dbFileName) :
    avtSTSDFileFormat(dbFileName), factory()
{
    initializedFactory = false;
}

// ****************************************************************************
//  Method: avtPDBFileFormat::InitializeFactory
//
//  Purpose:
//      Has the factory actually open the file.
//
//  Programmer: Hank Childs
//  Creation:   April 7, 2003
//
// ****************************************************************************

void
avtPDBFileFormat::InitializeFactory(void)
{
    //
    // Get the file. This will try to open the PDB file with various readers
    // and see if any of the readers can identify the format of the PDB file.
    // If no reader can identify the format, we throw an InvalidDBTypeException.
    //
    factory.Open(filename);
    factory.GetReader()->GetTimeVaryingInformation(metadata);

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
//
// Returns:    
//
// Note:       This is not yet implemented.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:37:54 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void *
avtPDBFileFormat::GetAuxiliaryData(const char *, const char *, void *,
    DestructorFunction &)
{
    return NULL;
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetMesh
//
// Purpose: 
//   Gets the named mesh from the PDB file.
//
// Arguments:
//   m : The name of the mesh.
//
// Returns:    A dataset containing the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:38:17 PST 2002
//
// Modifications:
//   
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

vtkDataSet *
avtPDBFileFormat::GetMesh(const char *m)
{
    debug5 << "Reading mesh " << m << endl;

    if (!initializedFactory)
    {
        InitializeFactory();
    }

    vtkDataSet *mesh = factory.GetReader()->GetMesh(m);

    return mesh;
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetVar
//
// Purpose: 
//   Gets the named scalar variable from the PDB file.
//
// Arguments:
//   s : The name of the scalar variable.
//
// Returns:    A data array containing the scalar.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:39:00 PST 2002
//
// Modifications:
//   
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

vtkDataArray *
avtPDBFileFormat::GetVar(const char *s)
{
    debug5 << "Reading scalar " << s << endl;

    if (!initializedFactory)
    {
        InitializeFactory();
    }

    vtkDataArray *scalar = factory.GetReader()->GetVar(s);

    return scalar;
}

// ****************************************************************************
// Method: avtPDBFileFormat::GetVectorVar
//
// Purpose: 
//   Gets the named vector variable from the PDB file.
//
// Arguments:
//   v : The name of the vector variable.
//
// Returns:    A data array containing the vector.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 23 14:39:00 PST 2002
//
// Modifications:
//   
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

vtkDataArray *
avtPDBFileFormat::GetVectorVar(const char *v)
{
    debug5 << "Reading in vector " << v << endl;

    if (!initializedFactory)
    {
        InitializeFactory();
    }

    vtkDataArray *vec = factory.GetReader()->GetVectorVar(v);

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
//   
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

void
avtPDBFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!initializedFactory)
    {
        InitializeFactory();
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
//   
//   Hank Childs, Mon Apr  7 18:44:46 PDT 2003
//   Make sure the factory has been initialized.
//
// ****************************************************************************

void
avtPDBFileFormat::PopulateIOInformation(avtIOInformation &ioInfo)
{
    if (!initializedFactory)
    {
        InitializeFactory();
    }

    int nMeshes = metadata->GetNumMeshes();

    if (nMeshes < 1)
    {
        debug1 << "Cannot populate I/O info since there are no meshes" << endl;
        ioInfo.SetNDomains(0);
        return;
    }
}


