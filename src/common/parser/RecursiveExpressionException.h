// ************************************************************************* //
//                      RecursiveExpressionException.h                       //
// ************************************************************************* //

#ifndef RECURSIVE_EXPRESSION_EXCEPTION_H
#define RECURSIVE_EXPRESSION_EXCEPTION_H
#include <parser_exports.h>

#include <VisItException.h>


// ****************************************************************************
//  Class: RecursiveExpressionException
//
//  Purpose:
//      Thrown when an expression is infinitely recursive.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 15, 2003
//
//  Modifications:
//
// **************************************************************************** 

class PARSER_API RecursiveExpressionException : public VisItException
{
  public:
                          RecursiveExpressionException(std::string);
    virtual              ~RecursiveExpressionException() VISIT_THROW_NOTHING {;};
};


#endif


