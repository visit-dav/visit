// ************************************************************************* //
//                             NoInputException.C                            //
// ************************************************************************* //

#include <NoInputException.h>


// ****************************************************************************
//  Method: NoInputException constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

NoInputException::NoInputException()
{
    msg = "No input was defined for this filter.";
}


