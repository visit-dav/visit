// ************************************************************************* //
//                         avtWavefrontOBJFileFormat.h                       //
// ************************************************************************* //

#ifndef AVT_WAVEFRONT_OBJ_FILE_FORMAT_H
#define AVT_WAVEFRONt_OBJ_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


// ****************************************************************************
//  Class: avtWavefrontOBJFileFormat
//
//  Purpose:
//      Handles files of the .obj file format.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

class avtWavefrontOBJFileFormat : public avtSTSDFileFormat
{
  public:
                          avtWavefrontOBJFileFormat(const char *);
    virtual              ~avtWavefrontOBJFileFormat();

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

    virtual const char   *GetType(void)  { return "WavefrontOBJ File Format"; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

  protected:
    vtkDataSet           *dataset;
    bool                  readInDataset;
    int                   nVars;

    static const char    *MESHNAME;
    static const char    *VARNAME1;
    static const char    *VARNAME2;

    void                  ReadInDataset(void);
};


#endif


