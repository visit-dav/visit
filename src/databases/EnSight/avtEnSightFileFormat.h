// ************************************************************************* //
//                           avtEnSightFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_EnSight_FILE_FORMAT_H
#define AVT_EnSight_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <string>


class     vtkGenericEnSightReader;


// ****************************************************************************
//  Class: avtEnSightFileFormat
//
//  Purpose:
//      A file format reader for EnSight files.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

class avtEnSightFileFormat : public avtMTSDFileFormat
{
  public:
                          avtEnSightFileFormat(const char *);
    virtual              ~avtEnSightFileFormat();
    
    virtual const char   *GetType(void) { return "EnSight File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int           GetNTimesteps(void);

    virtual void          RegisterVariableList(const char *,
                                               const std::vector<CharStrRef>&);

  protected:
    vtkGenericEnSightReader *reader;
    bool                  doneUpdate;

    void                  PrepReader(int);
    void                  InstantiateReader(const char *);
};


#endif


