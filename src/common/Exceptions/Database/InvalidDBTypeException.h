// ************************************************************************* //
//                          InvalidDBTypeException.h                          //
// ************************************************************************* //

#ifndef INVALID_DB_TYPE_EXCEPTION_H
#define INVALID_DB_TYPE_EXCEPTION_H

#include <avtexception_exports.h>

#include <DatabaseException.h>


// ****************************************************************************
//  Class: InvalidDBTypeException
//
//  Purpose:
//      The exception thrown when the database type chosen was not correct.
//
//  Programmer: Hank Childs
//  Creation:   October 14, 2002
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidDBTypeException : public DatabaseException
{
  public:
                          InvalidDBTypeException(const char *);
    virtual              ~InvalidDBTypeException() VISIT_THROW_NOTHING {;};
};


#endif


