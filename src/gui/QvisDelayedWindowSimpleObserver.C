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

#include <QvisDelayedWindowSimpleObserver.h>

// ****************************************************************************
// Method: QvisDelayedWindowSimpleObserver::QvisDelayedWindowSimpleObserver
//
// Purpose:
//   This is the constructor for the QvisDelayedWindowSimpleObserver class.
//
// Arguments:
//   caption : The name of the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:42:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 30 07:50:41 PDT 2002
//   I added window flags.
//
//   Brad Whitlock, Wed Apr  9 10:31:03 PDT 2008
//   Changed ctor args.
//
// **************************************************************************** 

QvisDelayedWindowSimpleObserver::QvisDelayedWindowSimpleObserver(
    const QString &caption, WFlags f) : QvisDelayedWindow(caption, f),
    SimpleObserver()
{
    selectedSubject = 0;
}

// ****************************************************************************
// Method: QvisDelayedWindowSimpleObserver::~QvisDelayedWindowSimpleObserver
//
// Purpose:
//   The destructor for the QvisDelayedWindowSimpleObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:42:10 PDT 2001
//
// Modifications:
//
// **************************************************************************** 

QvisDelayedWindowSimpleObserver::~QvisDelayedWindowSimpleObserver()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisDelayedWindowSimpleObserver::Udpate
//
// Purpose:
//   Updates the window when one of its subjects is updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:42:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 12 12:15:24 PDT 2002
//   I added the selectedSubject member.
//
// **************************************************************************** 

void
QvisDelayedWindowSimpleObserver::Update(Subject *subj)
{
    // This is what derived types will query to determine which subject
    // was modified.
    selectedSubject = subj;

    if(isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisDelayedWindowSimpleObserver::SubjectRemoved
//
// Purpose: 
//   This method is called when one of the Window's subjects is deleted.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject being removed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 12:17:41 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindowSimpleObserver::SubjectRemoved(Subject *TheRemovedSubject)
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisDelayedWindowSimpleObserver::SelectedSubject
//
// Purpose:
//   Returns which subject is currently being updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:42:10 PDT 2001
//
// Modifications:
//
// **************************************************************************** 

Subject *
QvisDelayedWindowSimpleObserver::SelectedSubject()
{
    return selectedSubject;
}
