// ************************************************************************* //
//                              QtVisWindow.h                                //
// ************************************************************************* //

#ifndef QT_VIS_WINDOW_H
#define QT_VIS_WINDOW_H
#include <qtviswindow_exports.h>

#include <VisWindow.h>


// ****************************************************************************
//  Class: QtVisWindow
//
//  Purpose:
//      A vis window that uses Qt to do its windowing.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Mar 24 12:23:47 PDT 2004
//    I made it build on Windows.
//
// ****************************************************************************

class QTVISWINDOW_API QtVisWindow : public VisWindow
{
  public:
    QtVisWindow();
};


#endif


