// ************************************************************************* //
//                           avtPLOT3DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_PLOT3D_FILE_FORMAT_H
#define AVT_PLOT3D_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>


class     vtkVisItPLOT3DReader;


// ****************************************************************************
//  Class: avtPLOT3DFileFormat
//
//  Purpose:
//      A file format reader for PLOT3D files.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

class avtPLOT3DFileFormat : public avtSTMDFileFormat
{
  public:
                          avtPLOT3DFileFormat(const char *);
    virtual              ~avtPLOT3DFileFormat();
    
    virtual const char   *GetType(void) { return "PLOT3D File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    vtkVisItPLOT3DReader *reader;
};


#endif


