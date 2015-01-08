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
