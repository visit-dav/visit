#ifndef SILO_READER_H
#define SILO_READER_H
#include <PDBReader.h>

// Forward declarations.
class avtDatabaseMetaData;
class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: SiloReader
//
// Purpose:
//   This class tries to identify a Silo file so it can fail the PDB test so
//   the file can be opened by another reader plugin.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 11 17:40:34 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 13:24:01 PST 2003
//   I added a time argument to GetMesh, GetVar, and GetVectorVar.
//
//   Brad Whitlock, Thu Aug 7 16:55:57 PST 2003
//   I added a pointer to a variable cache.
//
// ****************************************************************************

class SiloReader : public PDBReader
{
public:
    SiloReader(PDBfile *p, avtVariableCache *c) : PDBReader(p,c) { };
    virtual ~SiloReader() { };

    virtual bool Identify();
    virtual void GetTimeVaryingInformation(int,avtDatabaseMetaData *) { };
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *) { };
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);
};

#endif
