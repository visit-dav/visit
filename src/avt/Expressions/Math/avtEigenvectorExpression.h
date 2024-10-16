// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtEigenvectorExpression.h                     //
// ************************************************************************* //

#ifndef AVT_EIGENVECTOR_FILTER_H
#define AVT_EIGENVECTOR_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtEigenvectorExpression
//
//  Purpose:
//      A filter that calculates the eigenvectors of a tensor.
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

class EXPRESSION_API avtEigenvectorExpression : public avtUnaryMathExpression
{
  public:
                              avtEigenvectorExpression();
    virtual                  ~avtEigenvectorExpression();

    virtual const char       *GetType(void)  
                                        { return "avtEigenvectorExpression"; };
    virtual const char       *GetDescription(void) 
                                        { return "Calculating eigenvectors"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int               GetNumberOfComponentsInOutput(int)
                                          { return 9; };
    virtual int               GetVariableDimension(void) { return 9; };
};


#endif


