// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalAvgExpression.h                    //
// ************************************************************************* //

#ifndef AVT_GLOBAL_AVG_FILTER_H
#define AVT_GLOBAL_AVG_FILTER_H

#include <avtGhostAwareUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalAvgExpression
//
//  Purpose:
//      An expression that calculates the average across the entire mesh and
//      paints the result on every node or zone.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtGlobalAvgExpression : public avtGhostAwareUnaryMathExpression
{
  public:
                              avtGlobalAvgExpression();
    virtual                  ~avtGlobalAvgExpression();

    virtual const char       *GetType(void)   { return "avtGlobalAvgExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating average across mesh"; };

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


