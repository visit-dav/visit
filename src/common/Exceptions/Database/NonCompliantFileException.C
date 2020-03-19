// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         NonCompliantFileException.C                       //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <NonCompliantFileException.h>

// ****************************************************************************
//  Method: NonCompliantFileException constructor
//
//  Arguments:
//      dbtype  The type of file.
//      err     A specific message regarding why the file is noncompliant.
//
//  Programmer: Hank Childs
//  Creation:   May 20, 2010
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 29 11:15:03 EDT 2011
//    Simplified the error message since it no longer aborts the file
//    opening process.  (Plus, more detailed help instructions are given
//    in the higher-level messages.)
//
// ****************************************************************************

NonCompliantFileException::NonCompliantFileException(const char *dbtype,
                                                     std::string err)
{
    char t_str[1024];
    sprintf(t_str, "When attempting to use \"%s\", the file "
            "matched the general format expected by the reader but "
            "eventually encountered this error: \"%s\".",
            dbtype, err.c_str());

    msg = t_str;
}


