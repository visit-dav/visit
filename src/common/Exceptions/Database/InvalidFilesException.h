// ************************************************************************* //
//                          InvalidFilesException.h                          //
// ************************************************************************* //

#ifndef INVALID_FILES_EXCEPTION_H
#define INVALID_FILES_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: InvalidFilesException
//
//  Purpose:
//      The exception thrown when invalid files are encountered.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Sep 13 20:01:24 PDT 2000
//    Added a constructor with a file list.
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidFilesException : public DatabaseException
{
  public:
                          InvalidFilesException(const char *);
                          InvalidFilesException(const char * const *, int);
                          InvalidFilesException(int);
    virtual              ~InvalidFilesException() VISIT_THROW_NOTHING {;};
};


#endif


