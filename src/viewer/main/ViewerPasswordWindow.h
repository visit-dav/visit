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

#ifndef VIEWERPASSWORDWINDOW_H
#define VIEWERPASSWORDWINDOW_H
#include <qdialog.h>

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
// ****************************************************************************

class ViewerPasswordWindow : public QDialog
{
    Q_OBJECT
  public:
    ViewerPasswordWindow(QWidget *parent=NULL, const char *name=NULL);
    ~ViewerPasswordWindow();

    static const char *getPassword(const char *, const char *, bool = false);
    static void authenticate(const char *, const char *, int);
    static void SetConnectionProgressDialog(ViewerConnectionProgressDialog *d)
    {
        dialog = d;
    }
  private:
    QLineEdit *passedit;
    QLabel    *label;
    static ViewerConnectionProgressDialog *dialog;
    static ViewerPasswordWindow *instance;
};

#endif
