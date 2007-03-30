// ************************************************************************* //
//                           FileDoesNotExistException.C                     //
// ************************************************************************* //

#include <stdio.h>                  // for sprintf
#include <FileDoesNotExistException.h>


// ****************************************************************************
//  Method: FileDoesNotExistException constructor
//
//  Arguments:
//      filename    The name of a file that does not exist.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 26 11:16:13 PDT 2002
//
// ****************************************************************************

FileDoesNotExistException::FileDoesNotExistException(const char *filename)
{
    char str[1024];
    sprintf(str, "The file \"%s\" does not exist.", filename);

    msg = str;
}
