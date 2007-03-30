#ifndef PDB_FACTORY_H
#define PDB_FACTORY_H
#include <pdb.h>

// Forward declarations
class PDBReader;

// ****************************************************************************
// Class: PDBFactory
//
// Purpose:
//   Opens a PDB file and tests it against PDBReaders to determine the file's
//   type.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 09:00:14 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class PDBFactory
{
public:
    PDBFactory();
    ~PDBFactory();
    bool Open(const char *filename);
    void Close();
    PDBReader *GetReader() { return reader; };
private:
    PDBfile   *file;
    PDBReader *reader;
};

#endif
