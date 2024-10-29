// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalVarianceExpression.h               //
// ************************************************************************* //

#ifndef AVT_GLOBAL_VARIANCE_FILTER_H
#define AVT_GLOBAL_VARIANCE_FILTER_H

#include <avtGhostAwareUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalVarianceExpression
//
//  Purpose:
//      An expression that calculates the variance across the entire mesh and
//      paints the result on every node or zone.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtGlobalVarianceExpression : public avtGhostAwareUnaryMathExpression
{
  public:
                              avtGlobalVarianceExpression();
    virtual                  ~avtGlobalVarianceExpression();

    virtual const char       *GetType(void)   { return "avtGlobalVarianceExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating variance across mesh"; };

  protected:
    virtual void              CalculateWithoutGhosts(vtkDataArray *in, vtkDataArray *out,
                                                     int ncomponents, int ntuples);
    virtual void              CalculateWithGhosts(vtkDataArray *in, vtkDataArray *out,
                                                  int ncomponents, int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr);
};


#endif


