/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef VIEWER_TOOLBAR_H
#define VIEWER_TOOLBAR_H
#include <viewer_exports.h>
#include <string>
#include <map>

class QToolBar;
class ViewerActionBase;
class ViewerWindow;

// ****************************************************************************
// Class: ViewerToolbar
//
// Purpose:
//   Encapsulates ViewerWindow's toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 11:48:16 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 10:25:32 PDT 2003
//   I added the RemoveAction method.
//
// ****************************************************************************

class VIEWER_API ViewerToolbar
{
    struct ToolbarItem
    {
        ToolbarItem();
        ToolbarItem(const ToolbarItem &);
        ToolbarItem(QToolBar *, bool);
        virtual ~ToolbarItem();
        void operator = (const ToolbarItem &);
        QToolBar *toolbar;
        bool     visible;
    };
        
    typedef std::map<std::string, ToolbarItem> ToolbarMap;
public:
    ViewerToolbar(ViewerWindow *win);
    virtual ~ViewerToolbar();

    void Show(const std::string &toolBarName);
    void Hide(const std::string &toolBarName);
    void ShowAll();
    void HideAll();

    void AddAction(const std::string &toolBarName, ViewerActionBase *action);
    void RemoveAction(ViewerActionBase *action);
private:
    ToolbarItem CreateToolbar(const std::string &toolBarName);

    ViewerWindow *window;
    bool          hidden;
    ToolbarMap    toolbars;
};

#endif
