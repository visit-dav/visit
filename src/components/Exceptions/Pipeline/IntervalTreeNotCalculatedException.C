// ************************************************************************* //
//                   IntervalTreeNotCalculatedException.C                    //
// ************************************************************************* //

#include <IntervalTreeNotCalculatedException.h>


// ****************************************************************************
//  Method: IntervalTreeNotCalculatedException constructor
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

IntervalTreeNotCalculatedException::IntervalTreeNotCalculatedException()
{
    msg = "The interval tree was accessed even though it has not yet been"
          " calculated";
}


