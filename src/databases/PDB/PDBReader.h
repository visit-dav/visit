#ifndef PDB_READER_H
#define PDB_READER_H
#include <pdb.h>

// Forward declarations
class avtDatabaseMetaData;
class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: PDBReader
//
// Purpose:
//   Abstract base class for PDB file readers.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:55:05 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class PDBReader
{
public:
    PDBReader(PDBfile *pdb);
    virtual ~PDBReader();

    virtual bool Identify() = 0;
    virtual void GetTimeVaryingInformation(avtDatabaseMetaData *) = 0;
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *) = 0;
    virtual vtkDataSet   *GetMesh(const char *) = 0;
    virtual vtkDataArray *GetVar(const char *) = 0;
    virtual vtkDataArray *GetVectorVar(const char *) = 0;

    bool GetDouble(char *name, double *val);
    bool GetInteger(char *name, int *val);
    bool SymbolExists(char *name);
protected:
    PDBfile *pdb;
};

#endif
