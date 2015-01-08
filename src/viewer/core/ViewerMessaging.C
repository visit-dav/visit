/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <ViewerMessaging.h>
#include <ViewerInternalCommands.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <VisItException.h>

#include <MessageAttributes.h>
#include <StatusAttributes.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: ViewerMessaging::ViewerMessaging
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:47:31 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessaging::ViewerMessaging() : ViewerBase(), commands(), 
    blockSocketSignals(false), 
    commandsNotifyCB(NULL), commandsNotifyCBData(NULL)
{
}

// ****************************************************************************
// Method: ViewerMessaging::~ViewerMessaging
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:47:31 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessaging::~ViewerMessaging()
{
    // Delete any commands that are still in the queue.
    for(size_t i = 0; i < commands.size(); ++i)
        delete commands[i];
}

// ****************************************************************************
// Method: ViewerMessaging::BlockClientInput
//
// Purpose:
//   Sets whether socket input from clients is allowed.
//
// Arguments:
//   b : True if client input is allowed; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:48:15 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessaging::BlockClientInput(bool b)
{
    blockSocketSignals = b;
}

// ****************************************************************************
// Method: ViewerMessaging::ClientInputBlocked
//
// Purpose:
//   Return whether client input is blocked.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 13:20:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerMessaging::ClientInputBlocked() const
{
    return blockSocketSignals;
}

// ****************************************************************************
// Method: ViewerMessaging::GetCommand
//
// Purpose:
//   Read a command from the command queue.
//
// Returns:    A pointer to an internal command object or NULL if there is none.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:50:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerInternalCommand *
ViewerMessaging::GetCommand()
{
    ViewerInternalCommand *cmd = NULL;
    if(!commands.empty())
    {
        cmd = commands.front();
        commands.pop_front();
    }
    return cmd;
}

// ****************************************************************************
// Method: ViewerMessaging::QueueCommand
//
// Purpose:
//   Queue a command for processing.
//
// Arguments:
//   cmd : The command to queue up.
//
// Returns:    
//
// Note:       The command object will be deleted later when it is processed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:52:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessaging::QueueCommand(ViewerInternalCommand *cmd, int timeout)
{
    commands.push_back(cmd);

    // Process the commands via callback.
    if(commandsNotifyCB != NULL)
        (*commandsNotifyCB)(commandsNotifyCBData, timeout);
}

// ****************************************************************************
// Method: ViewerMessaging::SetCommandsNotifyCallback
//
// Purpose:
//   Sets the callback function to be called when we want to notify that there
//   are commands for processing.
//
// Arguments:
//   cb     : The callback.
//   cbdata : The callback data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:54:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessaging::SetCommandsNotifyCallback(void (*cb)(void *,int), void *cbdata)
{
    commandsNotifyCB = cb;
    commandsNotifyCBData = cbdata;
}

