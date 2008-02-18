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

#ifndef VIEWERPASSWORDWINDOW_H
#define VIEWERPASSWORDWINDOW_H
#include <visit-config.h>
#include <qdialog.h>
#include <set>

// Forward declarations
class QLineEdit;
class QLabel;
class ViewerConnectionProgressDialog;

// ****************************************************************************
//  Class:  ViewerPasswordWindow
//
//  Purpose:
//    Main window for the program.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 25, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Aug 29 10:40:19 PDT 2002
//    I removed the Windows API stuff since this file is not part of the
//    viewer on Windows. I also added a new static function.
//
//    Brad Whitlock, Thu Aug 29 17:50:25 PST 2002
//    I added a userName argument to the getPassword and authenticate methods.
//
//    Jeremy Meredith, Thu May 24 10:57:02 EDT 2007
//    Added support for checking failed SSH tunneling (port forwards).
//
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//    Hank Childs, Sat Nov 10 11:31:34 PST 2007
//    Add a new button for changing the username.
//
//    Kathleen Bonnell, Wed Feb 13 14:05:03 PST 2008 
//    Added static methods to retrieve and reset the value of 
//    'needToChangeUsername'. 
//
// ****************************************************************************

class ViewerPasswordWindow : public QDialog
{
    Q_OBJECT
  public:
    ViewerPasswordWindow(QWidget *parent=NULL, const char *name=NULL);
    ~ViewerPasswordWindow();

    static const char *getPassword(const char *, const char *, bool = false);
    static const bool getNeedToChangeUsername() { return needToChangeUsername; }
    static void resetNeedToChangeUsername() { needToChangeUsername = false; }
    static void authenticate(const char *, const char *, int);
    static void SetConnectionProgressDialog(ViewerConnectionProgressDialog *d)
    {
        dialog = d;
    }

private slots:
    void changeUsername();

#if defined(PANTHERHACK)
// Broken on Panther
  private:
#else
    static std::set<int> GetFailedPortForwards();
  private:
    static std::set<int> failedPortForwards;
#endif
    QLineEdit *passedit;
    QLabel    *label;
    static bool       needToChangeUsername;
    static ViewerConnectionProgressDialog *dialog;
    static ViewerPasswordWindow *instance;
};

#endif
