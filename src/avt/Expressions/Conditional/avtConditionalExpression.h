// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtConditionalExpression.h                         //
// ************************************************************************* //

#ifndef AVT_CONDITIONAL_FILTER_H
#define AVT_CONDITIONAL_FILTER_H

#include <avtMultipleInputExpressionFilter.h>


// ****************************************************************************
//  Class: avtConditionalExpression
//
//  Purpose:
//      Performs an if-then-else test.  The first argument is the conditional
//      for the 'if' and the second two are the 'then' and 'else' clauses.
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
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    Simple conditionals can handle these with no modifications.
//
// ****************************************************************************

class EXPRESSION_API avtConditionalExpression 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtConditionalExpression();
    virtual                  ~avtConditionalExpression();

    virtual const char       *GetType(void)  
                                    { return "avtConditionalExpression"; };
    virtual const char       *GetDescription(void)
                                    { return "Doing if-then-else"; };
    virtual int               NumVariableArguments() { return 3; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);

    virtual bool              FilterUnderstandsTransformedRectMesh();
};


#endif


