// ************************************************************************* //
//                         InvalidVariableException.h                        //
// ************************************************************************* //

#ifndef INVALID_VARIABLE_EXCEPTION_H
#define INVALID_VARIABLE_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: InvalidVariableException
//
//  Purpose:
//      The exception thrown when an invalid variable is requested from a
//      database.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidVariableException : public DatabaseException
{
  public:
                          InvalidVariableException(std::string);
    virtual              ~InvalidVariableException() VISIT_THROW_NOTHING {;};
};


#endif


