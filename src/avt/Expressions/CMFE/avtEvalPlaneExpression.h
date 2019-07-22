// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtEvalPlaneExpression.h                        //
// ************************************************************************* //

#ifndef AVT_EVAL_PLANE_EXPRESSION_H
#define AVT_EVAL_PLANE_EXPRESSION_H

#include <avtSymmEvalExpression.h>


// ****************************************************************************
//  Class: avtEvalPlaneExpression
//
//  Purpose:
//      Creates an expression that evaluates values after a transformation.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtEvalPlaneExpression : public avtSymmEvalExpression
{
  public:
                              avtEvalPlaneExpression();
    virtual                  ~avtEvalPlaneExpression();

  protected:
    virtual int               GetNumberOfInputParameters(void) { return 6; };
    virtual avtDataObject_p   TransformData(avtDataObject_p);
};


#endif


