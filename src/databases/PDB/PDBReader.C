#include <PDBReader.h>
#include <DebugStream.h>
#include <string.h>

// ****************************************************************************
// Method: PDBReader::PDBReader
//
// Purpose: 
//   Constructor for the PDBReader class.
//
// Arguments:
//   p : A pointer to the PDBfile object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:58:35 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PDBReader::PDBReader(PDBfile *p)
{
    pdb = p;
}

// ****************************************************************************
// Method: PDBReader::~PDBReader
//
// Purpose:
//   Destructor for the PDBReader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:59:02 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PDBReader::~PDBReader()
{
}

// ****************************************************************************
// Method: PDBReader::GetDouble
//
// Purpose: 
//   Reads the named double from the pdb file and returns its value.
//
// Arguments:
//   pdb  : A pointer to a pdb file object.
//   name : The name of the variable to read.
//   val  : The destination variable.
//
// Returns:    true if successful; otherwise false.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 9 09:26:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PDBReader::GetDouble(char *name, double *val)
{
    bool retval = false;
    syment *ep;

    if((ep = PD_inquire_entry(pdb, name, 0, NULL)) != NULL)
    {
        debug4 << "PDBReader::GetDouble: var="<< name
               << ", type=" << PD_entry_type(ep) << endl;

        if(strcmp(PD_entry_type(ep), "double") == 0)
        {
            retval = (PD_read(pdb, name, (void *)val) ==  TRUE);
        }
        else if(strcmp(PD_entry_type(ep), "float") == 0)
        {
            float tmp;
            retval = (PD_read(pdb, name, (void *)&tmp) ==  TRUE);
            if(retval)
                *val = (double)tmp;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PDBReader::GetInteger
//
// Purpose: 
//   Reads the named integer from the pdb file and returns its value.
//
// Arguments:
//   pdb  : A pointer to a pdb file object.
//   name : The name of the variable to read.
//   val  : The destination variable.
//
// Returns:    true if successful; otherwise false.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:56:46 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PDBReader::GetInteger(char *name, int *val)
{
    bool retval = false;
    syment *ep;

    if((ep = PD_inquire_entry(pdb, name, 0, NULL)) != NULL)
    {
        debug4 << "PDBReader::GetInteger: var="<< name
               << ", type=" << PD_entry_type(ep) << endl;

        if(strcmp(PD_entry_type(ep), "integer") == 0 ||
           strcmp(PD_entry_type(ep), "int") == 0)
        {
            retval = (PD_read(pdb, name, (void *)val) ==  TRUE);
        }
        else if(strcmp(PD_entry_type(ep), "long") == 0)
        {
            long tmp;
            retval = (PD_read(pdb, name, (void *)&tmp) ==  TRUE);
            if(retval)
                *val = (int)tmp;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PDBReader::SymbolExists
//
// Purpose: 
//   Tests the PDB file to see if a symbol exists.
//
// Arguments:
//   name : The name of the symbol we're looking for.
//
// Returns:    True if the symbol is in the file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:55:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PDBReader::SymbolExists(char *name)
{
    return (PD_inquire_entry(pdb, name, 0, NULL) != NULL);
}
