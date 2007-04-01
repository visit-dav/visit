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


