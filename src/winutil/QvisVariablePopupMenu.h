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

#ifndef QVIS_VARIABLE_POPUP_MENU_H
#define QVIS_VARIABLE_POPUP_MENU_H
#include <winutil_exports.h>
#include <qpopupmenu.h>

// ****************************************************************************
// Class: QvisVariablePopupMenu
//
// Purpose:
//   This is a popup menu that knows what kind of plot it is for. This allows
//   us to use fewer signals, etc. and support plugins inside the plot manager.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 17:57:49 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 10:29:38 PDT 2002
//   Added a methods to return/set the plot type.
//
//   Brad Whitlock, Mon Mar 17 15:12:48 PST 2003
//   I changed the API.
//
// ****************************************************************************

class WINUTIL_API QvisVariablePopupMenu : public QPopupMenu
{
    Q_OBJECT
public:
    QvisVariablePopupMenu(int plotType_, QWidget *parent, const char *name);
    virtual ~QvisVariablePopupMenu();
    int getPlotType() const { return plotType; };
    void setPlotType(int p) { plotType = p; };
    void setVarPath(const QString &str) { varPath = str; };
signals:
    void activated(int plotType_, const QString &varName);
private slots:
    void activatedCaught(int index);
private:
    QString varPath;
    int     plotType;
};

#endif
