// ************************************************************************* //
//                           avtMeshZCoordinateFilter.h                      //
// ************************************************************************* //

#ifndef AVT_MESH_Z_COORDINATE_FILTER_H
#define AVT_MESH_Z_COORDINATE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtMeshZCoordinateFilter
//
//  Purpose:
//      Gets the Z coordinates of a mesh.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
// ****************************************************************************

class EXPRESSION_API avtMeshZCoordinateFilter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtMeshZCoordinateFilter() {;};
    virtual                  ~avtMeshZCoordinateFilter() {;};

    virtual const char       *GetType(void)  
                                    { return "avtMeshZCoordinateFilter"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the z coordinates "
                                             "of the mesh."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 1; }
    virtual bool              IsPointVariable()      { return true; }
};


#endif


