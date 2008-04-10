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

#ifndef QVIS_POSTABLE_WINDOW_OBSERVER_H
#define QVIS_POSTABLE_WINDOW_OBSERVER_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// ****************************************************************************
// Class: QvisPostableWindowObserver
//
// Purpose:
//   This is the base class for postable windows that observe state
//   objects and update themselves when the state objects change.
//   This class also has an Apply button in addition to Post and
//   Dismiss buttons.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:06:56 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 13:44:14 PST 2001
//   Added the "extra_" argument to the contructor.
//
//   Brad Whitlock, Wed May 2 11:32:41 PDT 2001
//   Replaced extra argument with buttonCombination.
//
//   Brad Whitlock, Thu Aug 23 8:31:23 PDT 2001
//   Added a private slot function to help with make default.
//
//   Brad Whitlock, Fri Feb 15 11:17:34 PDT 2002
//   Changed the protection on some of the methods.
//
//   Brad Whitlock, Fri Nov 7 16:10:58 PST 2003
//   I made it inherit from QvisPostableWindowSimpleObserver and I moved
//   some of the functionality there too.
//
//   Brad Whitlock, Wed Apr  9 10:48:01 PDT 2008
//   QString for caption and shortName.
//
// ****************************************************************************

class GUI_API QvisPostableWindowObserver : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisPostableWindowObserver(Subject *subj,
                               const QString &caption = QString::null,
                               const QString &shortName = QString::null,
                               QvisNotepadArea *notepad = 0,
                               int buttonCombo = AllExtraButtons,
                               bool stretch = true);
    virtual ~QvisPostableWindowObserver();

    virtual void SubjectRemoved(Subject *TheRemovedSubject);

public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents() = 0;
    Subject *subject;
};

#endif
