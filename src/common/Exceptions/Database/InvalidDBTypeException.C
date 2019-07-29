// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            InvalidDBTypeException.C                       //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <InvalidDBTypeException.h>

// ****************************************************************************
//  Method: InvalidDBTypeException constructor
//
//  Arguments:
//      str     A specific message regarding why the database type is invalid.
//
//  Programmer: Hank Childs
//  Creation:   October 14, 2002
//
// ****************************************************************************

InvalidDBTypeException::InvalidDBTypeException(const char *str)
{
    char t_str[1024];
    sprintf(t_str, "The selected database type was not correct for the given "
                   "file.  The message from the database was %s", str);

    msg = t_str;
}


