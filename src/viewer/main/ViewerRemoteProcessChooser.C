/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include "ViewerRemoteProcessChooser.h"

#include <MachineProfile.h>
#include <HostProfileList.h>
#include <ViewerHostProfileSelectorNoWin.h>
#include <ViewerHostProfileSelectorWithWin.h>
#include <ViewerProperties.h>
#include <RemoteProxyBase.h>

#include <stdio.h>
#include <snprintf.h>

#include <DebugStream.h>

#include <string>
using std::string;

ViewerRemoteProcessChooser *ViewerRemoteProcessChooser::instance = NULL;

// ****************************************************************************
//  Constructor:  ViewerRemoteProcessChooser::ViewerRemoteProcessChooser
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Rework to use ViewerHostProfileSelector. 
//
//    Brad Whitlock, Tue Apr 14 11:42:27 PDT 2009
//    Inherit from ViewerBase.
//
// ****************************************************************************

ViewerRemoteProcessChooser::ViewerRemoteProcessChooser() : ViewerBase()
{
    if (GetViewerProperties()->GetNowin())
        selector = new ViewerHostProfileSelectorNoWin();
    else
        selector = new ViewerHostProfileSelectorWithWin();
}

// ****************************************************************************
//  Destructor:  ViewerRemoteProcessChooser::~ViewerRemoteProcessChooser
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Rework to use ViewerHostProfileSelector. 
//
// ****************************************************************************

ViewerRemoteProcessChooser::~ViewerRemoteProcessChooser()
{
    if (selector)
        delete selector;
}


// ****************************************************************************
//  Method:  ViewerRemoteProcessChooserNoWin::SelectProfile
//
//  Purpose:
//    Calls the chosen style of host profile selector.
//
//  Arguments:
//    hostProfileList : all host profiles
//    hostName        : the host name
//    skipChooser     : true if no window should appear
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 26, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Aug 5 09:25:20 PDT 2004
//    I made it return the profile.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
// ****************************************************************************

bool
ViewerRemoteProcessChooser::SelectProfile(HostProfileList *hostProfileList,
    const string &hostName, bool skipChooser, MachineProfile &profile)
{
    // sets profile
    bool retval = selector->SelectProfile(hostProfileList, hostName, skipChooser);

    // If a profile was selected, return it here.
    if(retval)
        profile = selector->GetMachineProfile();

    return retval;
}


// ****************************************************************************
//  Method:  ViewerRemoteProcessChooser::ClearCache
//
//  Purpose:
//    Erases previous settings for re-launching engines.
//    This will typically be called if the previous launch failed
//    or if the user closed the engine manually.
//
//  Arguments:
//    hostName   the host name of the remote engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Use selector.
//
// ****************************************************************************

void
ViewerRemoteProcessChooser::ClearCache(const std::string &hostName)
{
    if (selector)
        selector->ClearCache(hostName);
}

// ****************************************************************************
//  Method:  ViewerRemoteProcessChooser::AddRestartArgsToCachedProfile
//
//  Purpose:
//    Modify a cached profile to include the given arguments.
//
//  Arguments:
//    hostName   the host name of the remote engine
//    args       the arguments to save
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  2, 2004
//
// ****************************************************************************
void
ViewerRemoteProcessChooser::AddRestartArgsToCachedProfile(
                                          const std::string &hostName,
                                          const std::vector<std::string> &args)
{
    if (selector)
        selector->AddRestartArgsToCachedProfile(hostName, args);
}

// ****************************************************************************
//  Method:  ViewerRemoteProcessChooser::Instance
//
//  Purpose:
//    Gets (and creates on demand) the global instance of the dialog.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
// ****************************************************************************

ViewerRemoteProcessChooser *
ViewerRemoteProcessChooser::Instance()
{
    if (!instance)
        instance = new ViewerRemoteProcessChooser();
    return instance;
}
