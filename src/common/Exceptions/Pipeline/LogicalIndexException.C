// ************************************************************************* //
//                           LogicalIndexException.C                         //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <LogicalIndexException.h>


// ****************************************************************************
//  Method: LogicalIndexException constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

LogicalIndexException::LogicalIndexException()
{
    char str[1024];
    sprintf(str, "A logical index (i, j, [k]) is expected for this operation");

    msg = str;
}


// ****************************************************************************
//  Method: LogicalIndexException constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

LogicalIndexException::LogicalIndexException(const char *m)
{
    msg = m;
}


