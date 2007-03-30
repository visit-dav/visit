// ************************************************************************* //
//                          InvalidDirectoryException.h                          //
// ************************************************************************* //

#ifndef INVALID_DIRECTORY_EXCEPTION_H
#define INVALID_DIRECTORY_EXCEPTION_H
#include <plugin_exports.h>

#include <VisItException.h>

// ****************************************************************************
//  Class: InvalidDirectoryException
//
//  Purpose:
//      The exception thrown when invalid directories are encountered.
//
//  Programmer: Jeremy Meredith
//  Creation:   May 10, 2001
//
//  Modifications:
//
// ****************************************************************************

class PLUGIN_API InvalidDirectoryException : public VisItException
{
  public:
                          InvalidDirectoryException(const char *);
    virtual              ~InvalidDirectoryException() VISIT_THROW_NOTHING {;};
};


#endif


