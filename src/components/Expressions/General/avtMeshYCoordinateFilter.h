// ************************************************************************* //
//                           avtMeshYCoordinateFilter.h                      //
// ************************************************************************* //

#ifndef AVT_MESH_Y_COORDINATE_FILTER_H
#define AVT_MESH_Y_COORDINATE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtMeshYCoordinateFilter
//
//  Purpose:
//      Gets the Y coordinates of a mesh.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
// ****************************************************************************

class EXPRESSION_API avtMeshYCoordinateFilter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtMeshYCoordinateFilter() {;};
    virtual                  ~avtMeshYCoordinateFilter() {;};

    virtual const char       *GetType(void)  
                                    { return "avtMeshYCoordinateFilter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the y coordinates "
                                             "of the mesh."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
    virtual bool              IsPointVariable()      { return true; }
};


#endif


