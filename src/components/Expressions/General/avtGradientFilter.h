// ************************************************************************* //
//                             avtGradientFilter.h                           //
// ************************************************************************* //

#ifndef AVT_GRADIENT_FILTER_H
#define AVT_GRADIENT_FILTER_H


#include <avtSingleInputExpressionFilter.h>


class     vtkCell;
class     vtkCellDataToPointData;
class     vtkDataArray;
class     vtkDataSet;
class     vtkIdList;
class     vtkScalarData;


// ****************************************************************************
//  Class: avtGradientFilter
//
//  Purpose:
//      A filter that calculates the gradient at each node. Note uses simple
//      definition of looking in the x,y, and z directions. 
//
//  Programmer: Matthew Haddox
//  Creation:   July 30, 2002
//
// ****************************************************************************

class EXPRESSION_API avtGradientFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtGradientFilter();
    virtual                  ~avtGradientFilter();

    virtual const char       *GetType(void)   { return "avtGradientFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating Gradient of Each Node"; };


  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return true; };  
    virtual int               GetVariableDimension() { return 3; }

    float                     EvaluateComponent(float, float, float, float,
                                                float, float, float, 
                                                vtkDataSet *, vtkDataArray *,
                                                vtkIdList *);    
    float                     EvaluateValue(float, float, float, vtkDataSet *,
                                            vtkDataArray *,vtkIdList *,bool &);

    vtkCellDataToPointData    *myFilter;
};


#endif

