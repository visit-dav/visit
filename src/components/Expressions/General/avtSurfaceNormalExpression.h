// ************************************************************************* //
//                         avtSurfaceNormalExpression.h                       //
// ************************************************************************* //

#ifndef AVT_SURFACE_NORMAL_EXPRESSION_H
#define AVT_SURFACE_NORMAL_EXPRESSION_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtSurfaceNormalExpression
//
//  Purpose:
//      An expression that determines which nodes are external.
//          
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

class EXPRESSION_API avtSurfaceNormalExpression 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtSurfaceNormalExpression();
    virtual                  ~avtSurfaceNormalExpression();

    virtual const char       *GetType(void) 
                                      { return "avtSurfaceNormalExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Calculating surface normal"; };
    void                      DoPointNormals(bool val) { isPoint = val; };

  protected:
    bool                      isPoint;
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return isPoint; };
    virtual int               GetVariableDimension(void) { return 3; };
};


#endif


