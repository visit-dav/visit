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

#ifndef VIEWER_LOCK_ACTIONS_UI_H
#define VIEWER_LOCK_ACTIONS_UI_H
#include <viewer_exports.h>
#include <ViewerActionUIToggle.h>

// ****************************************************************************
// Class: ToggleAllowPopupActionUI
//
// Purpose:
//   Handles the toggle allow popup action.
//
// Notes:
//
// Programmer: Marc Durant
// Creation:   Tue Jan 10 09:18:00 MST 2012
//
// ****************************************************************************

class VIEWER_API ToggleAllowPopupActionUI : public ViewerActionUIToggle
{
public:
    ToggleAllowPopupActionUI(ViewerActionLogic *L);
    virtual ~ToggleAllowPopupActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleLockViewActionUI
//
// Purpose:
//   Handles the lock view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:44:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockViewActionUI : public ViewerActionUIToggle
{
public:
    ToggleLockViewActionUI(ViewerActionLogic *L);
    virtual ~ToggleLockViewActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleLockTimeActionUI
//
// Purpose:
//   Handles the lock time action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 18 17:12:50 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockTimeActionUI : public ViewerActionUIToggle
{
public:
    ToggleLockTimeActionUI(ViewerActionLogic *L);
    virtual ~ToggleLockTimeActionUI(){}

    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleLockToolActionUI
//
// Purpose:
//   Handles the lock tool action.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February 15, 2008
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockToolActionUI : public ViewerActionUIToggle
{
public:
    ToggleLockToolActionUI(ViewerActionLogic *L);
    virtual ~ToggleLockToolActionUI(){}

    virtual bool Checked() const;
};

// ****************************************************************************
// Class: TurnOffAllLocksActionUI
//
// Purpose:
//   Handles the turn off all locks action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:33:47 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API TurnOffAllLocksActionUI : public ViewerActionUISingle
{
public:
    TurnOffAllLocksActionUI(ViewerActionLogic *L);
    virtual ~TurnOffAllLocksActionUI(){}

    virtual bool AllowInToolbar() const { return false; }
};

#endif
