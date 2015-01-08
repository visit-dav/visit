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
#ifndef QUERY_ACTIONS_H
#define QUERY_ACTIONS_H
#include <ViewerActionLogic.h>

class ViewerQueryManager;

// ****************************************************************************
// Class: QueryActionBase
//
// Purpose:
//   Base class VIEWERCORE_API for query actions.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API QueryActionBase : public ViewerActionLogic
{
public:
    QueryActionBase(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~QueryActionBase() {}

protected:
    ViewerQueryManager *GetQueryManager();
};

// ****************************************************************************
// Class: ConstructDataBinningAction
//
// Purpose:
//   Handles ViewerRPC::ConstructDataBinningRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ConstructDataBinningAction : public QueryActionBase
{
public:
    ConstructDataBinningAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~ConstructDataBinningAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: GetQueryParametersAction
//
// Purpose:
//   Handles ViewerRPC::GetQueryParametersRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API GetQueryParametersAction : public QueryActionBase
{
public:
    GetQueryParametersAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~GetQueryParametersAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ProcessExpressionsAction
//
// Purpose:
//   Handles ViewerRPC::ProcessExpressionsRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ProcessExpressionsAction : public QueryActionBase
{
public:
    ProcessExpressionsAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~ProcessExpressionsAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: QueryAction
//
// Purpose:
//   Handles ViewerRPC::QueryRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API QueryAction : public QueryActionBase
{
public:
    QueryAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~QueryAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: RenamePickLabelAction
//
// Purpose:
//   Handles ViewerRPC::RenamePickLabelRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API RenamePickLabelAction : public QueryActionBase
{
public:
    RenamePickLabelAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~RenamePickLabelAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetLineoutColorAction
//
// Purpose:
//   Handles ViewerRPC::ResetLineoutColorRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetLineoutColorAction : public QueryActionBase
{
public:
    ResetLineoutColorAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~ResetLineoutColorAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetPickAttributesAction
//
// Purpose:
//   Handles ViewerRPC::ResetPickAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetPickAttributesAction : public QueryActionBase
{
public:
    ResetPickAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~ResetPickAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetPickLetterAction
//
// Purpose:
//   Handles ViewerRPC::ResetPickLetterRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetPickLetterAction : public QueryActionBase
{
public:
    ResetPickLetterAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~ResetPickLetterAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: ResetQueryOverTimeAttributesAction
//
// Purpose:
//   Handles ViewerRPC::ResetQueryOverTimeAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ResetQueryOverTimeAttributesAction : public QueryActionBase
{
public:
    ResetQueryOverTimeAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~ResetQueryOverTimeAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultPickAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultPickAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultPickAttributesAction : public QueryActionBase
{
public:
    SetDefaultPickAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetDefaultPickAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetDefaultQueryOverTimeAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetDefaultQueryOverTimeAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetDefaultQueryOverTimeAttributesAction : public QueryActionBase
{
public:
    SetDefaultQueryOverTimeAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetDefaultQueryOverTimeAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetGlobalLineoutAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetGlobalLineoutAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetGlobalLineoutAttributesAction : public QueryActionBase
{
public:
    SetGlobalLineoutAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetGlobalLineoutAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetPickAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetPickAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetPickAttributesAction : public QueryActionBase
{
public:
    SetPickAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetPickAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetQueryFloatFormatAction
//
// Purpose:
//   Handles ViewerRPC::SetQueryFloatFormatRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetQueryFloatFormatAction : public QueryActionBase
{
public:
    SetQueryFloatFormatAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetQueryFloatFormatAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetQueryOverTimeAttributesAction
//
// Purpose:
//   Handles ViewerRPC::SetQueryOverTimeAttributesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetQueryOverTimeAttributesAction : public QueryActionBase
{
public:
    SetQueryOverTimeAttributesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetQueryOverTimeAttributesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SetSuppressMessagesAction
//
// Purpose:
//   Handles ViewerRPC::SetSuppressMessagesRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SetSuppressMessagesAction : public QueryActionBase
{
public:
    SetSuppressMessagesAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SetSuppressMessagesAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: SuppressQueryOutputAction
//
// Purpose:
//   Handles ViewerRPC::SuppressQueryOutputRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API SuppressQueryOutputAction : public QueryActionBase
{
public:
    SuppressQueryOutputAction(ViewerWindow *win) : QueryActionBase(win) {}
    virtual ~SuppressQueryOutputAction() {}

    virtual void Execute();
};

#endif
