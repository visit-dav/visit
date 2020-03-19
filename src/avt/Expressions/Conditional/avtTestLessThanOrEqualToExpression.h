// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtTestLessThanOrEqualToExpression.h                    //
// ************************************************************************* //

#ifndef AVT_TEST_LESS_THAN_OR_EQUAL_TO_FILTER_H
#define AVT_TEST_LESS_THAN_OR_EQUAL_TO_FILTER_H

#include <avtComparisonExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTestLessThanOrEqualToExpression
//
//  Purpose:
//      A filter that determines when one input is less than or equal to
//      another.
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

class EXPRESSION_API avtTestLessThanOrEqualToExpression 
    : public avtComparisonExpression
{
  public:
                              avtTestLessThanOrEqualToExpression();
    virtual                  ~avtTestLessThanOrEqualToExpression();

    virtual const char       *GetType(void)
                               { return "avtTestLessThanOrEqualToExpression"; };
    virtual const char       *GetDescription(void) 
                               { return "Testing <="; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual bool     CanHandleSingletonConstants(void) {return true;};
};


#endif


