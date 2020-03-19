// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          BadPermissionException.h                         //
// ************************************************************************* //

#ifndef BAD_PERMISSION_EXCEPTION_H
#define BAD_PERMISSION_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: BadPermissionException
//
//  Purpose:
//      The exception thrown when a file with no permissions is encountered.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2001
//
// ****************************************************************************

class AVTEXCEPTION_API BadPermissionException : public DatabaseException
{
  public:
                          BadPermissionException(const char *);
    virtual              ~BadPermissionException() VISIT_THROW_NOTHING {;};
};


#endif


