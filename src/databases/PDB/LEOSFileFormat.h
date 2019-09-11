// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LEOSFILE_FORMAT_H
#define LEOSFILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <LEOSFileReader.h>

class avtFileFormatInterface;
class DBOptionsAttributes;

// ****************************************************************************
// Class: LEOSFileFormat
//
// Purpose:
//   This file format reads LEOS files 
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// Modifications:
//   Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//   Added read options to constructors
// ****************************************************************************

class LEOSFileFormat : public avtSTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *filename, const DBOptionsAttributes *rdopts);

    LEOSFileFormat(const char *filename, const DBOptionsAttributes *rdopts);
    LEOSFileFormat(PDBFileObject *p, const DBOptionsAttributes *rdopts);
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
