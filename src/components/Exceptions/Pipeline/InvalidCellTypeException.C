// ************************************************************************* //
//                          InvalidCellTypeException.C                       //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <InvalidCellTypeException.h>


// ****************************************************************************
//  Method: InvalidCellTypeException constructor
//
//  Arguments:
//      ct      The bad cell type.
//
//  Programmer: Hank Childs
//  Creation:   December 12, 2000
//
// ****************************************************************************

InvalidCellTypeException::InvalidCellTypeException(int ct)
{
    char str[1024];
    sprintf(str, "Encountered bad cell type = %d", ct);
    msg = str;
}


// ****************************************************************************
//  Method: InvalidCellTypeException constructor
//
//  Arguments:
//      name    The cell type name (or family).
//
//  Programmer: Hank Childs
//  Creation:   April 15, 2002
//
// ****************************************************************************

InvalidCellTypeException::InvalidCellTypeException(const char *name)
{
    char str[1024];
    sprintf(str, "A primitive not supported by this operation was encountered."
                 "  This operation does not support: %s", name);

    msg = str;
}


