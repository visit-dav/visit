// ************************************************************************* //
//                          InvalidCategoryException.C                       //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <InvalidCategoryException.h>


// ****************************************************************************
//  Method: InvalidCategoryException constructor
//
//  Arguments:
//      name    The category name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

InvalidCategoryException::InvalidCategoryException(const char *name)
{
    char str[1024];
    sprintf(str, "A category not supported by this operation was encountered."
                 "  This operation does not support: %s", name);

    msg = str;
}


