// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           BadPermissionException.C                        //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <BadPermissionException.h>


// ****************************************************************************
//  Method: BadPermissionException constructor
//
//  Arguments:
//      filename    The name of a file we don't have permissions to.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2001
//
// ****************************************************************************

BadPermissionException::BadPermissionException(const char *filename)
{
    char str[1024];
    sprintf(str, "No read permissions for %s.", filename);

    msg = str;
}


