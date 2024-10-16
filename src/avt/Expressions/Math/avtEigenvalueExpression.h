// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtEigenvalueExpression.h                      //
// ************************************************************************* //

#ifndef AVT_EIGENVALUE_FILTER_H
#define AVT_EIGENVALUE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtEigenvalueExpression
//
//  Purpose:
//      A filter that calculates the eigenvalue of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtEigenvalueExpression : public avtUnaryMathExpression
{
  public:
                              avtEigenvalueExpression();
    virtual                  ~avtEigenvalueExpression();

    virtual const char       *GetType(void)  
                                         { return "avtEigenvalueExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating eigenvalues"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int               GetNumberOfComponentsInOutput(int)
                                          { return 3; };
    virtual int               GetVariableDimension(void)  { return 3; };
};


#endif


