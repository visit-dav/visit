// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_CCSM_READER_H
#define AVT_CCSM_READER_H
#include <avtNETCDFReaderBase.h>
#include <map>
#include <string>
#include <vectortypes.h>

class NETCDFFileObject;
class avtDatabaseMetaData;
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
// Class: avtCCSMReader
//
// Purpose:
//   This class reads CCSM files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 27 14:39:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class avtCCSMReader : public avtNETCDFReaderBase
{
public:
                   avtCCSMReader(const char *);
                   avtCCSMReader(const char *, NETCDFFileObject *);
    virtual       ~avtCCSMReader();

    vtkDataSet    *GetMesh(int, const char *);
    vtkDataArray  *GetVar(int, const char *);

    void           PopulateDatabaseMetaData(int, avtDatabaseMetaData *);
protected:
    // DATA MEMBERS
    StringIntVectorMap     meshNameToDimensions;
    StringIntVectorMap     varToDimensions;

    size_t                *dimSizes;

    bool                   initialized;
};

#endif
