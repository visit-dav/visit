// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalVarianceExpression.h               //
// ************************************************************************* //

#ifndef AVT_GLOBAL_VARIANCE_FILTER_H
#define AVT_GLOBAL_VARIANCE_FILTER_H

#include <avtGlobalConstantExpression.h>

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

class EXPRESSION_API avtGlobalVarianceExpression : public avtGlobalConstantExpression
{
  public:
                              avtGlobalVarianceExpression();
    virtual                  ~avtGlobalVarianceExpression();

    virtual const char       *GetType(void)   { return "avtGlobalVarianceExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating variance across mesh"; };

  protected:
    // to calculate the variance we need to take into account the number of tuples
    virtual bool              NeedsNTuples() { return true; };
    
    // to calculate the variance we need to take into account the sum of elements
    virtual bool              NeedsSums() { return true; };

    // to calculate the variance we need to record local quantities
    virtual bool              NeedsIntermediateData() { return true; };
    
    // to calculate the variance we need to record another local quantity
    virtual bool              NeedsExtraIntermediateData() { return true; };
    
    virtual void              CalculateWithoutGhosts(vtkDataArray *in,
                                                     const int ncomponents,
                                                     const int ntuples,
                                                     std::vector<double> &constant_results,
                                                     std::vector<double> &extra_constant_results,
                                                     std::vector<double> &sums);

    virtual void              CalculateWithGhosts(vtkDataArray *in,
                                                  const int ncomponents,
                                                  const int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr,
                                                  std::vector<double> &constant_results,
                                                  std::vector<double> &extra_constant_results,
                                                  std::vector<double> &sums);

    virtual double            LocalIntermediateReduction(const double running_reduction,
                                                         const double intermediate_value);

    virtual void              GlobalIntermediateReduction(std::vector<double> &local_constant_results,
                                                          std::vector<double> &global_constant_results,
                                                          const int ncomps);

    virtual void              CalculateFinalResults(const std::vector<double> &global_constant_results,
                                                    const std::vector<double> &global_extra_constant_results,
                                                    const std::vector<double> &global_component_sums,
                                                    const int global_ncomps,
                                                    const int global_ntuples,
                                                    std::vector<double> &final_results);

    virtual double            GetUnusedValue() { return 0.0; };
};


#endif


