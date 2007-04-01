// ************************************************************************* //
//                            InvalidSetException.C                          //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <InvalidSetException.h>


// ****************************************************************************
//  Method: InvalidSetException constructor
//
//  Arguments:
//      catName    The category name.
//      setName    The set name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

InvalidSetException::InvalidSetException(const char *catName, const char *setName)
{
    char str[1024];
    sprintf(str, "Category %s does not have a set named %s", catName, setName);

    msg = str;
}

// ****************************************************************************
//  Method: InvalidSetException constructor
//
//  Arguments:
//    setName    The set name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 26, 2004 
//
// ****************************************************************************

InvalidSetException::InvalidSetException(const char *setName)
{
    char str[1024];
    sprintf(str, "The set %s has been turned OFF and cannot be used.", setName);

    msg = str;
}


