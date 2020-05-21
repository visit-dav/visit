// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             TyphonIOException.h                           //
// ************************************************************************* //

#ifndef TYPHONIO_EXCEPTION_H
#define TYPHONIO_EXCEPTION_H
#include <avtexception_exports.h>


#include <typhonio.h>

#include <DatabaseException.h>


// ****************************************************************************
//  Class: TyphonIOException
//
//  Purpose:
//      The exception thrown when a TyphonIO error occurs.
//
//  Programmer: Paul Selby
//  Creation:   March 16, 2015
//
// ****************************************************************************

class AVTEXCEPTION_API TyphonIOException : public DatabaseException
{
  public:
                          TyphonIOException(TIO_t);
    virtual              ~TyphonIOException() VISIT_THROW_NOTHING {;};
};


#endif


