// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtTestEqualToExpression.h                        //
// ************************************************************************* //

#ifndef AVT_TEST_EQUAL_TO_FILTER_H
#define AVT_TEST_EQUAL_TO_FILTER_H

#include <avtComparisonExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestEqualToExpression
//
//  Purpose:
//      A filter that determines when two inputs are equal.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Jan 14 20:01:04 PST 2008
//    Allow constants to be created as singletons.
//
// ****************************************************************************

class EXPRESSION_API avtTestEqualToExpression : public avtComparisonExpression
{
  public:
                              avtTestEqualToExpression();
    virtual                  ~avtTestEqualToExpression();

    virtual const char       *GetType(void)
                                  { return "avtTestEqualToExpression"; };
    virtual const char       *GetDescription(void) 
                                  { return "Testing for equality"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual bool     CanHandleSingletonConstants(void) {return true;};
};


#endif