// ****************************************************************************
// Method: ViewerMessaging::ProcessCommands
//
// Purpose:
//   Process queued commands.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 12:32:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessaging::ProcessCommands()
{
    TRY
    {
        ViewerInternalCommand *cmd = NULL;
        while((cmd = GetCommand()) != NULL)
        {
            cmd->Execute();
            delete cmd;
        }
    }
    CATCHALL
    {
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerMessaging::SetMessageAttributesText
//
// Purpose:
//   Helps set the ViewerText object into the message attributes.
//
// Arguments:
//   message : The message to store.
//
// Returns:    
//
// Note:       This is virtual so we can hook in translation in derived classes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 17:04:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessaging::SetMessageAttributesText(const ViewerText &message)
{
    GetViewerState()->GetMessageAttributes()->SetText(message.toStdString());
    GetViewerState()->GetMessageAttributes()->SetHasUnicode(false);
}

// ****************************************************************************
// Method: ViewerMessaging::SetStatusAttributesMessage
//
// Purpose:
//   Helps set the ViewerText object into the status attributes.
//
// Arguments:
//   message : The message to store.
//
// Returns:    
//
// Note:       This is virtual so we can hook in translation in derived classes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 17:04:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessaging::SetStatusAttributesMessage(const ViewerText &message)
{
    GetViewerState()->GetStatusAttributes()->SetStatusMessage(message.toStdString());
    GetViewerState()->GetStatusAttributes()->SetHasUnicode(false);
}

// ****************************************************************************
// Method: ViewerMessaging::Error
//
// Purpose: 
//   Sends an error message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//   hasUnicode : True if the message was originally created using ViewerTexts.
//                We provide the option for false in the event that we want
//                pass an AVT error message (no translation) up.
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
//   Cyrus Harrison, Thu Feb 21 15:01:20 PST 2008
//   Added check for message suppression.
//
//   Brad Whitlock, Tue Apr 29 10:58:40 PDT 2008
//   Converted to ViewerText.
//
//   Brad Whitlock, Tue Apr 14 11:53:29 PDT 2009
//   Use a method instead of a member.
//
// ****************************************************************************

void
ViewerMessaging::Error(const ViewerText &message)
{
    if (message.empty() || SuppressMessages() )
        return;

    // Send the message to the observers of the viewer's messageAtts.
    SetMessageAttributesText(message);
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::Error);
    GetViewerState()->GetMessageAttributes()->Notify();

    debug1 << "Error - " << message.toStdString() << endl;
}

// ****************************************************************************
// Method: ViewerMessaging::Warning
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
//   Cyrus Harrison, Thu Feb 21 15:01:20 PST 2008
//   Added check for message suppression.
//
//   Brad Whitlock, Tue Apr 29 10:58:40 PDT 2008
//   Converted to ViewerText.
//
//   Brad Whitlock, Tue Apr 14 11:53:29 PDT 2009
//   Use a method instead of a member.
//
// ****************************************************************************

void
ViewerMessaging::Warning(const ViewerText &message)
{
    if (message.empty() || SuppressMessages())
        return;

    // Send the message to the observers of the viewer's messageAtts.
    SetMessageAttributesText(message);
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::Warning);
    GetViewerState()->GetMessageAttributes()->Notify();

    debug1 << "Warning - " << message.toStdString() << endl;
}

// ****************************************************************************
// Method: ViewerMessaging::Message
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
//   Cyrus Harrison, Thu Feb 21 15:01:20 PST 2008
//   Added check for message suppression.
//
//   Brad Whitlock, Tue Apr 29 10:58:40 PDT 2008
//   Converted to ViewerText.
//
//   Brad Whitlock, Tue Apr 14 11:53:29 PDT 2009
//   Use a method instead of a member.
//
// ****************************************************************************

void
ViewerMessaging::Message(const ViewerText &message)
{
    if (message.empty() || SuppressMessages() )
        return;

    // Send the message to the observers of the viewer's messageAtts.
    SetMessageAttributesText(message);
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::Message);
    GetViewerState()->GetMessageAttributes()->Notify();

    debug1 << "Message - " << message.toStdString() << endl;
}

// ****************************************************************************
// Method: ViewerMessaging::ErrorClear
//
// Purpose: 
//   Sends a clear errors message to the clients.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 11 15:06:36 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:04:39 PDT 2008
//   Set a new unicode flag.
//
// ****************************************************************************

