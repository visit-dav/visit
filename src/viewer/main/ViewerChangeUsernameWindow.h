/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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

#ifndef VIEWERCHANGEUSERNAMEWINDOW_H
#define VIEWERCHANGEUSERNAMEWINDOW_H
#include <VisItChangeUsernameWindow.h>

#include <string>

// ****************************************************************************
//  Class:  ViewerChangeUsernameWindow
//
//  Purpose:
//     A window where the user can set the username for a host.
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb 13 14:05:03 PST 2008
//    Added static getUsername method.
//
//    Brad Whitlock, Fri May 23 10:36:48 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Tue Jun 12 15:55:34 PST 2012
//    I made it inherit from VisItChangeUsernameWindow.
//
// ****************************************************************************

class ViewerChangeUsernameWindow : public VisItChangeUsernameWindow
{
    Q_OBJECT
public:
    ViewerChangeUsernameWindow(QWidget *parent = 0);
    virtual ~ViewerChangeUsernameWindow();

    static bool changeUsername(const std::string &hostname, std::string& username);
    static bool changeUsername(const std::string &hostname);

private:
    std::string username;
    static ViewerChangeUsernameWindow *instance;
};

#endif
