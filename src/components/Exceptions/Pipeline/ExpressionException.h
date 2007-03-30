// ************************************************************************* //
//                           ExpressionException.h                           //
// ************************************************************************* //

#ifndef EXPRESSION_EXCEPTION_H
#define EXPRESSION_EXCEPTION_H
#include <avtexception_exports.h>

#include <PipelineException.h>


// ****************************************************************************
//  Class: ExpressionException
//
//  Purpose:
//      Thrown when an expression fails to evaluate or an expression filter
//      fails.  Example: taking the log of a negative number or trying to
//      add variables that don't conform to the same mesh.
//
//
//  Programmer: Sean Ahern
//  Creation:   Fri Mar 22 13:21:20 PST 2002
//
//  Modifications:
//    Brad Whitlock, Fri Jun 28 13:26:48 PST 2002
//    Added windows api.
//
// **************************************************************************** 

class AVTEXCEPTION_API ExpressionException : public PipelineException
{
  public:
                          ExpressionException(std::string);
    virtual              ~ExpressionException() VISIT_THROW_NOTHING {;};
};


#endif


