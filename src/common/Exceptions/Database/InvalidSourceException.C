// ************************************************************************* //
//                           InvalidSourceException.C                        //
// ************************************************************************* //

#include <InvalidSourceException.h>


// ****************************************************************************
//  Method: InvalidSourceException constructor
//
//  Programmer: Hank Childs
//  Creation:   August 10, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug 14 13:54:21 PDT 2000
//    Renamed InvalidSourceException from NoSourceException and changed log
//    message.
//
// ****************************************************************************

InvalidSourceException::InvalidSourceException()
{
    msg = "The source object specified is invalid.";
}


