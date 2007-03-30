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
