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
#ifndef CLIENT_ACTIONS_H
#define CLIENT_ACTIONS_H
#include <ViewerActionLogic.h>
#include <ViewerWindow.h>
#include <ViewerMethods.h>
#include <InstallationFunctions.h>

// ****************************************************************************
// Class: OpenGUIClientAction
//
// Purpose:
//   This action opens the VisIt GUI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 09:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenGUIClientAction : public ViewerActionLogic
{
public:
    OpenGUIClientAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~OpenGUIClientAction() { }

    virtual void Execute()
    {
        stringVector args;
        GetViewerMethods()->OpenClient("GUI", GetVisItLauncher(), args);
    }
};

// ****************************************************************************
// Class: OpenCLIClientAction
//
// Purpose:
//   This action opens the VisIt CLI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 09:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenCLIClientAction : public ViewerActionLogic
{
public:
    OpenCLIClientAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~OpenCLIClientAction() { }

    virtual void Execute()
    {
        stringVector args;
        args.push_back("-cli");
        args.push_back("-newconsole");
        GetViewerMethods()->OpenClient("CLI", GetVisItLauncher(), args);
    }
};

// ****************************************************************************
// Class: MenuQuitAction
//
// Purpose:
//   This action quits the viewer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 09:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MenuQuitAction : public ViewerActionLogic
{
public:
    MenuQuitAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~MenuQuitAction() { }

    virtual void Execute()
    {
        GetViewerMethods()->Close();
        EXCEPTION1(VisItException, "Bypass action manager update. This is safe and by design");
    }
};

#endif
