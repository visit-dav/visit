// ************************************************************************* //
//                              BadIndexException.C                          //
// ************************************************************************* //

#include <BadIndexException.h>

// for sprintf
#include <stdio.h>


// ****************************************************************************
//  Method: BadIndexException constructor
//
//  Arguments:
//      index        The domain number that was out of range.
//      numIndices   The total number of domains.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Sep  2 15:09:42 PDT 2002
//    Made the exception be a little more readable.
//
// ****************************************************************************

BadIndexException::BadIndexException(int index, int numIndices)
{
    char str[1024];
    sprintf(str, "Tried to access an invalid index %d (Maximum = %d).", index,
            numIndices-1);
    msg = str;
}


