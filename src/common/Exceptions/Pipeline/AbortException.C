// ************************************************************************* //
//                              AbortException.C                             //
// ************************************************************************* //

#include <AbortException.h>


// ****************************************************************************
//  Method: AbortException constructor
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
// ****************************************************************************

AbortException::AbortException()
{
    msg = "The pipeline was ordered to abort.";
}


