// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtBinaryAddExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_ADD_FILTER_H
#define AVT_BINARY_ADD_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryAddExpression
//
//  Purpose:
//      A filter that calculates the sum of its two inputs.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sun Jan 13 20:07:56 PST 2008
//    Allow constants to be created as singletons.
//
//    Jeremy Meredith, Wed Mar 18 14:27:33 EDT 2009
//    Added better support for array vars by returning a real
//    output variable type (when possible).
//
// ****************************************************************************

class EXPRESSION_API avtBinaryAddExpression : public avtBinaryMathExpression
{
  public:
                              avtBinaryAddExpression();
    virtual                  ~avtBinaryAddExpression();

    virtual const char       *GetType(void)   { return "avtBinaryAddExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating binary addition"; };

  protected:
    virtual avtVarType        GetVariableType();
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual bool     CanHandleSingletonConstants(void) {return true;};
};


#endif


