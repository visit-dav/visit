// ************************************************************************* //
//                           avtEnSightFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_EnSight_FILE_FORMAT_H
#define AVT_EnSight_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>


class     vtkVisItGenericEnSightReader;


// ****************************************************************************
//  Class: avtEnSightFileFormat
//
//  Purpose:
//      A file format reader for EnSight files.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Jul  9 07:37:46 PDT 2004
//    Make the reader be an MTMD file format.
//
// ****************************************************************************

class avtEnSightFileFormat : public avtMTMDFileFormat
{
  public:
                          avtEnSightFileFormat(const char *);
    virtual              ~avtEnSightFileFormat();
    
    virtual const char   *GetType(void) { return "EnSight File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int           GetNTimesteps(void);

    virtual void          RegisterVariableList(const char *,
                                               const std::vector<CharStrRef>&);

  protected:
    vtkVisItGenericEnSightReader *reader;
    bool                  doneUpdate;

    void                  PrepReader(int);
    void                  InstantiateReader(const char *);
};


#endif


