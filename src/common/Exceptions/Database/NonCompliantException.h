// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        NonCompliantException.h                            //
// ************************************************************************* //

#ifndef NON_COMPLIANT_EXCEPTION_H
#define NON_COMPLIANT_EXCEPTION_H

#include <avtexception_exports.h>

#include <DatabaseException.h>


// ****************************************************************************
//  Class: NonCompliantException
//
//  Purpose:
//      The generic exception thrown when something is not compliant.
//
//  Programmer: Hank Childs
//  Creation:   May 20, 2010
//
// ****************************************************************************

class AVTEXCEPTION_API NonCompliantException : public DatabaseException
{
  public:
                          NonCompliantException(const char *, std::string);
    virtual              ~NonCompliantException() VISIT_THROW_NOTHING {;};
};


#endif


