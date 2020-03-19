// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtGradientExpression.h                       //
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
//  Class: avtGradientExpression
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
//    Cyrus Harrison, Wed Aug  8 11:17:51 PDT 2007
//    Add support for multiple gradient algorithms.
//
//    Cyrus Harrison, Tue Apr  1 11:06:28 PDT 2008
//    Added IsPointVariable() to deal with NZQH centering change.
//
//    Hank Childs, Fri Jan  9 17:56:00 CST 2009
//    Added the approximate gradient option, to be used with ray casted
//    volume rendering.
//
//    Hank Childs, Sat Dec  4 11:30:22 PST 2010
//    Added a static method for calculating expressions.  This allows for other
//    places in VisIt to access a gradient without instantiating the filter.
//
// ****************************************************************************

typedef enum
{
    SAMPLE  =  0,
    LOGICAL , /* 1 */
    NODAL_TO_ZONAL_QUAD_HEX, /* 2 */
    FAST /* 3 */
} GradientAlgorithmType;


class EXPRESSION_API avtGradientExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtGradientExpression();
    virtual                  ~avtGradientExpression();

    void                      SetAlgorithm(GradientAlgorithmType algo)
                               {gradientAlgo = algo;}

    virtual const char       *GetType(void)   { return "avtGradientExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating Gradient"; };

    virtual void              PreExecute(void);
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

    static vtkDataArray      *CalculateGradient(vtkDataSet *, const char *,
                                                GradientAlgorithmType=SAMPLE);

  protected:
    GradientAlgorithmType     gradientAlgo;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual int               GetVariableDimension() { return 3; }
    virtual bool              IsPointVariable(void);
    
    virtual avtContract_p     ModifyContract(avtContract_p);

    static vtkDataArray      *RectilinearGradient(vtkRectilinearGrid *, 
                                                  const char *);
    static vtkDataArray      *LogicalGradient(vtkStructuredGrid *, 
                                              const char *);
    static vtkDataArray      *NodalToZonalQuadHexGrad(vtkStructuredGrid *, 
                                                      const char *);
    static vtkDataArray      *FastGradient(vtkDataSet *, const char *);
    static void               CalculateNodalToZonalQuadGrad(vtkDataSet *,
                                                            vtkDataArray *,
                                                            int ,
                                                            double *);
    static void               CalculateNodalToZonalHexGrad(vtkDataSet *,
                                                           vtkDataArray *,
                                                           int ,
                                                           double *);
    static double             EvaluateComponent(double, double, double, double,
                                                double, double, double,
                                                vtkDataSet *, vtkDataArray *,
                                                vtkIdList *);
    static double             EvaluateValue(double, double, double, 
                                            vtkDataSet *,
                                            vtkDataArray *,vtkIdList *,bool &);
};


#endif

