// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalMaxExpression.h                    //
// ************************************************************************* //

#ifndef AVT_GLOBAL_MAX_FILTER_H
#define AVT_GLOBAL_MAX_FILTER_H

#include <avtGlobalConstantExpression.h>

#include <limits>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalMaxExpression
//
//  Purpose:
//      An expression that calculates the maximum across the entire mesh and
//      paints the result on every node or zone.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//      Justin Privitera, Wed Oct  8 14:13:21 PDT 2025
//      Changed inheritance from avtGhostAwareUnaryMathExpression to 
//      avtGlobalConstantExpression, which required adding
//      NeedsExtraIntermediateData(), LocalIntermediateReduction(),
//      GlobalIntermediateReduction(), CalculateFinalResults(), and 
//      GetUnusedValue().
//
// ****************************************************************************

class EXPRESSION_API avtGlobalMaxExpression : public avtGlobalConstantExpression
{
  public:
                              avtGlobalMaxExpression();
    virtual                  ~avtGlobalMaxExpression();

    virtual const char       *GetType(void)   { return "avtGlobalMaxExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating max across mesh"; };

  protected:    
    // to calculate the maximum we don't need to record any other local quantity
    virtual bool              NeedsExtraIntermediateData() { return false; };
    
    virtual void              CalculateWithoutGhosts(vtkDataArray *in,
                                                     const int ncomponents,
                                                     const int ntuples,
                                                     std::vector<double> &constant_results,
                                                     std::vector<double> &extra_constant_results);

    virtual void              CalculateWithGhosts(vtkDataArray *in,
                                                  const int ncomponents,
                                                  const int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr,
                                                  std::vector<double> &constant_results,
                                                  std::vector<double> &extra_constant_results);

    virtual double            LocalIntermediateReduction(const double running_reduction,
                                                         const double intermediate_value);

    virtual void              GlobalIntermediateReduction(std::vector<double> &local_constant_results,
                                                          std::vector<double> &global_constant_results,
                                                          const int ncomps);

    virtual void              CalculateFinalResults(const std::vector<double> &global_constant_results,
                                                    const std::vector<double> &global_extra_constant_results,
                                                    const int global_ncomps,
                                                    const int global_ntuples,
                                                    std::vector<double> &final_results);

    // For cases where there is no data, we need to provide a value that will not interfere 
    // with reductions. In the case of global_max, the safe value is
    // std::numeric_limits<double>::lowest(), which will disappear when calculating the 
    // maximum between two values or arrays.
    virtual double            GetUnusedValue() { return std::numeric_limits<double>::lowest(); };
};


#endif