void
ViewerMessaging::ErrorClear()
{
    // Send the message to the observers of the viewer's messageAtts.
    debug1 << "Sending ErrorClear message to clients." << endl;
    GetViewerState()->GetMessageAttributes()->SetText("");
    GetViewerState()->GetMessageAttributes()->SetHasUnicode(false);
    GetViewerState()->GetMessageAttributes()->SetSeverity(MessageAttributes::ErrorClear);
    GetViewerState()->GetMessageAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerMessaging::Status
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
//    Brad Whitlock, Tue Apr 29 11:05:28 PDT 2008
//    Converted to ViewerText.
//
// ****************************************************************************

void
ViewerMessaging::Status(const ViewerText &message)
{
    GetViewerState()->GetStatusAttributes()->SetSender("viewer");
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(1);
    SetStatusAttributesMessage(message);
    GetViewerState()->GetStatusAttributes()->SetDuration(StatusAttributes::DEFAULT_DURATION);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerMessaging::Status
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
//    Brad Whitlock, Tue Apr 29 11:05:28 PDT 2008
//    Converted to ViewerText.
//
// ****************************************************************************

void
ViewerMessaging::Status(const ViewerText &message, int milliseconds)
{
    GetViewerState()->GetStatusAttributes()->SetSender("viewer");
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(1);
    SetStatusAttributesMessage(message);
    GetViewerState()->GetStatusAttributes()->SetDuration(milliseconds);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerMessaging::Status
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
ViewerMessaging::Status(const ViewerText &sender, const ViewerText &message)
{
    GetViewerState()->GetStatusAttributes()->SetSender(sender.toStdString());
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(1);
    SetStatusAttributesMessage(message);
    GetViewerState()->GetStatusAttributes()->SetDuration(StatusAttributes::DEFAULT_DURATION);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerMessaging::Status
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
//    Brad Whitlock, Tue Apr 29 11:10:16 PDT 2008
//    Set the hasUnicode flag to false.
//
// ****************************************************************************

void
ViewerMessaging::Status(const ViewerText &sender, int percent, int curStage,
    const ViewerText &curStageName, int maxStage)
{
    GetViewerState()->GetStatusAttributes()->SetSender(sender.toStdString());
    GetViewerState()->GetStatusAttributes()->SetClearStatus(false);
    GetViewerState()->GetStatusAttributes()->SetMessageType(2);
    GetViewerState()->GetStatusAttributes()->SetPercent(percent);
    GetViewerState()->GetStatusAttributes()->SetCurrentStage(curStage);
    GetViewerState()->GetStatusAttributes()->SetCurrentStageName(curStageName.toStdString());
    GetViewerState()->GetStatusAttributes()->SetMaxStage(maxStage);
    GetViewerState()->GetStatusAttributes()->SetDuration(StatusAttributes::DEFAULT_DURATION);
    GetViewerState()->GetStatusAttributes()->SetHasUnicode(false);
    GetViewerState()->GetStatusAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerMessaging::ClearStatus
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
ViewerMessaging::ClearStatus(const ViewerText &sender)
{
    std::string s(sender.empty() ? std::string("viewer") : sender.toStdString());
    GetViewerState()->GetStatusAttributes()->SetSender(s);
    GetViewerState()->GetStatusAttributes()->SetClearStatus(true);
    GetViewerState()->GetStatusAttributes()->SetPercent(0);
    GetViewerState()->GetStatusAttributes()->SetCurrentStage(0);
    GetViewerState()->GetStatusAttributes()->SetCurrentStageName("");
    GetViewerState()->GetStatusAttributes()->SetMaxStage(0);
    GetViewerState()->GetStatusAttributes()->Notify();
}

void
ViewerMessaging::EnableMessageSuppression()
{
    GetViewerProperties()->SetSuppressMessages(true);
}

void
ViewerMessaging::DisableMessageSuppression()
{
    GetViewerProperties()->SetSuppressMessages(false);
}

bool
ViewerMessaging::SuppressMessages()
{
    return GetViewerProperties()->GetSuppressMessages();
}

// ****************************************************************************
// Method: ViewerMessaging::InformationBox
//
// Purpose:
//   Shows an information box and returns the result.
//
// Arguments:
//    title : The dialog window title.
//    msg   : The dialog message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:47:31 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerMessaging::InformationBox(const ViewerText &title, const ViewerText &msg)
{
    return true;
}
