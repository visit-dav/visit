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
#ifndef STATE_ACTIONS_H
#define STATE_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: AddAnnotationObjectAction
//
// Purpose:
//   Handles ViewerRPC::AddAnnotationObjectRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API AddAnnotationObjectAction : public ViewerActionLogic
{
public:
    AddAnnotationObjectAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~AddAnnotationObjectAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: DeleteActiveAnnotationObjectsAction
//
// Purpose:
//   Handles ViewerRPC::DeleteActiveAnnotationObjectsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API DeleteActiveAnnotationObjectsAction : public ViewerActionLogic
{
public:
    DeleteActiveAnnotationObjectsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~DeleteActiveAnnotationObjectsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: HideActiveAnnotationObjectsAction
//
// Purpose:
//   Handles ViewerRPC::HideActiveAnnotationObjectsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API HideActiveAnnotationObjectsAction : public ViewerActionLogic
{
public:
    HideActiveAnnotationObjectsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~HideActiveAnnotationObjectsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: LowerActiveAnnotationObjectsAction
//
// Purpose:
//   Handles ViewerRPC::LowerActiveAnnotationObjectsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API LowerActiveAnnotationObjectsAction : public ViewerActionLogic
{
public:
    LowerActiveAnnotationObjectsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~LowerActiveAnnotationObjectsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: RaiseActiveAnnotationObjectsAction
//
// Purpose:
//   Handles ViewerRPC::RaiseActiveAnnotationObjectsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RaiseActiveAnnotationObjectsAction : public ViewerActionLogic
{
public:
    RaiseActiveAnnotationObjectsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~RaiseActiveAnnotationObjectsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ReadHostProfilesFromDirectoryAction
//
// Purpose:
//   Handles ViewerRPC::ReadHostProfilesFromDirectoryRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 15 14:59:45 PST 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ReadHostProfilesFromDirectoryAction : public ViewerActionLogic
{
public:
    ReadHostProfilesFromDirectoryAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ReadHostProfilesFromDirectoryAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetAnnotationAttributesAction
//
// Purpose:
//   Handles ViewerRPC::ResetAnnotationAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetAnnotationAttributesAction : public ViewerActionLogic
{
public:
    ResetAnnotationAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetAnnotationAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetAnnotationObjectListAction
//
// Purpose:
//   Handles ViewerRPC::ResetAnnotationObjectListRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetAnnotationObjectListAction : public ViewerActionLogic
{
public:
    ResetAnnotationObjectListAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetAnnotationObjectListAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetInteractorAttributesAction
//
// Purpose:
//   Handles ViewerRPC::ResetInteractorAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetInteractorAttributesAction : public ViewerActionLogic
{
public:
    ResetInteractorAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetInteractorAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetLightListAction
//
// Purpose:
//   Handles ViewerRPC::ResetLightListRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetLightListAction : public ViewerActionLogic
{
public:
    ResetLightListAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetLightListAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetAnimationAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetAnimationAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetAnimationAttributesAction : public ViewerActionLogic
{
public:
    SetAnimationAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetAnimationAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetAnnotationAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetAnnotationAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetAnnotationAttributesAction : public ViewerActionLogic
{
public:
    SetAnnotationAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetAnnotationAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetAnnotationObjectOptionsAction
//
// Purpose:
//   Handles ViewerRPC::SetAnnotationObjectOptionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetAnnotationObjectOptionsAction : public ViewerActionLogic
{
public:
    SetAnnotationObjectOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetAnnotationObjectOptionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultAnnotationAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultAnnotationAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultAnnotationAttributesAction : public ViewerActionLogic
{
public:
    SetDefaultAnnotationAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultAnnotationAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultAnnotationObjectListAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultAnnotationObjectListRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultAnnotationObjectListAction : public ViewerActionLogic
{
public:
    SetDefaultAnnotationObjectListAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultAnnotationObjectListAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultInteractorAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultInteractorAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultInteractorAttributesAction : public ViewerActionLogic
{
public:
    SetDefaultInteractorAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultInteractorAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultLightListAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultLightListRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultLightListAction : public ViewerActionLogic
{
public:
    SetDefaultLightListAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultLightListAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetInteractorAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetInteractorAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetInteractorAttributesAction : public ViewerActionLogic
{
public:
    SetInteractorAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetInteractorAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetKeyframeAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetKeyframeAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetKeyframeAttributesAction : public ViewerActionLogic
{
public:
    SetKeyframeAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetKeyframeAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetLightListAction
//
// Purpose:
//   Handles ViewerRPC::SetLightListRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetLightListAction : public ViewerActionLogic
{
public:
    SetLightListAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetLightListAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetRenderingAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetRenderingAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetRenderingAttributesAction : public ViewerActionLogic
{
public:
    SetRenderingAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetRenderingAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: UpdateColorTableAction
//
// Purpose:
//   Handles ViewerRPC::UpdateColorTableRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API UpdateColorTableAction : public ViewerActionLogic
{
public:
    UpdateColorTableAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~UpdateColorTableAction() {}

    virtual void Execute();
};

#endif
