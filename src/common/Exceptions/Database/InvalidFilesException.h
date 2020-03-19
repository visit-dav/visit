// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          InvalidFilesException.h                          //
// ************************************************************************* //

#ifndef INVALID_FILES_EXCEPTION_H
#define INVALID_FILES_EXCEPTION_H

#include <avtexception_exports.h>

#include <DatabaseException.h>

#include <string>
#include <vector>


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
//    Jeremy Meredith, Wed Aug 11 16:27:32 PDT 2004
//    Adding a new constructor that allows a more informative message.
//
//    Hank Childs, Thu Jan 11 15:49:54 PST 2007
//    Add a new constructor for the plugin types tried.
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidFilesException : public DatabaseException
{
  public:
                          InvalidFilesException(const char *);
                          InvalidFilesException(const std::string &);
                          InvalidFilesException(const char *, 
                                                std::vector<std::string> &);
                          InvalidFilesException(const char *, 
                                                std::vector<std::string> &,
                                                const char *);
                          InvalidFilesException(const char * const *, int);
                          InvalidFilesException(const std::string&,
                                                           const std::string&);
                          InvalidFilesException(int);
    virtual              ~InvalidFilesException() VISIT_THROW_NOTHING {;};
};


#endif


