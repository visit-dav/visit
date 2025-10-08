// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtGlobalConstantExpression.h                       //
// ************************************************************************* //

#ifndef AVT_GLOBAL_CONSTANT_EXPRESSION_H
#define AVT_GLOBAL_CONSTANT_EXPRESSION_H

#include <avtExpressionFilter.h>

class     vtkDataArray;
class     vtkDataSet;
class     intermediateResults;

// ****************************************************************************
//  Class: avtGlobalConstantExpression
//
//  Purpose:
//      A class of expressions that result in a constant-valued variable
//      across the entire mesh. There are several levels of reductions that
//      make this possible:
//      1. Each data tree leaf contains some number of tuples that are reduced
//         across.
//      2. Then a reduction is performed across all of the tree leaf results.
//      3. If we are running in parallel, a reduction is performed across all
//         of the global results.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtGlobalConstantExpression : public avtExpressionFilter
{
  public:
                              avtGlobalConstantExpression();
    virtual                  ~avtGlobalConstantExpression();

    virtual const char       *GetType(void)
                                     { return "avtGlobalConstantExpression"; };
    virtual const char       *GetDescription(void) = 0;

    virtual int               NumVariableArguments() { return 1; };
  protected:
    int                       currentProgress;
    int                       totalSteps;
    avtCentering              centering;

    virtual void              Execute(void);

    // some expressions (like global_std_dev and global_variance) require an 
    // extra array of intermediate data, as they have additional quantities 
    // that must be accumulated across all processors in order to calculate a 
    // final answer, so we provide this method as a toggle for children of 
    // avtGlobalConstantExpression to enable computation with an additional
    // data array.
    virtual bool              NeedsExtraIntermediateData() { return false; };

    // All children of this class must implement a method to perform a reduction across
    // a data tree leaf without taking into account ghost zones or nodes. This class
    // can deduce if it needs to track ghosts or not and can use this method as a shortcut.
    virtual void              CalculateWithoutGhosts(vtkDataArray *in,
                                                     const int ncomponents,
                                                     const int ntuples,
                                                     std::vector<double> &constant_results,
                                                     std::vector<double> &extra_constant_results) = 0;

    // all children of this class must implement a method to perform a reduction across
    // a data tree leaf taking into account ghost zones or nodes.
    virtual void              CalculateWithGhosts(vtkDataArray *in,
                                                  const int ncomponents,
                                                  const int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr,
                                                  std::vector<double> &constant_results,
                                                  std::vector<double> &extra_constant_results) = 0;

    // A "Local Intermediate Reduction" is a step in which results are reduced across
    // an avtDataTree on one particular rank. The children of this class implement this
    // method to tell us how to reduce pairs of elements. For example, in the case of 
    // the global_sum expression, these values are added together.
    virtual double            LocalIntermediateReduction(const double running_reduction,
                                                         const double intermediate_value) = 0;

    // A "Global Intermediate Reduction" is a step in which results are reduced across
    // all ranks. The children of this class implement this method to tell us how to 
    // reduce arrays of intermediate data. For example, in the case of the global_sum
    // expression, the local arrays are added together using MPI and the result is 
    // stored in the provided global array.
    virtual void              GlobalIntermediateReduction(std::vector<double> &local_constant_results,
                                                          std::vector<double> &global_constant_results,
                                                          const int ncomps) = 0;

    // With some expressions, like global_sum, we don't need another step to calculate
    // after using MPI to take the sum across all ranks. But other expressions, like
    // global_avg, need an additional step, since we need to calculate the sum across
    // all ranks and the number of non-ghosted tuples across all ranks so we can divide
    // the two quantities. That's what CalculateFinalResults is for. Some children of
    // this class simply use std::move() to have final_results take ownership of the 
    // already computed constant results, while others perform a final calculation
    // using all of the computed quantities that were reduced across all ranks.
    virtual void              CalculateFinalResults(const std::vector<double> &global_constant_results,
                                                    const std::vector<double> &global_extra_constant_results,
                                                    const int global_ncomps,
                                                    const int global_ntuples,
                                                    std::vector<double> &final_results) = 0;

    // This method is to be implemented in subclasses of this class. In some cases, like 
    // when a rank has no data, or when there are no non-ghosted tuples for a particular
    // avtDataTree leaf, we want to exclude data from being reduced across the avtDataTree
    // or across all the ranks. But excluding isn't always possible or feasible, so instead
    // we can have each subclass provide a value that will not interfere with reductions.
    // For example, the global_sum expression's unused value is "0", meaning that ranks
    // with no data will contribute "0" to the sum.
    virtual double            GetUnusedValue() = 0;

  private:
    void                      IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                   vtkDataArray *ghostZones,
                                                   vtkDataArray *ghostNodes,
                                                   std::vector<int> &nodeShouldBeIgnored);

    void                      DoOperation(vtkDataArray *inputArray,
                                          const int ncomponents,
                                          const int ntuples,
                                          vtkDataSet *in_ds,
                                          std::vector<double> &constant_results,
                                          std::vector<double> &extra_constant_results,
                                          int &num_non_ghosted_tuples);

    void                      DeriveVariable(vtkDataSet *in_ds,
                                             intermediateResults &per_leaf_results);

    int                       ConstantEvaluation(avtDataTree_p inputDataTree,
                                                 std::map<int, intermediateResults> &intermediate_results_map,
                                                 int leaf_number = 0);

    avtDataRepresentation    *WriteData_VTK(avtDataRepresentation *in_dr,
                                            intermediateResults &per_leaf_results,
                                            std::vector<double> &global_constant_results);

    avtDataTree_p             WriteDataTree(avtDataRepresentation *in_dr,
                                            intermediateResults &per_leaf_results,
                                            std::vector<double> &global_constant_results);

    int                       WriteResult(avtDataTree_p inputDataTree, 
                                          avtDataTree_p &outputDataTree,
                                          std::map<int, intermediateResults> &intermediate_results_map,
                                          std::vector<double> &global_constant_results,
                                          int leaf_number = 0);
};


#endif

