// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtSTLFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_STL_FILE_FORMAT_H
#define AVT_STL_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


// ****************************************************************************
//  Class: avtSTLFileFormat
//
//  Purpose:
//      Handles stereo lithography files.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//   Jeremy Meredith, Thu Jan  7 11:46:03 EST 2010
//   Added ability for mdserver to check the file.
//
//    Jean Favre, Fri Jan 27 10:41:38 CET 2012
//    Added DBOptionsAttributes
// ****************************************************************************

class     DBOptionsAttributes;

class avtSTLFileFormat : public avtSTSDFileFormat
{
  public:
                          avtSTLFileFormat(const char *, const DBOptionsAttributes*);
    virtual              ~avtSTLFileFormat();

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

    virtual const char   *GetType(void)  { return "Stereo Lithography File ";};

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

  protected:
    vtkDataSet           *dataset;
    bool                  readInDataset;
    bool                  checkedFile;

    static const char    *MESHNAME;

    void                  ReadInDataset(void);
};


#endif


