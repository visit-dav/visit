// ************************************************************************* //
//                           avtPoint3DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_POINT3D_FILE_FORMAT_H
#define AVT_POINT3D_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


#include <vector>
#include <string>


class     vtkFloatArray;
class     vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtPoint3DFileFormat
//
//  Purpose:
//      A file format reader for curves.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

class avtPoint3DFileFormat : public avtSTSDFileFormat
{
  public:
                               avtPoint3DFileFormat(const char *);
    virtual                   ~avtPoint3DFileFormat();
    
    virtual const char        *GetType(void) { return "3D points"; };
    
    virtual vtkDataSet        *GetMesh(const char *);
    virtual vtkDataArray      *GetVar(const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int                GetCycle(void) { return 0; };

  protected:
    bool                       haveReadData;
    vtkFloatArray             *column1;
    vtkFloatArray             *column2;
    vtkFloatArray             *column3;
    vtkFloatArray             *column4;
    vtkUnstructuredGrid       *points;
    std::vector<std::string>   varnames;

    static const char         *MESHNAME;

    void                       ReadData(void);
};


#endif


