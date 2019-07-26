// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtComparisonExpression.h                       //
// ************************************************************************* //

#ifndef AVT_COMPARISON_FILTER_H
#define AVT_COMPARISON_FILTER_H

#include <avtBinaryMathExpression.h>

#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Class: avtComparisonExpression
//
//  Purpose:
//      This is the base class for any expression that compares two arrays
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

class avtComparisonExpression : public avtBinaryMathExpression
{
  public:
                           avtComparisonExpression();
    virtual               ~avtComparisonExpression();

  protected:
    virtual int            GetNumberOfComponentsInOutput(int, int) { return 1; };
    virtual vtkDataArray  *CreateArray(vtkDataArray *, vtkDataArray*)
                                { return vtkUnsignedCharArray::New(); };
};


#endif


