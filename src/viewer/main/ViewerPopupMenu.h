/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef VIEWER_POPUP_MENU_H
#define VIEWER_POPUP_MENU_H
#include <viewer_exports.h>
#include <string>
#include <map>

// Forward declares.
class QPopupMenu;
class ViewerActionBase;
class ViewerWindow;

// ****************************************************************************
// Class: ViewerPopupMenu
//
// Purpose:
//   This is the popup menu class for the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 4 15:37:09 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 10:09:39 PDT 2003
//   I added RemoveAction.
//
// ****************************************************************************

class VIEWER_API ViewerPopupMenu
{
    struct SubMenuInfo
    {
        SubMenuInfo();
        SubMenuInfo(const SubMenuInfo &);
        SubMenuInfo(QPopupMenu *, int);
        virtual ~SubMenuInfo();
        void operator = (const SubMenuInfo &);

        QPopupMenu *menu;
        int         menuId;
    };

    typedef std::map<std::string, SubMenuInfo> MenuMap;
public:
    ViewerPopupMenu(ViewerWindow *win);
    virtual ~ViewerPopupMenu();

    void ShowMenu();
    void HideMenu();
    void SetEnabled(bool val);

    void AddAction(ViewerActionBase *action);
    void AddAction(const std::string &menuName, ViewerActionBase *action);
    void RemoveAction(ViewerActionBase *action);
    void EnableMenu(const std::string &menuName);
    void DisableMenu(const std::string &menuName);
private:
    QPopupMenu   *CreateMenu(const std::string &name);

    QPopupMenu   *popup;
    ViewerWindow *window;
    MenuMap       menus;
};

#endif
