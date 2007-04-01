// ************************************************************************* //
//                       FileDoesNotExistException.h                         //
// ************************************************************************* //

#ifndef FILE_DOES_NOT_EXIST_EXCEPTION_H
#define FILE_DOES_NOT_EXIST_EXCEPTION_H
#include <avtexception_exports.h>
#include <DatabaseException.h>


// ****************************************************************************
//  Class: FileDoesNotExistException
//
//  Purpose:
//      The exception thrown when a file does not exist.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 26 11:15:22 PDT 2002
//
//  Modifications:
//
// ****************************************************************************

class AVTEXCEPTION_API FileDoesNotExistException : public DatabaseException
{
  public:
                          FileDoesNotExistException(const char *);
    virtual              ~FileDoesNotExistException() VISIT_THROW_NOTHING {;};
};


#endif
