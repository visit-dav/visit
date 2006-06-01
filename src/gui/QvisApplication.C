/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisApplication.h>

#ifdef Q_WS_MACX
// Include some MacOS X stuff
#include <Carbon/Carbon.h>
#include <visit-config.h>
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
}

QvisApplication::QvisApplication( int &argc, char **argv, bool GUIenabled ) :
    QApplication(argc, argv, GUIenabled)
{
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
//   
// ****************************************************************************

bool
QvisApplication::macEventFilter(EventRef event)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    
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
                if(mainWindowName == QString(widget->name()))
                    emit showApplication();
                //qDebug("ekind = kEventWindowShown");
            }
            else if(ekind == kEventWindowHidden)
            {
                QString mainWindowName(QString("VisIt ") + QString(VERSION));
                if(mainWindowName == QString(widget->name()))
                    emit hideApplication();            
                //qDebug("ekind = kEventWindowHidden");
            }
        }        
        break;
      }
#if 0
    // Trap for other Carbon events.
    case kEventClassApplication:
        qDebug("kEventClassApplication");
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
    case kEventClassMenu:
        qDebug("kEventClassMenu");
        break;
    case kEventClassMouse:
        qDebug("kEventClassMouse");
        break;
#endif
    }

    return false;
}
#endif
