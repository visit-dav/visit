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
//  Modifications:
//
//    Mark C. Miller, Mon Apr 19 12:00:52 PDT 2004
//    Made it inheret from new base class for PlotterException
//
// ****************************************************************************

InvalidColortableException::InvalidColortableException(const std::string &ct) :
    PlotterException()
{
    msg  = std::string("There is no color table named \"") + ct +
           std::string("\". You need to specify the name of a valid color table.");
}


