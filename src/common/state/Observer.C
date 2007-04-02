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

#include <Observer.h>
#include <Subject.h>

// *******************************************************************
// Method: Observer::Observer
//
// Purpose: 
//   Constructor for the Observer class. It registers the observer
//   with the subject that is passed in.
//
// Arguments:
//     s : This is a pointer to a valid subject that we want to observe.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 6 10:43:37 PDT 2000
//
// Modifications:
//   
// *******************************************************************

Observer::Observer(Subject *s) : SimpleObserver()
{
    subject = s;
    subject->Attach(this);
}

// *******************************************************************
// Method: Observer::~Observer
//
// Purpose: 
//   Destructor for the Observer class. Detaches this observer from
//   the subject that it is watching.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 6 10:44:47 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 31 15:05:59 PST 2000
//   I changed the code so it does not try to detach from the subject.
//   The subject told the observer that it was destroyed.
//
// *******************************************************************

Observer::~Observer()
{
    if(subject != 0)
    {
        subject->Detach(this);
        subject = 0;
    }
}

// *******************************************************************
// Method: Observer::SubjectRemoved
//
// Purpose: 
//   Indicates that the subject that the Observer was watching is
//   not around anymore.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 15:20:45 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Observer::SubjectRemoved(Subject *)
{
    subject = 0;
}
