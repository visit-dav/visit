// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtGlobalConstantExpression.h                       //
// ************************************************************************* //

#ifndef AVT_GLOBAL_CONSTANT_EXPRESSION_H
#define AVT_GLOBAL_CONSTANT_EXPRESSION_H

#include <avtExpressionFilter.h>

// #include <vector>

class     vtkDataArray;
class     vtkDataSet;
class     intermediateResults;

// ****************************************************************************
//  Class: avtGlobalConstantExpression
//
//  Purpose:
//      TODO
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

    // TODO do I need these?
    virtual int               NumVariableArguments() { return 1; };
    virtual int               GetVariableDimension() { return 1; };

  protected:
    int                       currentProgress;
    int                       totalSteps;
    avtCentering              centering;

    virtual void              Execute(void);

    // TODO explanation
    virtual bool              NeedsNTuples() { return false; };

    // TODO explanation
    virtual bool              NeedsSums() { return false; };

    // TODO explanation
    virtual bool              NeedsIntermediateData() { return true; };

    // expressions like standard deviation and variance require an extra array of intermediate data
    virtual bool              NeedsExtraIntermediateData() { return false; };

    virtual void              CalculateWithoutGhosts(vtkDataArray *in,
                                                     const int ncomponents,
                                                     const int ntuples,
                                                     std::vector<double> &constant_results,
                                                     std::vector<double> &extra_constant_results,
                                                     std::vector<double> &sums) = 0;

    virtual void              CalculateWithGhosts(vtkDataArray *in,
                                                  const int ncomponents,
                                                  const int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr,
                                                  std::vector<double> &constant_results,
                                                  std::vector<double> &extra_constant_results,
                                                  std::vector<double> &sums) = 0;

    virtual double            LocalIntermediateReduction(const double running_reduction,
                                                         const double intermediate_value) = 0;

    virtual void              GlobalIntermediateReduction(std::vector<double> &local_constant_results,
                                                          std::vector<double> &global_constant_results,
                                                          const int ncomps) = 0;

    virtual void              CalculateFinalResults(const std::vector<double> &global_constant_results,
                                                    const std::vector<double> &global_extra_constant_results,
                                                    const std::vector<double> &global_component_sums,
                                                    const int global_ncomps,
                                                    const int global_ntuples,
                                                    std::vector<double> &final_results) = 0;

    virtual double            GetUnusedValue() { return 0.0; };

  private:
    int                       GetLocalNumTuples(const std::map<int, intermediateResults> &intermediate_results_map);

    std::vector<int>          IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                   vtkDataArray *ghostZones,
                                                   vtkDataArray *ghostNodes,
                                                   std::vector<int> &nodeShouldBeIgnored);

    void                      DoOperation(vtkDataArray *inputArray,
                                          const int ncomponents,
                                          const int ntuples,
                                          vtkDataSet *in_ds,
                                          std::vector<double> &constant_results,
                                          std::vector<double> &extra_constant_results,
                                          std::vector<double> &sums,
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

