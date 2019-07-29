// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            InvalidDirectoryException.C                        //
// ************************************************************************* //

#include <stdio.h>
#include <InvalidDirectoryException.h>

// ****************************************************************************
//  Method: InvalidDirectoryException constructor
//
//  Arguments:
//      dirname    the directory name
//
//  Programmer: Jeremy Meredith
//  Creation:   May 10, 2001
//
// ****************************************************************************

InvalidDirectoryException::InvalidDirectoryException(const char *dirname)
{
    char str[1024];
    sprintf(str, "The directory %s is invalid.", dirname);

    msg = str;
}
