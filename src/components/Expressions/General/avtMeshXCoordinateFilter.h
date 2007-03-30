// ************************************************************************* //
//                           avtMeshXCoordinateFilter.h                      //
// ************************************************************************* //

#ifndef AVT_MESH_X_COORDINATE_FILTER_H
#define AVT_MESH_X_COORDINATE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtMeshXCoordinateFilter
//
//  Purpose:
//      Gets the X coordinates of a mesh.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
// ****************************************************************************

class EXPRESSION_API avtMeshXCoordinateFilter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtMeshXCoordinateFilter() {;};
    virtual                  ~avtMeshXCoordinateFilter() {;};

    virtual const char       *GetType(void)  
                                    { return "avtMeshXCoordinateFilter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the x coordinates "
                                             "of the mesh."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
    virtual bool              IsPointVariable()      { return true; }
};


#endif


