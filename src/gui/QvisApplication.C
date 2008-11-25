/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisApplication.h>

#include <QMenuBar>

#ifdef Q_WS_MACX
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
#ifdef Q_WS_MACX
    needToMakeActive = false;
    eventLoop = 0;
#endif
}

QvisApplication::QvisApplication( int &argc, char **argv, bool GUIenabled ) :
    QApplication(argc, argv, GUIenabled)
{
#ifdef Q_WS_MACX
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

#ifdef Q_WS_MACX
// ****************************************************************************
// Method: QvisApplication::macEventFilter
//
// Purpose: 
//   Handles some events for VisIt when running on MacOS X.
//
// Arguments:
//   event : The event that we might trap.
//
// Returns:    false so the main event handler will also trap the event and
//             do the right thing with it. We use this method to trap for
//             window hide/show events for the main window. When we get such
//             an event, we emit hideApplication and showApplication signals
//             so we can tell the viewer to hide and show too.
//
// Note:       This code is only built on MacOS X.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:18:57 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Oct 9 15:16:34 PST 2007
//   Changed signature for macEventFilter to match newer Qt method. Fixed
//   focus click problem for the menus.
//
//   Brad Whitlock, Fri May 30 11:40:21 PDT 2008
//   Qt 4.
//
// ****************************************************************************
static EventRef request_make_app_active = NULL;

bool
QvisApplication::macEventFilter(EventHandlerCallRef er, EventRef event)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    bool ret = false;

    switch(eclass)
    {
    case kEventClassWindow:
      { // new scope
        //qDebug("macEventFilter: kEventClassWindow");
        WindowRef wid;
        GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
                          sizeof(WindowRef), NULL, &wid);
        QWidget *widget = QWidget::find((WId)wid);
        if(widget)
        {
            if(ekind == kEventWindowShown)
            {
                QString mainWindowName(QString("VisIt ") + QString(VERSION));
                if(mainWindowName == QString(widget->windowTitle()))
                    emit showApplication();
                //qDebug("ekind = kEventWindowShown");
            }
            else if(ekind == kEventWindowHidden)
            {
                QString mainWindowName(QString("VisIt ") + QString(VERSION));
                if(mainWindowName == QString(widget->windowTitle()))
                    emit hideApplication();            
                //qDebug("ekind = kEventWindowHidden");
            }
#ifdef PRINT_CARBON_EVENTS
            else if(ekind == kEventWindowClose)
                qDebug("\tkEventWindowClose");
            else if(ekind == kEventWindowDrawContent)
                qDebug("\tkEventWindowDrawContent");
            else if(ekind == kEventWindowBoundsChanged)
                qDebug("\tkEventWindowBoundsChanged");
#endif
        }        
        break;
      }
    // Trap for other Carbon events.
    case kEventClassApplication:
        if(ekind == kEventAppDeactivated)
        {
            //qDebug("\tkEventAppDeactivated");

            // We're deactivating the application so the next time we activate it
            // via the menu, we need to make it active.
            needToMakeActive = true;
        }
        else if(ekind == kEventAppActivated)
        {
            //qDebug("\tkEventAppActivated");
            needToMakeActive = false;
        }
#ifdef PRINT_CARBON_EVENTS
        else if(ekind == kEventAppQuit)
            qDebug("\tkEventAppQuit");
        else if(ekind == kEventAppLaunchNotification)
            qDebug("\tkEventAppLaunchNotification");
        else if(ekind == kEventAppLaunched)
            qDebug("\tkEventAppLaunched");
        else if(ekind == kEventAppTerminated)
            qDebug("\tkEventAppTerminated");
        else if(ekind == kEventAppFrontSwitched)
            qDebug("\tkEventAppFrontSwitched");
        else if(ekind == kEventAppFocusMenuBar)
            qDebug("\tkEventAppFocusMenuBar");
        else if(ekind == kEventAppFocusNextDocumentWindow)
            qDebug("\tkEventAppFocusNextDocumentWindow");
        else if(ekind == kEventAppFocusNextFloatingWindow)
            qDebug("\tkEventAppFocusNextFloatingWindow");
        else if(ekind == kEventAppFocusToolbar)
            qDebug("\tkEventAppFocusToolbar");
        else if(ekind == kEventAppFocusDrawer)
            qDebug("\tkEventAppFocusDrawer");
        else if(ekind == kEventAppGetDockTileMenu)
            qDebug("\tkEventAppGetDockTileMenu");
        else if(ekind == kEventAppIsEventInInstantMouser)
            qDebug("\tkEventAppIsEventInInstantMouser");
        else if(ekind == kEventAppHidden)
            qDebug("\tkEventAppHidden");
        else if(ekind == kEventAppShown)
            qDebug("\tkEventAppShown");
        else if(ekind == kEventAppSystemUIModeChanged)
            qDebug("\tkEventAppSystemUIModeChanged");
        else if(ekind == kEventAppAvailableWindowBoundsChanged)
            qDebug("\tkEventAppAvailableWindowBoundsChanged");
        else if(ekind == kEventAppActiveWindowChanged)
            qDebug("\tkEventAppActiveWindowChanged");
        break;
    case kEventClassCommand:
        qDebug("kEventClassCommand");
        break;
    case kEventClassControl:
        qDebug("kEventClassControl");
        break;
    case kEventClassKeyboard:
        qDebug("kEventClassKeyboard");
        break;
#endif
    case kEventClassMenu:
#ifdef PRINT_CARBON_EVENTS
        qDebug("kEventClassMenu");
        if(ekind == kEventMenuBeginTracking)
            qDebug("\tkEventMenuBeginTracking");
        else if(ekind == kEventMenuEndTracking)
            qDebug("\tkEventMenuEndTracking");
        else if(ekind == kEventMenuChangeTrackingMode)
            qDebug("\tkEventMenuChangeTrackingMode");
        else if(ekind == kEventMenuClosed)
            qDebug("\tkEventMenuClosed");
        else if(ekind == kEventMenuTargetItem)
            qDebug("\tkEventMenuTargetItem");
        else if(ekind == kEventMenuMatchKey)
            qDebug("\tkEventMenuMatchKey");
        else if(ekind == kEventMenuEnableItems)
            qDebug("\tkEventMenuEnableItems");
        else
#endif
        if(ekind == kEventMenuOpening)
        {
            //qDebug("\tkEventMenuOpening");

            if(needToMakeActive)
            {
                // If we got here then it's from making the menu active after having
                // left the application for the viewer. In this case, we pull some 
                // tricks on Qt to make it execute the AppActivated event before the
                // current event.
                needToMakeActive = false;

                // Inject a Carbon event to make the application active.
                CreateEvent(NULL, kEventClassApplication, kEventAppActivated, GetCurrentEventTime(),
		     NULL, &request_make_app_active);
                PostEventToQueue(GetCurrentEventQueue(), request_make_app_active, kEventPriorityHigh);
                // Inject a directive to exit a sub-event loop that we'll be creating.
                QTimer::singleShot(10, this, SLOT(exitTheLoop()));

                // Start a new event loop to make the app active and then quit
                // the sub-event loop.
                eventLoop = new QEventLoop(0);
                eventLoop->exec();
            }
        }
        break;
#ifdef PRINT_CARBON_EVENTS
    case kEventClassMouse:
        qDebug("kEventClassMouse");
        break;
#endif
    }

    return ret;
}
#endif

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
#ifdef Q_WS_MACX
    eventLoop->exit();
    delete eventLoop;
    eventLoop = 0;
#endif
}
