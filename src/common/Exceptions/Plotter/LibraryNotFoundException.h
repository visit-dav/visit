// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          LibraryNotFoundException.h                       //
// ************************************************************************* //

#ifndef LIBRARY_NOT_FOUND_EXCEPTION_H
#define LIBRARY_NOT_FOUND_EXCEPTION_H
#include <avtexception_exports.h>

#include <PlotterException.h>

// ****************************************************************************
//  Class: LibraryNotFoundException
//
//  Purpose:
//      The exception thrown when a required library was not found.
//
//  Programmer: Tom Fogal
//  Creation:   Tue Dec  8 14:12:00 MST 2009
//
//  Modifications:
//
// ****************************************************************************

class AVTEXCEPTION_API LibraryNotFoundException : public PlotterException
{
  public:
                  LibraryNotFoundException(const std::string &);
    virtual      ~LibraryNotFoundException() VISIT_THROW_NOTHING {;};
};
#endif
