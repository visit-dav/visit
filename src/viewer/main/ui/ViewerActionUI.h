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

#ifndef VIEWER_ACTION_UI_H
#define VIEWER_ACTION_UI_H
#include <viewer_exports.h>
#include <ViewerBaseUI.h>

class ViewerActionLogic;

class QIcon;
class QMenu;
class QToolBar;

// ****************************************************************************
// Class: ViewerActionUI
//
// Purpose:
//   This is an abstract base class for action UI's.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 14:46:56 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerActionUI : public ViewerBaseUI
{
    Q_OBJECT
public:
    ViewerActionUI(ViewerActionLogic *);
    virtual ~ViewerActionUI();

    virtual void Update() = 0;

    virtual bool Enabled() const           { return true;  }
    virtual bool Checked() const           { return false; }
    virtual bool MenuTopLevel() const      { return false; }
    virtual bool CanHaveOwnToolbar() const { return false; }
    virtual bool AllowInToolbar() const    { return true; }

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QMenu *menu) = 0;
    virtual void RemoveFromMenu(QMenu *menu) = 0;
    virtual void ConstructToolbar(QToolBar *toolbar) = 0;
    virtual void RemoveFromToolbar(QToolBar *toolbar) = 0;
    virtual void UpdateConstruction() = 0;

    ViewerActionLogic *GetLogic() const;
protected slots:
    virtual void Activate();
    virtual void ActivateEx(bool interactive);
protected:
    virtual void PreExecute();
    virtual void PostponeExecute();

    // Methods to set the action's attributes.
    virtual void SetText(const QString &text) = 0;
    virtual void SetMenuText(const QString &text) = 0;
    virtual void SetToolTip(const QString &text) = 0;
    virtual void SetIcon(const QIcon &icons) = 0;
    virtual void SetToggleAction(bool val);

    void         SetAllText(const QString &text);

    void         SetActiveAction(int);
    int          GetActiveAction() const;
private:
    ViewerActionLogic *logic;
    int                activeAction;
};

#endif
