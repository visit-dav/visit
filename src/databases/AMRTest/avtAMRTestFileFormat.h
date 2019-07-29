// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtAMRTestFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_AMRTEST_FILE_FORMAT_H
#define AVT_AMRTEST_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>


// ****************************************************************************
//  Class: avtAMRTestFileFormat
//
//  Purpose:
//      A simple example of an AMR file reader.
//
//  Programmer:  Eric Brugger
//  Creation:    November 20, 2013
//
// ****************************************************************************

class avtAMRTestFileFormat : public avtSTMDFileFormat
{
  public:
                          avtAMRTestFileFormat(const char *);
    virtual              ~avtAMRTestFileFormat();
    
    virtual const char   *GetType(void) { return "AMRTest File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    void                  GetLevelAndLocalPatch(int, int&, int&);
    void                  PopulateDomainNesting();

    std::string           filename;

    int                   maxLevels;
    int                   numLevels;
    int                   totalPatches;

    int                   nPatchesForLevel[10];
    int                   patchOffsetForLevel[10];
    int                   refinementForLevel[10];
};


#endif
