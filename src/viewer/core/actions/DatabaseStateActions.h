// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DATABASE_STATE_ACTIONS_H
#define DATABASE_STATE_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: ResetMaterialAttributesAction
//
// Purpose:
//   Handles ViewerRPC::ResetMaterialAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetMaterialAttributesAction : public ViewerActionLogic
{
public:
    ResetMaterialAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetMaterialAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetMeshManagementAttributesAction
//
// Purpose:
//   Handles ViewerRPC::ResetMeshManagementAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetMeshManagementAttributesAction : public ViewerActionLogic
{
public:
    ResetMeshManagementAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~ResetMeshManagementAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetBackendTypeAction
//
// Purpose:
//   Handles ViewerRPC::SetBackendTypeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetBackendTypeAction : public ViewerActionLogic
{
public:
    SetBackendTypeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetBackendTypeAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetCreateMeshQualityExpressionsAction
//
// Purpose:
//   Handles ViewerRPC::SetCreateMeshQualityExpressionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetCreateMeshQualityExpressionsAction : public ViewerActionLogic
{
public:
    SetCreateMeshQualityExpressionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetCreateMeshQualityExpressionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetCreateTimeDerivativeExpressionsAction
//
// Purpose:
//   Handles ViewerRPC::SetCreateTimeDerivativeExpressionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetCreateTimeDerivativeExpressionsAction : public ViewerActionLogic
{
public:
    SetCreateTimeDerivativeExpressionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetCreateTimeDerivativeExpressionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetCreateVectorMagnitudeExpressionsAction
//
// Purpose:
//   Handles ViewerRPC::SetCreateVectorMagnitudeExpressionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetCreateVectorMagnitudeExpressionsAction : public ViewerActionLogic
{
public:
    SetCreateVectorMagnitudeExpressionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetCreateVectorMagnitudeExpressionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultFileOpenOptionsAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultFileOpenOptionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultFileOpenOptionsAction : public ViewerActionLogic
{
public:
    SetDefaultFileOpenOptionsAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultFileOpenOptionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultMaterialAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultMaterialAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultMaterialAttributesAction : public ViewerActionLogic
{
public:
    SetDefaultMaterialAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultMaterialAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultMeshManagementAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultMeshManagementAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultMeshManagementAttributesAction : public ViewerActionLogic
{
public:
    SetDefaultMeshManagementAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetDefaultMeshManagementAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetMaterialAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetMaterialAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetMaterialAttributesAction : public ViewerActionLogic
{
public:
    SetMaterialAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetMaterialAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetMeshManagementAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetMeshManagementAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetMeshManagementAttributesAction : public ViewerActionLogic
{
public:
    SetMeshManagementAttributesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetMeshManagementAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetPrecisionTypeAction
//
// Purpose:
//   Handles ViewerRPC::SetPrecisionTypeRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPrecisionTypeAction : public ViewerActionLogic
{
public:
    SetPrecisionTypeAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetPrecisionTypeAction() {}

    virtual void Execute();
};


// ****************************************************************************
// Class: SetRemoveDuplicateNodesAction
//
// Purpose:
//   Handles ViewerRPC::SetRemoveDuplicateNodesRPC
//
// Programmer: Kathleen Biagas
// Creation:   December 18, 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API SetRemoveDuplicateNodesAction : public ViewerActionLogic
{
public:
    SetRemoveDuplicateNodesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetRemoveDuplicateNodesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetTreatAllDBsAsTimeVaryingAction
//
// Purpose:
//   Handles ViewerRPC::SetTreatAllDBsAsTimeVaryingRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetTreatAllDBsAsTimeVaryingAction : public ViewerActionLogic
{
public:
    SetTreatAllDBsAsTimeVaryingAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetTreatAllDBsAsTimeVaryingAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetTryHarderCyclesTimesAction
//
// Purpose:
//   Handles ViewerRPC::SetTryHarderCyclesTimesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetTryHarderCyclesTimesAction : public ViewerActionLogic
{
public:
    SetTryHarderCyclesTimesAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetTryHarderCyclesTimesAction() {}

    virtual void Execute();
};

#endif
