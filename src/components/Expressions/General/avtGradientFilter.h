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
class     vtkPointDataToCellData;
class     vtkRectilinearGrid;
class     vtkScalarData;


// ****************************************************************************
//  Class: avtGradientFilter
//
//  Purpose:
//      A filter that calculates the gradient at each node. Note uses simple
//      definition of looking in the x,y, and z directions. 
//
//  Programmer: Akira Haddox
//  Creation:   July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Dec 13 10:42:15 PST 2003
//    Added support for rectilinear meshes.  Also don't force cell data to be
//    point data in the output.  Added ReleaseData to avoid memory bloat.
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

    virtual void              ReleaseData(void);

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 3; }

    float                     EvaluateComponent(float, float, float, float,
                                                float, float, float, 
                                                vtkDataSet *, vtkDataArray *,
                                                vtkIdList *);    
    float                     EvaluateValue(float, float, float, vtkDataSet *,
                                            vtkDataArray *,vtkIdList *,bool &);
    vtkDataArray             *RectilinearGradient(vtkRectilinearGrid *);

    vtkCellDataToPointData   *cd2pd;
    vtkPointDataToCellData   *pd2cd;
};


#endif

