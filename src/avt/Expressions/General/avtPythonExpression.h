// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtPythonExpression.h                            //
// ************************************************************************* //

#ifndef AVT_PYTHON_EXPRESSION_H
#define AVT_PYTHON_EXPRESSION_H

#include <avtExpressionFilter.h>

class avtPythonFilterEnvironment;

// ****************************************************************************
//  Class: avtPythonExpression
//
//  Purpose:
//      Interface to python expressions.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Feb  2 13:45:21 PST 2010
//
//  Modifications:
//   Cyrus Harrison, Tue Jan 11 16:33:27 PST 2011
//   Added exprType & exprDescription members.
//
// ****************************************************************************

class EXPRESSION_API avtPythonExpression : public avtExpressionFilter
{
  public:
                              avtPythonExpression();
    virtual                  ~avtPythonExpression();
    void                      CleanUp();

    virtual const char       *GetType(void);
    virtual const char       *GetDescription(void);

    virtual int               NumVariableArguments(void);
    virtual int               GetVariableDimension(void);
    virtual bool              IsPointVariable(void);

    virtual void              ProcessArguments(ArgsExpr *, ExprPipelineState *);
    virtual void              AddInputVariableName(const char *);


  protected:
    // Keep track of this filter's input variables
    std::vector<char *>       varnames;

    virtual void              Execute(void);
    virtual avtContract_p     ModifyContract(avtContract_p);


  private:
    avtPythonFilterEnvironment *pyEnv;
    std::string                 pyScript;

    std::string                 exprType;
    std::string                 exprDescription;

};


#endif


