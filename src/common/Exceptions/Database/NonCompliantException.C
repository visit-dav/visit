// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         NonCompliantException.C                           //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <NonCompliantException.h>

// ****************************************************************************
//  Method: NonCompliantException constructor
//
//  Arguments:
//      err     A specific message regarding why something is noncompliant.
//
//  Programmer: Hank Childs
//  Creation:   May 20, 2010
//
// ****************************************************************************

NonCompliantException::NonCompliantException(const char * opp,
                                             std::string err)
{
    char t_str[1024];
    sprintf(t_str, "\nVisIt has encountered an error while performing "
                   "the following operation: \"%s\".\n"
                   "\nThe description of the error is:\n \"%s\".\n",
            opp, err.c_str());

    msg = t_str;
}


