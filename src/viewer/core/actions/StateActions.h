// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
