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

#ifndef QVIS_DELAYED_WINDOW_SIMPLEOBSERVER_H
#define QVIS_DELAYED_WINDOW_SIMPLEOBSERVER_H
#include <gui_exports.h>
#include <QvisDelayedWindow.h>
#include <SimpleObserver.h>

// ****************************************************************************
// Class: QvisDelayedWindowObserver
//
// Purpose:
//   This is the base class for a non-postable window that observes
//   multiple subjects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 15:53:55 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 12 12:16:08 PDT 2002
//   I added the ability for derived classes to query which subject caused
//   the call to the UpdateWindow method.
//
//   Brad Whitlock, Mon Sep 30 07:49:25 PDT 2002
//   I added window flags.
//
// ****************************************************************************

class GUI_API QvisDelayedWindowSimpleObserver : public QvisDelayedWindow, public SimpleObserver
{
    Q_OBJECT
public:
    QvisDelayedWindowSimpleObserver(const char *caption = 0, WFlags f = 0);
    virtual ~QvisDelayedWindowSimpleObserver();
    virtual void CreateWindowContents() = 0;
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
protected:
    Subject *SelectedSubject();
private:
    Subject *selectedSubject;
};

#endif
