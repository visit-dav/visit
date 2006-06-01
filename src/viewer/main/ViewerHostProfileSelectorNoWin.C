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

#include <ViewerHostProfileSelectorNoWin.h>

#include <HostProfileList.h>

#include <stdio.h>

#include <DebugStream.h>

#include <string>
using std::string;


// ****************************************************************************
//  Constructor:  ViewerHostProfileSelectorNoWin::ViewerHostProfileSelectorNoWin
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    Februrary 5, 2003 
//
//  Modifications:
//
// ****************************************************************************

ViewerHostProfileSelectorNoWin::ViewerHostProfileSelectorNoWin()
{
}

// ****************************************************************************
//  Destructor:  ViewerHostProfileSelectorNoWin::~ViewerHostProfileSelectorNoWin
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
// ****************************************************************************

ViewerHostProfileSelectorNoWin::~ViewerHostProfileSelectorNoWin()
{
}

// ****************************************************************************
//  Method:  ViewerHostProfileSelectorNoWin::SelectProfile
//
//  Purpose:
//    Selects a host profile based on the host profile list and the hostName.
//
//  Arguments:
//    hostProfileList : a list of possible profiles
//    hostName        : the host name
//    skipChooser     : if true, a default host profile is returned. 
//
//  Returns:  true if a host profile could be selected, false otherwise.
//
//  Notes:
//    Extracted from ViewerEngineChooser::GetNewEngine by Jeremy Meredith.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  2 14:30:11 PST 2004
//    Added caching of the empty profile when we skip the chooser.  The reason
//    this is useful is because I added a method to save the manual arguments
//    with a cached profile, but this needed to work even if we skipped the
//    chooser.
//
// ****************************************************************************

bool
ViewerHostProfileSelectorNoWin::SelectProfile(
    HostProfileList *hostProfileList, const string &hostName, bool skipChooser)
{
    int  i;
    profile = HostProfile();

    if (skipChooser)
    {
        // do nothing; leave the profile completely blank
        // Save it so we can cache the extra arguments, though
        cachedProfile[hostName] = profile;
    }
    else if (cachedProfile.count(hostName))
    {
        profile = cachedProfile[hostName];
    }
    else
    {
        matchingProfiles = 
                    hostProfileList->FindAllMatchingProfileForHost(hostName);

        if (matchingProfiles.size() > 0)
        {
            profile = *matchingProfiles[0];
        }
        
        if (matchingProfiles.size() > 1 ||
            (matchingProfiles.size() == 1 &&
             matchingProfiles[0]->GetParallel()))
        {
            for (i = 0; i < matchingProfiles.size(); i++)
            {
                if (matchingProfiles[i]->GetActive())
                {
                    profile.SetNumProcessors(matchingProfiles[i]->GetNumProcessors());
                    profile.SetNumNodes(matchingProfiles[i]->GetNumNodes());
                    profile.SetBank(matchingProfiles[i]->GetBank());
                    profile.SetTimeLimit(matchingProfiles[i]->GetTimeLimit());
                }
            }
        }

        // Save it for use later
        cachedProfile[hostName] = profile;
    }
    return true;
}

