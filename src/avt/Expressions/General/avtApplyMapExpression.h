// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtApplyMapExpression.h                            //
// ************************************************************************* //

#ifndef AVT_APPLY_MAP_EXPRESSION_H
#define AVT_APPLY_MAP_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

#include <vector>
#include <map>


class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ExprParseTreeNode;
class     ListExpr;

// ****************************************************************************
//  Class: avtApplyMapExpression
//
//  Purpose:
//      Map a set of numeric values to string labels, or another set of 
//      numeric values.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thursday August 07, 2009
//
// ****************************************************************************

class EXPRESSION_API avtApplyMapExpression
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtApplyMapExpression();
    virtual                  ~avtApplyMapExpression();

    virtual const char       *GetType(void) 
                                 { return "avtApplyMapExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Applying map expression"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return 1; };

  protected:
    virtual vtkDataArray         *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType            GetVariableType(void) { return AVT_SCALAR_VAR;};
    virtual void                  UpdateDataObjectInfo();
  private:
    int                           mapType;
    std::map<double,double>       numericMap;
    double                        numericDefault;

    std::map<double,std::string>  stringMap;
    std::string                   stringDefault;
    int                           maxStringLength;

    void                          BuildMap(ListExpr *to_list);
    void                          BuildMap(ListExpr *to_list,
                                           const std::vector<double> &from_values);

    void                          SetMapDefaultValues(ExprParseTreeNode *node);
    void                          ThrowError(const std::string &msg);
};


#endif


