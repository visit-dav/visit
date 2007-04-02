// ************************************************************************* //
//                             avtGradientFilter.h                           //
// ************************************************************************* //

#ifndef AVT_GRADIENT_FILTER_H
#define AVT_GRADIENT_FILTER_H

#include <avtSingleInputExpressionFilter.h>


class     vtkCell;
class     vtkDataArray;
class     vtkDataSet;
class     vtkIdList;
class     vtkRectilinearGrid;
class     vtkStructuredGrid;


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
//    Hank Childs, Fri Mar  4 08:21:04 PST 2005
//    Removed data centering conversion modules.
//
//    Hank Childs, Mon Feb 13 14:45:18 PST 2006
//    Add support for logical gradients.  Also add perform restriction, so we
//    can request ghost zones.
//
// ****************************************************************************

class EXPRESSION_API avtGradientFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtGradientFilter();
    virtual                  ~avtGradientFilter();

    void                      SetDoLogicalGradient(bool b)
                               { doLogicalGradients = b; };
    virtual const char       *GetType(void)   { return "avtGradientFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating Gradient"; };

    virtual void              PreExecute(void);

  protected:
    bool                      doLogicalGradients;
    bool                      haveIssuedWarning;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual int               GetVariableDimension() { return 3; }

    float                     EvaluateComponent(float, float, float, float,
                                                float, float, float, 
                                                vtkDataSet *, vtkDataArray *,
                                                vtkIdList *);    
    float                     EvaluateValue(float, float, float, vtkDataSet *,
                                            vtkDataArray *,vtkIdList *,bool &);
    vtkDataArray             *RectilinearGradient(vtkRectilinearGrid *);
    vtkDataArray             *LogicalGradient(vtkStructuredGrid *);
    virtual avtPipelineSpecification_p
                               PerformRestriction(avtPipelineSpecification_p);
};


#endif

