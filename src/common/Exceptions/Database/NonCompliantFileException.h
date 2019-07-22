// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        NonCompliantFileException.h                        //
// ************************************************************************* //

#ifndef NON_COMPLIANT_FILE_EXCEPTION_H
#define NON_COMPLIANT_FILE_EXCEPTION_H

#include <avtexception_exports.h>

#include <DatabaseException.h>


// ****************************************************************************
//  Class: NonCompliantFileException
//
//  Purpose:
//      The exception thrown when the database type is correct, but the file
//      is not compliant with that type.
//
//  Programmer: Hank Childs
//  Creation:   May 20, 2010
//
// ****************************************************************************

class AVTEXCEPTION_API NonCompliantFileException : public DatabaseException
{
  public:
                          NonCompliantFileException(const char *, std::string);
    virtual              ~NonCompliantFileException() VISIT_THROW_NOTHING {;};
};


#endif


