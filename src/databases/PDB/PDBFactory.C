#include <PDBFactory.h>
#include <PF3DReader.h>
#include <SiloReader.h>
#include <InvalidDBTypeException.h>

// ****************************************************************************
// Method: PDBFactory::PDBFactory
//
// Purpose: 
//   Constructor for the PDBFactory class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 09:01:07 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PDBFactory::PDBFactory()
{
    file = NULL;
    reader = NULL;
}

// ****************************************************************************
// Method: PDBFactory::~PDBFactory
//
// Purpose: 
//   Destructor for the PDBFactory class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 09:01:28 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PDBFactory::~PDBFactory()
{
    Close();
}

// ****************************************************************************
// Method: PDBFactory::Open
//
// Purpose: 
//   Opens the PDB file and creates a reader object for it.
//
// Arguments:
//   filename : The name of the PDB file to open.
//
// Returns:    False if the file cannot be opened or if no compatible reader
//             is found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 09:01:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PDBFactory::Open(const char *filename)
{
    bool retval = false;

    //
    // Open the PDB file.
    //
    if((file = PD_open((char *)filename, "r")) != NULL)
    {
        // See if the file is a Silo file.
        if(reader == NULL)
        {
            reader = new SiloReader(file);
            bool isSilo = reader->Identify();
            delete reader;
            reader = NULL;
            if(isSilo)
            {
                EXCEPTION1(InvalidDBTypeException,
                           "The PDB reader does not read Silo files.");
            }
        }

        // See if the file is a PF3D file.
        if(reader == NULL)
        {
            reader = new PF3DReader(file);
            if(!(retval = reader->Identify()))
            {
                delete reader;
                reader = NULL;
            }
        }

        // Try other flavors if reader == NULL.

        // Finally, if no reader can be created for the file, close it
        // and return false.
        if(reader == NULL)
        {
            Close();
            retval = false;
        }
    }
    
    //
    // If no file reader can read the PDB file, or the file was not PDB to
    // begin with, throw an invalid DB exception so the database factory
    // can try to open the file using a different database plugin.
    //
    if(!retval)
    {
        EXCEPTION1(InvalidDBTypeException,
                   "No available PDB reader can read the file.");
    }

    return retval;
}

// ****************************************************************************
// Method: PDBFactory::Close
//
// Purpose: 
//   Closes the file and destroys the reader.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 09:04:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PDBFactory::Close()
{
    if(file != NULL)
    {
        PD_close(file);
        file = NULL;
    }

    if(reader != NULL)
    {
        delete reader;
        reader = NULL;
    }
}
