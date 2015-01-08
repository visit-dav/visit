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
#ifndef SELECTION_ACTIONS_H
#define SELECTION_ACTIONS_H
#include <ViewerActionLogic.h>

class EngineKey;

// ****************************************************************************
// Class: SelectionActionBase
//
// Purpose:
//   Base class VIEWERCORE_API for selection actions.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SelectionActionBase : public ViewerActionLogic
{
public:
    SelectionActionBase(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SelectionActionBase() {}
protected:
    bool GetNamedSelectionEngineKey(const std::string &selName, EngineKey &ek);
    void ReplaceNamedSelection(const EngineKey &engineKey,
                               const std::string &selName,
                               const std::string &newSelName);
};

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: ApplyNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::ApplyNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ApplyNamedSelectionAction : public SelectionActionBase
{
public:
    ApplyNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~ApplyNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CreateNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::CreateNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CreateNamedSelectionAction : public SelectionActionBase
{
public:
    CreateNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~CreateNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DeleteNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::DeleteNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeleteNamedSelectionAction : public SelectionActionBase
{
public:
    DeleteNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~DeleteNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: InitializeNamedSelectionVariablesAction
//
// Purpose:
//   Handles ViewerRPC::InitializeNamedSelectionVariablesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API InitializeNamedSelectionVariablesAction : public SelectionActionBase
{
public:
    InitializeNamedSelectionVariablesAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~InitializeNamedSelectionVariablesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: LoadNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::LoadNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API LoadNamedSelectionAction : public SelectionActionBase
{
public:
    LoadNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~LoadNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SaveNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::SaveNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SaveNamedSelectionAction : public SelectionActionBase
{
public:
    SaveNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~SaveNamedSelectionAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetNamedSelectionAutoApplyAction
//
// Purpose:
//   Handles ViewerRPC::SetNamedSelectionAutoApplyRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetNamedSelectionAutoApplyAction : public SelectionActionBase
{
public:
    SetNamedSelectionAutoApplyAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~SetNamedSelectionAutoApplyAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: UpdateNamedSelectionAction
//
// Purpose:
//   Handles ViewerRPC::UpdateNamedSelectionRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API UpdateNamedSelectionAction : public SelectionActionBase
{
public:
    UpdateNamedSelectionAction(ViewerWindow *win) : SelectionActionBase(win) {}
    virtual ~UpdateNamedSelectionAction() {}

    virtual void Execute();
private:
    void UpdateNamedSelection(const std::string &selName, bool updatePlots,
                              bool allowCache);
};

#endif
