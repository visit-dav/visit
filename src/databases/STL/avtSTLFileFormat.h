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
// ****************************************************************************

class avtSTLFileFormat : public avtSTSDFileFormat
{
  public:
                          avtSTLFileFormat(const char *);
    virtual              ~avtSTLFileFormat();

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

    virtual const char   *GetType(void)  { return "Stereo Lithography File ";};

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

  protected:
    vtkDataSet           *dataset;
    bool                  readInDataset;

    static const char    *MESHNAME;

    void                  ReadInDataset(void);
};


#endif


