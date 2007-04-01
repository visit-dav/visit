// ************************************************************************* //
//                               QtVisWindow.C                               //
// ************************************************************************* //

#include <QtVisWindow.h>

#include <VisWinRenderingWithWindow.h>


// ****************************************************************************
//  Method: QtVisWindow constructor
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2004
//
// ****************************************************************************

QtVisWindow::QtVisWindow() : VisWindow(false)
{
    Initialize(new VisWinRenderingWithWindow(colleagueProxy));
}


