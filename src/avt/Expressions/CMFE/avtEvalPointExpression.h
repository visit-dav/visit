// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtEvalPointExpression.h                        //
// ************************************************************************* //

#ifndef AVT_EVAL_POINT_EXPRESSION_H
#define AVT_EVAL_POINT_EXPRESSION_H

#include <avtSymmEvalExpression.h>


// ****************************************************************************
//  Class: avtEvalPointExpression
//
//  Purpose:
//      Creates an expression that evaluates values after a transformation.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtEvalPointExpression : public avtSymmEvalExpression
{
  public:
                              avtEvalPointExpression();
    virtual                  ~avtEvalPointExpression();

  protected:
    virtual int               GetNumberOfInputParameters(void) { return 3; };
    virtual avtDataObject_p   TransformData(avtDataObject_p);
};


#endif


