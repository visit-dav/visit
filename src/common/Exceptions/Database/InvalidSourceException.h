// ************************************************************************* //
//                          InvalidSourceException.h                         //
// ************************************************************************* //

#ifndef INVALID_SOURCE_EXCEPTION_H
#define INVALID_SOURCE_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: InvalidSourceException
//
//  Purpose:
//      The exception thrown when the database does not recognize a source
//      object.
//
//  Programmer: Hank Childs
//  Creation:   August 10, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug 14 13:54:21 PDT 2000
//    Renamed InvalidSourceException from NoSourceException.
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidSourceException : public DatabaseException
{
  public:
                          InvalidSourceException();
    virtual              ~InvalidSourceException() VISIT_THROW_NOTHING {;};
};


#endif


