// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtBinaryPowerExpression.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_POWER_FILTER_H
#define AVT_BINARY_POWER_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryPowerExpression
//
//  Purpose:
//      A filter that calculates the first variable raised to the power of
//      the second filter.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 26 17:05:31 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
// ****************************************************************************

class EXPRESSION_API avtBinaryPowerExpression : public avtBinaryMathExpression
{
  public:
                              avtBinaryPowerExpression();
    virtual                  ~avtBinaryPowerExpression();

    virtual const char       *GetType(void) { return "avtBinaryPowerExpression"; };
    virtual const char       *GetDescription(void)
                                            { return "Calculating power"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual bool     CanHandleSingletonConstants(void) {return true;};
};


#endif


