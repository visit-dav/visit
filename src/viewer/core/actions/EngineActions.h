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
#ifndef ENGINE_ACTIONS_H
#define ENGINE_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: ClearCacheForAllEnginesAction
//
// Purpose:
//   Handles ViewerRPC::ClearCacheForAllEnginesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ClearCacheForAllEnginesAction : public ViewerActionLogic
{
public:
    ClearCacheForAllEnginesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearCacheForAllEnginesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ClearCacheAction
//
// Purpose:
//   Handles ViewerRPC::ClearCacheRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ClearCacheAction : public ViewerActionLogic
{
public:
    ClearCacheAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ClearCacheAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CloseComputeEngineAction
//
// Purpose:
//   Handles ViewerRPC::CloseComputeEngineRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CloseComputeEngineAction : public ViewerActionLogic
{
public:
    CloseComputeEngineAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CloseComputeEngineAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: GetProcInfoAction
//
// Purpose:
//   Handles ViewerRPC::GetProcInfoRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API GetProcInfoAction : public ViewerActionLogic
{
public:
    GetProcInfoAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~GetProcInfoAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: OpenComputeEngineAction
//
// Purpose:
//   Handles ViewerRPC::OpenComputeEngineRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenComputeEngineAction : public ViewerActionLogic
{
public:
    OpenComputeEngineAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~OpenComputeEngineAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SendSimulationCommandAction
//
// Purpose:
//   Handles ViewerRPC::SendSimulationCommandRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SendSimulationCommandAction : public ViewerActionLogic
{
public:
    SendSimulationCommandAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SendSimulationCommandAction() {}

    virtual void Execute();
};

#endif
