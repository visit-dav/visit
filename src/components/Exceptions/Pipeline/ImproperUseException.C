// ************************************************************************* //
//                          ImproperUseException.C                           //
// ************************************************************************* //

#include <ImproperUseException.h>

using std::string;


// ****************************************************************************
//  Method: ImproperUseException constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

ImproperUseException::ImproperUseException(std::string reason)
{
    if (reason == "")
        msg = "The pipeline object is being used improperly.";
    else
        msg = "The pipeline object is being used improperly: " + reason;
}


