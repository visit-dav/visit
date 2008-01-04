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

#include <QvisVariablePopupMenu.h>

// ****************************************************************************
// Method: QvisVariablePopupMenu::QvisVariablePopupMenu
//
// Purpose: 
//   Constructor for the QvisVariablePopupMenu class.
//
// Arguments:
//   plotType_ : The plotType that will be passed along in the extended 
//               activate signal.
//   parent    : The widget's parent widget.
//   name      : The name of the popup menu.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 11:29:16 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 11:17:57 PDT 2002
//   Added initialization of varPath member.
//
// ****************************************************************************

QvisVariablePopupMenu::QvisVariablePopupMenu(int plotType_, QWidget *parent,
    const char *name) : QPopupMenu(parent, name), varPath("")
{
    plotType = plotType_;

    // Connect a slot to QPopupMenu's activate signal so we can re-emit a
    // signal that has more information.
    connect(this, SIGNAL(activated(int)),
            this, SLOT(activatedCaught(int)));
}

// ****************************************************************************
// Method: QvisVariablePopupMenu::~QvisVariablePopupMenu
//
// Purpose: 
//   Destructor for the QvisVariablePopupMenu class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 11:30:25 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisVariablePopupMenu::~QvisVariablePopupMenu()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisVariablePopupMenu::activatedCaught
//
// Purpose: 
//   This is a Qt slot function that catches QPopupMenu's activated slot and
//   re-emits it with the plotType and variable name.
//
// Arguments:
//   index : The index of the menu item that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 11:30:48 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 11:19:02 PDT 2002
//   Modified to support cascading variable menus.
//
// ****************************************************************************

void
QvisVariablePopupMenu::activatedCaught(int index)
{
    // Re-emit the signal with different arguments.
    QString completeVar(varPath + text(index));
    emit activated(plotType, completeVar);
}
