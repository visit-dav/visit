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

#include <ViewerBase.h>

#include <ViewerState.h>
#include <ViewerMethods.h>

#include <MessageAttributes.h>
#include <StatusAttributes.h>

#include <DebugStream.h>

ViewerState   *ViewerBase::base_viewerState = 0;
ViewerMethods *ViewerBase::base_viewerMethods = 0;

// ****************************************************************************
// Method: ViewerBase::ViewerBase
//
// Purpose: 
//   Constructor for the ViewerBase class.
//
// Arguments:
//   parent : The parent object.
//   name   : The name of the object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:36:43 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerBase::ViewerBase(QObject *parent, const char *name) : 
    QObject(parent, name)
{
}

// ****************************************************************************
// Method: ViewerBase::~ViewerBase
//
// Purpose: 
//   Destructor for the ViewerBase class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:37:16 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerBase::~ViewerBase()
{
}

// ****************************************************************************
// Method: ViewerBase::GetViewerState
//
// Purpose: 
//   Returns pointer to the ViewerState object.
//
// Returns:    Pointer to the ViewerState object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:37:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
ViewerBase::GetViewerState()
{
    if(base_viewerState == 0)
        base_viewerState = new ViewerState;

    return base_viewerState;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerMethods
//
// Purpose: 
//   Returns a pointer to the ViewerMethods object.
//
// Returns:    Pointer to the ViewerMethods object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:38:34 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods *
ViewerBase::GetViewerMethods()
{
    if(base_viewerMethods == 0)
        base_viewerMethods = new ViewerMethods(GetViewerState());

    return base_viewerMethods;
}

// ****************************************************************************
// Method: ViewerBase::Error
//
// Purpose: 
//   Sends an error message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 13:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:24:51 PST 2003
//   Updated MessageAttributes.
//
//   Brad Whitlock, Fri Mar 19 16:12:00 PST 2004
//   Added code to print errors to debug1.
//
// ****************************************************************************

void
ViewerBase::Error(const char *message)
{
    if ((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    GetViewerState()->GetMessageAttributes()->SetText(std::string(message));
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::Error);
    GetViewerState()->GetMessageAttributes()->Notify();

    debug1 << "Error - " << message << endl;
}

// ****************************************************************************
// Method: ViewerBase::Warning
//
// Purpose: 
//   Sends a warning message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 13:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:24:51 PST 2003
//   Updated MessageAttributes.
//
//   Brad Whitlock, Fri Mar 19 16:12:00 PST 2004
//   Added code to print errors to debug1.
//
// ****************************************************************************

void
ViewerBase::Warning(const char *message)
{
    if ((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    GetViewerState()->GetMessageAttributes()->SetText(std::string(message));
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::Warning);
    GetViewerState()->GetMessageAttributes()->Notify();

    debug1 << "Warning - " << message << endl;
}

// ****************************************************************************
// Method: ViewerBase::Message
//
// Purpose: 
//   Sends a message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 13:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:24:51 PST 2003
//   Updated MessageAttributes.
//   
//   Brad Whitlock, Fri Mar 19 16:12:00 PST 2004
//   Added code to print errors to debug1.
//
// ****************************************************************************

void
ViewerBase::Message(const char *message)
{
    if ((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    GetViewerState()->GetMessageAttributes()->SetText(std::string(message));
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::Message);
    GetViewerState()->GetMessageAttributes()->Notify();

    debug1 << "Message - " << message << endl;
}

// ****************************************************************************
// Method: ViewerBase::ErrorClear
//
// Purpose: 
//   Sends a clear errors message to the clients.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 11 15:06:36 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
ViewerBase::ErrorClear()
{
    // Send the message to the observers of the viewer's messageAtts.
    debug1 << "Sending ErrorClear message to clients." << endl;
    GetViewerState()->GetMessageAttributes()->SetText("");
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::ErrorClear);
    GetViewerState()->GetMessageAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerBase::Status
//
// Purpose: 
//   Sends a status message to the GUI.
//
// Arguments:
//   message : The status message that is sent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:38:09 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:09:08 PDT 2001
//    Added the MessageType field.
//
//    Brad Whitlock, Fri Sep 21 13:26:46 PST 2001
//    Added the duration field.
//
// ****************************************************************************

void
ViewerBase::Status(const char *message)
{
    GetViewerState()->GetStatusAttributes()->SetSender("viewer");
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(1);
    GetViewerState()->GetStatusAttributes()->SetStatusMessage(message);
    GetViewerState()->GetStatusAttributes()->SetDuration(StatusAttributes::DEFAULT_DURATION);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerBase::Status
//
// Purpose: 
//   Sends a status message to the GUI.
//
// Arguments:
//   message      : The status message that is sent.
//   milliseconds : The duration of time that the status message is displayed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 21 13:24:52 PST 2001
//
// Modifications:
//
// ****************************************************************************

void
ViewerBase::Status(const char *message, int milliseconds)
{
    GetViewerState()->GetStatusAttributes()->SetSender("viewer");
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(1);
    GetViewerState()->GetStatusAttributes()->SetStatusMessage(message);
    GetViewerState()->GetStatusAttributes()->SetDuration(milliseconds);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerBase::Status
//
// Purpose: 
//   Sends a status message for a component other than the viewer.
//
// Arguments:
//   sender  : The name of the component sending the message.
//   message : The message to be sent.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 1 12:06:22 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:09:08 PDT 2001
//    Added the MessageType field.
//   
//    Brad Whitlock, Fri Sep 21 13:26:46 PST 2001
//    Added the duration field.
//
// ****************************************************************************

void
ViewerBase::Status(const char *sender, const char *message)
{
    GetViewerState()->GetStatusAttributes()->SetSender(sender);
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(1);
    GetViewerState()->GetStatusAttributes()->SetStatusMessage(message);
    GetViewerState()->GetStatusAttributes()->SetDuration(StatusAttributes::DEFAULT_DURATION);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerBase::Status
//
// Purpose: 
//   Sends a status message for a component other than the viewer. This
//   message includes percent done, etc.
//
// Arguments:
//   sender       : The component that sent the status update. This is a
//                  host name for an engine that sent the message.
//   percent      : The percent through the current stage.
//   curStage     : The number of the current stage.
//   curStageName : The name of the current stage.
//   maxStage     : The maximum number of stages.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:39:30 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:09:08 PDT 2001
//    Added the MessageType field.
//   
//    Brad Whitlock, Fri Sep 21 13:26:46 PST 2001
//    Added the duration field.
//
// ****************************************************************************

void
ViewerBase::Status(const char *sender, int percent, int curStage,
    const char *curStageName, int maxStage)
{
    GetViewerState()->GetStatusAttributes()->SetSender(sender);
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(2);
    GetViewerState()->GetStatusAttributes()->SetPercent(percent);
    GetViewerState()->GetStatusAttributes()->SetCurrentStage(curStage);
    GetViewerState()->GetStatusAttributes()->SetCurrentStageName(curStageName);
    GetViewerState()->GetStatusAttributes()->SetMaxStage(maxStage);
    GetViewerState()->GetStatusAttributes()->SetDuration(StatusAttributes::DEFAULT_DURATION);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerBase::ClearStatus
//
// Purpose: 
//   Sends an empty message to the status bar to clear it.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:33:37 PST 2001
//
// Modifications:
//    Jeremy Meredith, Tue Jul  3 15:21:37 PDT 2001
//    Also set all percentages/stages to zeroes.
//
// ****************************************************************************

void
ViewerBase::ClearStatus(const char *sender)
{
    GetViewerState()->GetStatusAttributes()->SetSender((sender == 0) ? "viewer" : sender);
    GetViewerState()->GetStatusAttributes()->SetClearStatus(true);
    GetViewerState()->GetStatusAttributes()->SetPercent(0);
    GetViewerState()->GetStatusAttributes()->SetCurrentStage(0);
    GetViewerState()->GetStatusAttributes()->SetCurrentStageName("");
    GetViewerState()->GetStatusAttributes()->SetMaxStage(0);
    GetViewerState()->GetStatusAttributes()->Notify();
}
