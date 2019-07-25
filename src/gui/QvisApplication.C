// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisApplication.h>

#include <QMenuBar>

#if defined(Q_OS_MAC)
// Include some MacOS X stuff
#include <Carbon/Carbon.h>
#include <visit-config.h>
// Include extra Qt stuff.
#include <QEventLoop>
#include <QTimer>
#endif

// ****************************************************************************
// Method: QvisApplication::QvisApplication
//
// Purpose: 
//   Constructor for the QvisApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:18:47 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisApplication::QvisApplication( int &argc, char **argv) :
    QApplication(argc, argv)
{
#if defined(Q_OS_MAC)
    needToMakeActive = false;
    eventLoop = 0;
#endif
}

QvisApplication::QvisApplication( int &argc, char **argv, bool GUIenabled ) :
    QApplication(argc, argv, GUIenabled)
{
#if defined(Q_OS_MAC)
    needToMakeActive = false;
    eventLoop = 0;
#endif
}

// ****************************************************************************
// Method: QvisApplication::~QvisApplication
//
// Purpose: 
//   Destructor for the QvisApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:18:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisApplication::~QvisApplication()
{
}

// ****************************************************************************
// Method: QvisApplication::exitTheLoop
//
// Purpose: 
//   Exits a sub-event loop.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 9 18:28:34 PST 2007
//
// Modifications:
//   Brad Whitlock, Fri May 30 11:52:03 PDT 2008
//   Qt 4.
//
// ****************************************************************************
void
QvisApplication::exitTheLoop()
{
#if defined(Q_OS_MAC)
    eventLoop->exit();
    delete eventLoop;
    eventLoop = 0;
#endif
}
