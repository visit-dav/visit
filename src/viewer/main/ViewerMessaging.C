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
