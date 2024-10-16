// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIdentityExpression.h                      //
// ************************************************************************* //

#ifndef AVT_IDENTITY_FILTER_H
#define AVT_IDENTITY_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtIdentityExpression
//
//  Purpose:
//      A filter that takes the identity of a variable.
//
//  Programmer: Hank Childs
//  Creation:   December 9, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs and Kathleen Bonnell, Fri Apr 24 17:18:47 PDT 2009
//    Fix problem with identity variables of meshes.
//
// ****************************************************************************

class EXPRESSION_API avtIdentityExpression : public avtUnaryMathExpression
{
  public:
                              avtIdentityExpression();
    virtual                  ~avtIdentityExpression();

    virtual const char       *GetType(void)  { return "avtIdentityExpression"; }
    virtual const char       *GetDescription(void) 
                                    { return "Applying the identity filter"; }

    // "Tolerated" may be a better phrase than "Expected" here.
    virtual bool              NullInputIsExpected(void) { return true; }

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


