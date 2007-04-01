// ************************************************************************* //
//                             avtVTKFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_VTK_FILE_FORMAT_H
#define AVT_VTK_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

class vtkStructuredPoints;

// ****************************************************************************
//  Class: avtVTKFileFormat
//
//  Purpose:
//      Handles files of the .vtk file format.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Thu Mar 11 12:53:12 PST 2004 
//    Added ConvertStructuredPointsToRGrid. 
//
// ****************************************************************************

class avtVTKFileFormat : public avtSTSDFileFormat
{
  public:
                          avtVTKFileFormat(const char *);
    virtual              ~avtVTKFileFormat();

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);
    virtual vtkDataArray *GetVectorVar(const char *);

    virtual const char   *GetType(void)  { return "VTK File Format"; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

  protected:
    vtkDataSet           *dataset;
    bool                  readInDataset;

    static const char    *MESHNAME;
    static const char    *VARNAME;

    void                  ReadInDataset(void);
    vtkDataSet           *ConvertStructuredPointsToRGrid(vtkStructuredPoints *);
};


#endif


