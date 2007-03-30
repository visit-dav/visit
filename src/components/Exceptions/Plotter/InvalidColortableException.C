// ************************************************************************* //
//                            InvalidColortableException.C                   //
// ************************************************************************* //

#include <InvalidColortableException.h>

// ****************************************************************************
//  Method: InvalidColortableException constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Apr 25 10:24:10 PDT 2003
//
// ****************************************************************************

InvalidColortableException::InvalidColortableException(const std::string &ct) :
    VisItException()
{
    msg  = std::string("There is no color table named \"") + ct +
           std::string("\". You need to specify the name of a valid color table.");
}


