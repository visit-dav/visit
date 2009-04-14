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

#ifndef VIEWERREMOTEPROCESSCHOOSER_H
#define VIEWERREMOTEPROCESSCHOOSER_H
#include <viewer_exports.h>
#include <ViewerBase.h>

#include <string>
#include <vector>

class HostProfile;
class HostProfileList;
class RemoteProxyBase;
class ViewerHostProfileSelector;

// ****************************************************************************
//  Class:  ViewerRemoteProcessChooser
//
//  Purpose:
//    Picks a host profile and some parallel settings to launch the engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Jeremy Meredith, Sat Aug 17 11:03:20 PDT 2002
//    Added nowin support.
//
//    Jeremy Meredith, Wed Dec 18 17:20:23 PST 2002
//    Added support for skipping the chooser when launching engines.
//
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003   
//    Moved host-profile selection code to ViewerHostProfileSelector. Added
//    member selector. 
//
//    Jeremy Meredith, Thu Jun 26 10:54:37 PDT 2003
//    Renamed to ViewerRemoteProcessChooser.  Split GetNewEngine into two
//    functions (SelectProfile and AddProfileArguments) so that the caller
//    could create a new proxy itself.  This disassociates this object
//    from the Engine and lets it be used with the VCL (for example).
//
//    Jeremy Meredith, Fri Apr  2 14:35:24 PST 2004
//    Added AddRestartArgsToCachedProfile.  This is a better way to save
//    the arguments than what the ViewerEngineManager was doing for
//    normal engine launches.
//
//    Brad Whitlock, Wed Aug 4 17:38:16 PST 2004
//    I made the selected host profile get passed out of SelectProfile. I also
//    moved AddProfileArguments into RemoteProxyBase.
//
//    Brad Whitlock, Tue Apr 14 11:42:03 PDT 2009
//    Inherit from ViewerBase.
//
// ****************************************************************************

class VIEWER_API ViewerRemoteProcessChooser : public ViewerBase
{
  public:
    virtual ~ViewerRemoteProcessChooser();

    static ViewerRemoteProcessChooser *Instance();
    static void SetNoWinMode(bool nw);
    bool SelectProfile(HostProfileList*, const std::string&, bool skip,
                       HostProfile &profile);
    void AddRestartArgsToCachedProfile(const std::string&,
                                       const std::vector<std::string>&);
    void ClearCache(const std::string&);

  private:
    static ViewerRemoteProcessChooser *instance;

    ViewerHostProfileSelector *selector;
    ViewerRemoteProcessChooser();
};

#endif
