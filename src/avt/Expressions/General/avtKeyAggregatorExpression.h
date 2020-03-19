// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtKeyAggregatorExpression.h                      //
// ************************************************************************* //

#ifndef AVT_KEY_AGGREGATOR_EXPRESSION_H
#define AVT_KEY_AGGREGATOR_EXPRESSION_H

#include <avtExpressionFilter.h>

#include <string>
#include <vector>

class vtkDataArray;
class vtkDoubleArray;

// ****************************************************************************
//  Class: avtKeyAggregatorExpression
//
//  Purpose:
//      This expression allows an user to aggregate values by a set of 
//      contigious interger keys. The keys and values are cell fields passed 
//      to the expression. 
//
//
//  Programmer: Cyrus Harrison
//  Creation:   March 26, 2009
//
//  Modifications:
//   Cyrus Harrison, Thu Oct 29 10:26:42 PDT 2009
//   Switch to generic use of vtkDataArray to support non integer key types.
//
// ****************************************************************************

class EXPRESSION_API avtKeyAggregatorExpression : public avtExpressionFilter
{
  public:
                              avtKeyAggregatorExpression();
    virtual                  ~avtKeyAggregatorExpression();

    virtual void              AddInputVariableName(const char *var);

    virtual const char       *GetType(void)
                                     { return "avtKeyAggregatorExpression"; };
    virtual const char       *GetDescription(void)
                                      {return "Aggregating values per key.";};


    virtual int               NumVariableArguments() { return 2; };
    virtual int               GetVariableDimension();

    virtual bool              IsPointVariable(void) { return false; }

  protected:
    virtual void              Execute(void);

  private:
    int                       FindMaxKey(std::vector<vtkDataArray*> &key_arrays);

    void                      Aggregate(std::vector<vtkDataArray*> &key_arrays,
                                        std::vector<vtkDataArray*> &val_arrays,
                                        int num_keys, int num_val_comps,
                                        std::vector<double> &key_results);

    vtkDoubleArray            *CreateResultArray(vtkDataArray *keys,
                                              std::vector<double> &key_results,
                                              int num_val_comps);

    std::vector<std::string>  inputVarNames;
};


#endif


