/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef VIEWER_ACTION_BASE_H
#define VIEWER_ACTION_BASE_H
#include <viewer_exports.h>
#include <qobject.h>
#include <ViewerRPC.h>

class DataNode;
class QPopupMenu;
class QToolBar;
class ViewerWindow;
class ViewerWindowManager;

// ****************************************************************************
// Class: ViewerActionBase
//
// Purpose:
//   This is an abstract base class for actions that the viewer can perform.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 09:44:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 09:36:58 PDT 2003
//   I added the UpdateConstruction, RemoveFromMenu, RemoveFromToolbar methods.
//
//   Brad Whitlock, Tue Jul 1 10:17:46 PDT 2003
//   I added CreateNode and SetFromNode.
//
//   Brad Whitlock, Fri Apr 15 09:10:55 PDT 2005
//   I added SetRPCType, GetArgs.
//
//   Brad Whitlock, Wed Apr 27 15:13:38 PST 2005
//   I added CopyFrom.
//
// ****************************************************************************

class VIEWER_API ViewerActionBase : public QObject
{
    Q_OBJECT
public:
    ViewerActionBase(ViewerWindow *win, const char *name = 0);
    virtual ~ViewerActionBase();
    
    ViewerWindow *GetWindow() const        { return window; }
    const char *GetName() const            { return name(); }

    virtual void Execute() = 0;
    virtual void Update() = 0;

    virtual bool CopyFrom(const ViewerActionBase *)
                                           { return false; }

    virtual bool Enabled() const           { return true; }
    virtual bool VisualEnabled() const     { return allowVisualRepresentation; }
    virtual bool MenuTopLevel() const      { return false; }
    virtual bool CanHaveOwnToolbar() const { return false; }
    virtual bool AllowInToolbar() const    { return true; }

    virtual bool CreateNode(DataNode *)    { return false; }
    virtual void SetFromNode(DataNode *)   { }

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QPopupMenu *menu) = 0;
    virtual void RemoveFromMenu(QPopupMenu *menu) = 0;
    virtual void ConstructToolbar(QToolBar *toolbar) = 0;
    virtual void RemoveFromToolbar(QToolBar *toolbar) = 0;
    virtual void UpdateConstruction() { }

    void         SetRPCType(ViewerRPC::ViewerRPCType);

    static  void SetArgs(const ViewerRPC &a);
    static  const ViewerRPC &GetArgs();
public slots:
    virtual void Activate();
    virtual void Activate(bool setup);
protected:
    virtual void Setup() = 0;
    virtual void PreExecute() { }

    // Methods to set the action's attributes.
    virtual void SetText(const char *text) = 0;
    virtual void SetMenuText(const char *text) = 0;
    virtual void SetToolTip(const char *text) = 0;
    virtual void DisableVisual()       { allowVisualRepresentation = false; }
    virtual void EnableVisual()        { allowVisualRepresentation = true; }

    ViewerWindow               *window;
    int                         windowId;
    bool                        allowVisualRepresentation;
    ViewerRPC::ViewerRPCType    rpcType;

    static ViewerWindowManager *windowMgr;
    static ViewerRPC            args;
};

#endif
