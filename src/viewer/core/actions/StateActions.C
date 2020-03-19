// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <StateActions.h>

#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <AnnotationObject.h>
#include <HostProfileList.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddAnnotationObjectAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::AddAnnotationObjectRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
AddAnnotationObjectAction::Execute()
{
    windowMgr->AddAnnotationObject(args.GetIntArg1(), args.GetStringArg1());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteActiveAnnotationObjectsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::DeleteActiveAnnotationObjectsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
DeleteActiveAnnotationObjectsAction::Execute()
{
    windowMgr->DeleteActiveAnnotationObjects();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HideActiveAnnotationObjectsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::HideActiveAnnotationObjectsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
HideActiveAnnotationObjectsAction::Execute()
{
    windowMgr->HideActiveAnnotationObjects();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LowerActiveAnnotationObjectsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::LowerActiveAnnotationObjectsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
LowerActiveAnnotationObjectsAction::Execute()
{
    windowMgr->LowerActiveAnnotationObjects();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RaiseActiveAnnotationObjectsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::RaiseActiveAnnotationObjectsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
RaiseActiveAnnotationObjectsAction::Execute()
{
    windowMgr->RaiseActiveAnnotationObjects();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReadHostProfilesFromDirectoryAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ReadHostProfilesFromDirectoryRPC
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 15 15:00:47 PST 2014
//
// Modifications:
//   
// ****************************************************************************

void
ReadHostProfilesFromDirectoryAction::Execute()
{
    GetViewerStateManager()->ReadHostProfilesFromDirectory(args.GetDatabase(), args.GetBoolFlag());
    GetViewerState()->GetHostProfileList()->Notify();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetAnnotationAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetAnnotationAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetAnnotationAttributesAction::Execute()
{
    windowMgr->SetAnnotationAttsFromDefault();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetAnnotationObjectListAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetAnnotationObjectListRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetAnnotationObjectListAction::Execute()
{
    AnnotationObjectList legendPlusDefault;

    // Add the legends.
    for(int i = 0; i < GetViewerState()->GetAnnotationObjectList()->GetNumAnnotations(); ++i)
    {
        if(GetViewerState()->GetAnnotationObjectList()->GetAnnotation(i).GetObjectType() ==
           AnnotationObject::LegendAttributes)
        {
            legendPlusDefault.AddAnnotation(GetViewerState()->GetAnnotationObjectList()->GetAnnotation(i));
        }
    }
    // Add the defaults.
    for(int i = 0; i < windowMgr->GetDefaultAnnotationObjectList()->GetNumAnnotations(); ++i)
        legendPlusDefault.AddAnnotation(windowMgr->GetDefaultAnnotationObjectList()->GetAnnotation(i));

    // We should add an optional bool array to CreateAnnotationObjectsFromList that
    // lets us tell the routine whether to set the options for the newly created object.
    // That would let us create annotation objects but not use the attributes we have 
    // for them. The use case for that is resetting the legends to default values.
    GetWindow()->DeleteAllAnnotationObjects();
    GetWindow()->CreateAnnotationObjectsFromList(legendPlusDefault);
    windowMgr->UpdateAnnotationObjectList();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetInteractorAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetInteractorAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetInteractorAttributesAction::Execute()
{
    windowMgr->SetInteractorAttsFromDefault();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ResetLightListAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ResetLightListRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ResetLightListAction::Execute()
{
    windowMgr->SetLightListFromDefault();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetAnimationAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetAnimationAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetAnimationAttributesAction::Execute()
{
    windowMgr->SetAnimationAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetAnnotationAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetAnnotationAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetAnnotationAttributesAction::Execute()
{
    windowMgr->SetAnnotationAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetAnnotationObjectOptionsAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetAnnotationObjectOptionsRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetAnnotationObjectOptionsAction::Execute()
{
    windowMgr->SetAnnotationObjectOptions();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultAnnotationAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultAnnotationAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultAnnotationAttributesAction::Execute()
{
    ViewerWindowManager::SetDefaultAnnotationAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultAnnotationObjectListAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultAnnotationObjectListRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultAnnotationObjectListAction::Execute()
{
    ViewerWindowManager::SetDefaultAnnotationObjectListFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultInteractorAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultInteractorAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultInteractorAttributesAction::Execute()
{
    ViewerWindowManager::SetDefaultInteractorAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetDefaultLightListAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetDefaultLightListRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetDefaultLightListAction::Execute()
{
    ViewerWindowManager::SetDefaultLightListFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetInteractorAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetInteractorAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetInteractorAttributesAction::Execute()
{
    windowMgr->SetInteractorAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetKeyframeAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetKeyframeAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetKeyframeAttributesAction::Execute()
{
    windowMgr->SetKeyframeAttsFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetLightListAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetLightListRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetLightListAction::Execute()
{
    windowMgr->SetLightListFromClient();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetRenderingAttributesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetRenderingAttributesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetRenderingAttributesAction::Execute()
{
    windowMgr->SetRenderingAttributes();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: UpdateColorTableAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::UpdateColorTableRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Notes:      It is worth noting that this RPC may be called with the name
//             of a color table that does not exist. When that happens, it is
//             usually assumed to be the case that the default color table
//             as been deleted and has been set to a new value. This routine
//             is then called to update all plots which use the default color
//             table.
//
// Modifications:
//   
// ****************************************************************************

void
UpdateColorTableAction::Execute()
{
    windowMgr->UpdateColorTable(args.GetColorTableName());
}
