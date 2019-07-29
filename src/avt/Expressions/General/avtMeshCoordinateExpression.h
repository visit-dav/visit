// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtMeshCoordinateExpression.h                      //
// ************************************************************************* //

#ifndef AVT_MESH_COORDINATE_FILTER_H
#define AVT_MESH_COORDINATE_FILTER_H

#include <avtSingleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtMeshCoordinateExpression
//
//  Purpose:
//      Gets the coordinates of a mesh.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
//  Modifications:
//      Sean Ahern, Thu Mar  6 01:47:29 America/Los_Angeles 2003
//      Combined this with the other coordinate filters.
//
//      Sean Ahern, Fri Mar  7 21:21:20 America/Los_Angeles 2003
//      Made this return a vector of coordinates, rather than just one.
//
//      Hank Childs, Thu Feb  5 17:11:06 PST 2004
//      Moved inlined constructor and destructor definitions to .C files
//      because certain compilers have problems with them.
//
//      Hank Childs, Wed Feb 25 11:41:35 PST 2004
//      Corrected the variable dimension of the output.
//
// ****************************************************************************

class EXPRESSION_API avtMeshCoordinateExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtMeshCoordinateExpression();
    virtual                  ~avtMeshCoordinateExpression();

    virtual const char       *GetType(void)  
                                    { return "avtMeshCoordinateExpression"; };
    virtual const char       *GetDescription(void)
                                    { return "Pulling out the coordinates "
                                             "of the mesh."; };

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension() { return 3; }
    virtual bool              IsPointVariable()      { return true; }
};


#endif


