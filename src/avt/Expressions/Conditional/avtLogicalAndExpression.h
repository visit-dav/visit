// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtLogicalAndExpression.h                         //
// ************************************************************************* //

#ifndef AVT_LOGICAL_AND_FILTER_H
#define AVT_LOGICAL_AND_FILTER_H

#include <avtComparisonExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtLogicalAndExpression
//
//  Purpose:
//      A filter that finds the logical and of two inputs.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtLogicalAndExpression : public avtComparisonExpression
{
  public:
                              avtLogicalAndExpression();
    virtual                  ~avtLogicalAndExpression();

    virtual const char       *GetType(void)
                                  { return "avtLogicalAndExpression"; };
    virtual const char       *GetDescription(void) 
                                  { return "Performing Logical \'And\'"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


