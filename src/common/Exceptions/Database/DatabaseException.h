// ************************************************************************* //
//                              DatabaseException.h                          //
// ************************************************************************* //

#ifndef DATABASE_EXCEPTION_H
#define DATABASE_EXCEPTION_H
#include <avtexception_exports.h>


#include <VisItException.h>


// ****************************************************************************
//  Class: DatabaseException
//
//  Purpose:
//      A base class for all database exceptions.  Meant to make logic easier
//      on catches.
//
//  Programmer: Hank Childs
//  Creation:   August 10, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API DatabaseException : public VisItException
{
  public:
                         ~DatabaseException() VISIT_THROW_NOTHING {;};
};


#endif


