// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtGhostAwareUnaryMathExpression.h             //
// ************************************************************************* //

#ifndef AVT_GHOST_AWARE_UNARY_MATH_FILTER_H
#define AVT_GHOST_AWARE_UNARY_MATH_FILTER_H

#include <expression_exports.h>

#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtGhostAwareUnaryMathExpression
//
//  Purpose:
//      A filter that performs a calculation on a single variable, taking 
//      ghost zones/nodes into account.
//
//  Programmer: Justin Privitera
//  Creation:   10/25/24
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtGhostAwareUnaryMathExpression : public avtUnaryMathExpression
{
  public:
                              avtGhostAwareUnaryMathExpression();
    virtual                  ~avtGhostAwareUnaryMathExpression();

    virtual const char       *GetType(void)   { return "avtGhostAwareUnaryMathExpression";};
    virtual const char       *GetDescription(void) = 0;

    static std::vector<int>   IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                   vtkDataArray *ghostZones,
                                                   vtkDataArray *ghostNodes);

  protected:
    virtual void              CalculateWithoutGhosts(vtkDataArray *in, vtkDataArray *out,
                                                     int ncomponents, int ntuples) = 0;
    virtual void              CalculateWithGhosts(vtkDataArray *in, vtkDataArray *out,
                                                  int ncomponents, int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr) = 0;
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, 
                                          vtkDataSet *in_ds);
};


#endif


