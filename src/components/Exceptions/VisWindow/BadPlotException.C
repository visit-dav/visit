// ************************************************************************* //
//                            BadPlotException.C                             //
// ************************************************************************* //

#include <BadPlotException.h>


// ****************************************************************************
//  Method: BadPlotException constructor
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2000
//
// ****************************************************************************

BadPlotException::BadPlotException()
{
    msg  = "Tried to access non-existant plot.";
}


