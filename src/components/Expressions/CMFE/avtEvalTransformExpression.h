// ************************************************************************* //
//                       avtEvalTransformExpression.h                        //
// ************************************************************************* //

#ifndef AVT_EVAL_TRANSFORM_EXPRESSION_H
#define AVT_EVAL_TRANSFORM_EXPRESSION_H

#include <avtSymmEvalExpression.h>


// ****************************************************************************
//  Class: avtEvalTransformExpression
//
//  Purpose:
//      Creates an expression that evaluates values after a transformation.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtEvalTransformExpression : public avtSymmEvalExpression
{
  public:
                              avtEvalTransformExpression();
    virtual                  ~avtEvalTransformExpression();

  protected:
    virtual int               GetNumberOfInputParameters(void) { return 9; };
    virtual avtDataObject_p   TransformData(avtDataObject_p);
};


#endif


