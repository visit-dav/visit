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
#ifndef VIEWER_MESSAGING_H
#define VIEWER_MESSAGING_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <ViewerText.h>

#include <deque>

class ViewerInternalCommand;

// ****************************************************************************
// Class: ViewerMessaging
//
// Purpose:
//   Handles messaging within the viewer.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 10:38:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerMessaging : public ViewerBase
{
public:
    ViewerMessaging();
    virtual ~ViewerMessaging();

    void BlockClientInput(bool b);
    bool ClientInputBlocked() const;

    // Methods for internal commands.
    ViewerInternalCommand *GetCommand();
    void                   QueueCommand(ViewerInternalCommand *cmd, int timeout = 1);
    void                   ProcessCommands();
    void                   SetCommandsNotifyCallback(void (*cb)(void *, int),
                                                     void *cbdata);

    // Methods to send messages
    void Error(const ViewerText &message);
    void Warning(const ViewerText &message);
    void Message(const ViewerText &message);
    void ErrorClear();

    // Methods to send status
    void Status(const ViewerText &message);
    void Status(const ViewerText &message, int milliseconds);
    void Status(const ViewerText &sender, const ViewerText &message);
    void Status(const ViewerText &sender, int percent, int curStage,
                        const ViewerText &curStageName, int maxStage);
    void ClearStatus(const ViewerText &sender = ViewerText());
    
    // Message Suppression Control
    void EnableMessageSuppression();
    void DisableMessageSuppression();
    bool SuppressMessages();

    // Message box
    virtual bool InformationBox(const ViewerText &title, const ViewerText &msg);
protected:
    // Helper functions that we can override.
    virtual void SetMessageAttributesText(const ViewerText &message);
    virtual void SetStatusAttributesMessage(const ViewerText &message);
private:
    std::deque<ViewerInternalCommand *> commands;
    bool blockSocketSignals;
    void (*commandsNotifyCB)(void *, int);
    void *commandsNotifyCBData;
};

#endif
