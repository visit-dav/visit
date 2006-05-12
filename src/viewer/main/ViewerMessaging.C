#include <ViewerMessaging.h>
#include <ViewerSubject.h>

// An external pointer to the ViewerSubject.
extern ViewerSubject *viewerSubject;

// ****************************************************************************
// Function: Error
//
// Purpose:
//   Sends an error message to the observers of the viewer's messageAtts
//   subject.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 14:03:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Error(const char *message)
{
    viewerSubject->Error(message);
}

// ****************************************************************************
// Function: Warning
//
// Purpose:
//   Sends a warning message to the observers of the viewer's messageAtts
//   subject.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 14:03:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Warning(const char *message)
{
    viewerSubject->Warning(message);
}

// ****************************************************************************
// Function: Message
//
// Purpose:
//   Sends a message to the observers of the viewer's messageAtts subject.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 14:03:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Message(const char *message)
{
    viewerSubject->Message(message);
}

// ****************************************************************************
// Function: ErrorClear
//
// Purpose:
//   Sends an error clear message to the observers of the viewer's messageAtts
//   subject.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu May 11 15:04:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ErrorClear()
{
    viewerSubject->ErrorClear();
}

// ****************************************************************************
// Function: Status
//
// Purpose: 
//   Sends a status message to the GUI. This is displayed in the status bar.
//
// Arguments:
//   message : The message that gets displayed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:45:28 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Status(const char *message)
{
    viewerSubject->Status(message);
}

// ****************************************************************************
// Function: Status
//
// Purpose: 
//   Sends a status message to the GUI. This is displayed in the status bar.
//
// Arguments:
//   message      : The message that gets displayed.
//   milliseconds : How long the message shoould be displayed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 21 13:22:01 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Status(const char *message, int milliseconds)
{
    viewerSubject->Status(message, milliseconds);
}

// ****************************************************************************
// Function: Status
//
// Purpose: 
//   Sends a status message to the GUI. This is displayed in the status bar.
//
// Arguments:
//   message : The message that gets displayed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:45:28 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Status(const char *sender, const char *message)
{
    viewerSubject->Status(sender, message);
}

// ****************************************************************************
// Function: Status
//
// Purpose: 
//   Sends a status message back to the GUI. This message contains information
//   about a progressive operation in the engine.
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
// Creation:   Mon Apr 30 14:46:17 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
Status(const char *sender, int percent, int curStage,
       const char *curStageName, int maxStage)
{
    viewerSubject->Status(sender, percent, curStage, curStageName, maxStage);
}

// ****************************************************************************
// Function: ClearStatus
//
// Purpose: 
//   Clears the status bar in the GUI.
//
// Arguments:
//   sender : The component that sent the message.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:47:14 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ClearStatus(const char *sender)
{
    viewerSubject->ClearStatus(sender);
}
