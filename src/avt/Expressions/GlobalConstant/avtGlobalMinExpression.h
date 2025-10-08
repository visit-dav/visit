// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalMinExpression.h                    //
// ************************************************************************* //

#ifndef AVT_GLOBAL_MIN_FILTER_H
#define AVT_GLOBAL_MIN_FILTER_H

#include <avtGlobalConstantExpression.h>

#include <limits>

class vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalMinExpression
//
//  Purpose:
//      An expression that calculates the minimum across the entire mesh and
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

class EXPRESSION_API avtGlobalMinExpression : public avtGlobalConstantExpression
{
  public:
                              avtGlobalMinExpression();
    virtual                  ~avtGlobalMinExpression();

    virtual const char       *GetType(void)   { return "avtGlobalMinExpression"; }
    virtual const char       *GetDescription(void) 
                                              { return "Calculating min across mesh"; }

  protected:
    // to calculate the minimum we don't need to record any other local quantity
    virtual bool              NeedsExtraIntermediateData() { return false; }
    
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
    // with reductions. In the case of global_min, the safe value is
    // std::numeric_limits<double>::max(), which will disappear when calculating the 
    // minimum between two values or arrays.
    virtual double            GetUnusedValue() { return std::numeric_limits<double>::max(); }
};


#endif


