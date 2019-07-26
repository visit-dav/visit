// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtApplyEnumerationExpression.h                     //
// ************************************************************************* //

#ifndef AVT_APPLY_ENUMERATION_FILTER_H
#define AVT_APPLY_ENUMERATION_FILTER_H

#include <avtMultipleInputExpressionFilter.h>

#include <vector>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;


// ****************************************************************************
//  Class: avtApplyEnumerationExpression
//
//  Purpose:
//      Map the nonnegative integers to other values.
//          
//  Programmer: Jeremy Meredith
//  Creation:   February 13, 2008
//
// ****************************************************************************

class EXPRESSION_API avtApplyEnumerationExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtApplyEnumerationExpression();
    virtual                  ~avtApplyEnumerationExpression();

    virtual const char       *GetType(void) 
                                 { return "avtApplyEnumerationExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Applying an enumeration"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return 1; };

  protected:
    std::vector<double>       enumeratedValues;

    virtual void              UpdateDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual avtVarType        GetVariableType(void) { return AVT_SCALAR_VAR; };
};


#endif


