// ************************************************************************* //
//                             avtPLOT2DFileFormat.h                         //
// ************************************************************************* //

#ifndef AVT_PLOT2D_FILE_FORMAT_H
#define AVT_PLOT2D_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include <string>


class     vtkFloatArray;


// ****************************************************************************
//  Class: avtPLOT2DFileFormat
//
//  Purpose:
//      Handles 2D single grid PLOT3D files for Wai Sun Don.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2003
//
// ****************************************************************************

class avtPLOT2DFileFormat : public avtSTSDFileFormat
{
  public:
                          avtPLOT2DFileFormat(const char *);
    virtual              ~avtPLOT2DFileFormat();

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

    virtual const char   *GetType(void)  { return "2D PLOT3D"; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

  protected:
    std::string           grid_filename;
    std::string           soln_filename;
    static const char    *MESHNAME;
    bool                  haveReadVars;
    vtkFloatArray        *rho;
    vtkFloatArray        *u;
    vtkFloatArray        *v;
    vtkFloatArray        *E;
};


#endif


