/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef QVIS_APPLICATION_H
#define QVIS_APPLICATION_H
#include <QApplication>

#ifdef Q_WS_MACX
class QEventLoop;
#endif

// ****************************************************************************
// Class: QvisApplication
//
// Purpose:
//   This class inherits from QApplication so it can redefine some event
//   handling methods on MacOS X. Otherwise it is the same as QApplication.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 4 10:17:21 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Oct 9 15:16:34 PST 2007
//   Changed signature for macEventFilter to match newer Qt method.
//
//   Brad Whitlock, Fri May 30 11:50:32 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisApplication : public QApplication
{
    Q_OBJECT
public:
    QvisApplication(int &argc, char **argv);
    QvisApplication(int &argc, char **argv, bool GUIenabled);
    virtual ~QvisApplication();
    
signals:
    void showApplication();
    void hideApplication();
private slots:
    void exitTheLoop();

#ifdef Q_WS_MACX
public:
    virtual bool macEventFilter(EventHandlerCallRef, EventRef);
    bool         needToMakeActive;
    QEventLoop  *eventLoop;
#endif
};
#endif
