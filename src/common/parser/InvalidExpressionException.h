// ************************************************************************* //
//                        InvalidExpressionException.h                       //
// ************************************************************************* //

#ifndef INVALID_EXPRESSION_EXCEPTION_H
#define INVALID_EXPRESSION_EXCEPTION_H
#include <parser_exports.h>

#include <VisItException.h>


// ****************************************************************************
//  Class: InvalidExpressionException
//
//  Purpose:
//      Thrown when an expression is ill-formed and cannot be parsed.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 15, 2003
//
//  Modifications:
//
// **************************************************************************** 

class PARSER_API InvalidExpressionException : public VisItException
{
  public:
                          InvalidExpressionException(std::string);
    virtual              ~InvalidExpressionException() VISIT_THROW_NOTHING {;};
};


#endif


