#ifndef LEOSFILE_FORMAT_H
#define LEOSFILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <LEOSFileReader.h>

class avtFileFormatInterface;

// ****************************************************************************
// Class: LEOSFileFormat
//
// Purpose:
//   This file format reads LEOS files 
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

class LEOSFileFormat : public avtSTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *filename);

    LEOSFileFormat(const char *filename);
    LEOSFileFormat(PDBFileObject *p);
    virtual ~LEOSFileFormat();

    // Mimic PDBReader interface.
    bool Identify();
    void SetOwnsPDBFile(bool);

    // Methods overrides for an STSD file format.
    virtual const char   *GetType();
    virtual int           GetCycle(void);
    virtual double        GetTime(void);
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual vtkDataSet   *GetMesh(const char *var);
    virtual vtkDataArray *GetVar(const char *var);

protected:
    LEOSFileReader        reader;
};

#endif
