// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Sean Ahern, Mon Dec 10 09:53:05 EST 2007
//    Required the expression name.
//
//    Jeremy Meredith, December 18, 2007
//    Added a const char *name constructor to handle the case where the name
//    is NULL.
//
// **************************************************************************** 

class AVTEXCEPTION_API ExpressionException : public PipelineException
{
  public:
                          ExpressionException(std::string name, std::string reason);
                          ExpressionException(const char *name, std::string reason);
    virtual              ~ExpressionException() VISIT_THROW_NOTHING {;};
};



// ****************************************************************************
//  Class: ExpressionParseException
//
//  Purpose:
//      Thrown when an expression fails to parse.
//
//  Programmer: Sean Ahern
//  Creation:   Mon Dec 10 13:13:08 EST 2007
//
//  Modifications:
// **************************************************************************** 

class AVTEXCEPTION_API ExpressionParseException : public PipelineException
{
  public:
                          ExpressionParseException(std::string reason);
    virtual              ~ExpressionParseException() VISIT_THROW_NOTHING {;};
};


#endif


