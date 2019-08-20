// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         InvalidPluginException.h                          //
// ************************************************************************* //

#ifndef INVALID_PLUGIN_EXCEPTION_H
#define INVALID_PLUGIN_EXCEPTION_H
#include <plugin_exports.h>

#include <VisItException.h>

// ****************************************************************************
//  Class: InvalidPluginException
//
//  Purpose:
//      The exception thrown when invalid plugins are encountered.
//
//  Programmer: Jeremy Meredith
//  Creation:   May 10, 2001
//
//  Modifications:
//
// ****************************************************************************

class PLUGIN_API2 InvalidPluginException : public VisItException
{
  public:
                          InvalidPluginException(const char *message,
                                                 const char *name,
                                                 const char *err = NULL);
    virtual              ~InvalidPluginException() VISIT_THROW_NOTHING {;};
};


#endif
