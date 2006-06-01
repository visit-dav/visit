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

#include <QvisPostableWindowObserver.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <AttributeSubject.h>

// ****************************************************************************
// Method: QvisPostableWindowObserver::QvisPostableWindowObserver
//
// Purpose: 
//   Constructor for the QvisPostableWindowObserver class. It
//   passes construction responsibility to the superclasses.
//
// Arguments:
//   caption : A string containing the name of the window.
//   notepad : The notepad that the window will post to.
//   subj    : The subject that the window is observing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:10:59 PST 2000
//
// Modifications:
//    Brad Whitlock, Fri Nov 7 16:23:53 PST 2003
//    I made it inherit from QvisPostableWindowSimpleObserver.
//
// ****************************************************************************

QvisPostableWindowObserver::QvisPostableWindowObserver(
    Subject *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad, int buttonCombo, bool stretch) : 
    QvisPostableWindowSimpleObserver(caption, shortName, notepad, buttonCombo,
        stretch)
{
    subject = subj;
    subject->Attach(this);
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::~QvisPostableWindowObserver
//
// Purpose: 
//   Destructor for the QvisPostableWindowObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:11:41 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPostableWindowObserver::~QvisPostableWindowObserver()
{
    if(subject != 0)
        subject->Detach(this);
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::SubjectRemoved
//
// Purpose: 
//   This method is called when the subject is deleted so we can set the 
//   selectedSubject pointer to zero so we don't try and detach from the
//   subject in this window's destructor.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject being removed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 16:25:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowObserver::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(subject == TheRemovedSubject)
        subject = 0;
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::apply
//
// Purpose: 
//   This is a Qt slot function that forces the subject that is being
//   observed to notify its observers if it has changed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 18:28:49 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 7 16:53:29 PST 2003
//   I made it use the selectedSubject pointer.
//
// ****************************************************************************

void
QvisPostableWindowObserver::apply()
{
    AttributeSubject *as = (AttributeSubject *)selectedSubject;

    // If the AttributeSubject that we're observing has fields
    // that are selected, make it notify all its observers.
    if(as->NumAttributesSelected() > 0)
    {
        as->Notify();
    }
}

