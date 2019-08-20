// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               SiloException.h                             //
// ************************************************************************* //

#ifndef SILO_EXCEPTION_H
#define SILO_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: SiloException
//
//  Purpose:
//      The exception thrown when a Silo error occurs.
//
//  Programmer: Hank Childs
//  Creation:   April 9, 2001
//
// ****************************************************************************

class AVTEXCEPTION_API SiloException : public DatabaseException
{
  public:
                          SiloException(const char *);
    virtual              ~SiloException() VISIT_THROW_NOTHING {;};
};


#endif


