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

#include <ObserverToCallback.h>
#include <Subject.h>

// ****************************************************************************
// Method: ObserverToCallback::ObserverToCallback
//
// Purpose: 
//   Constructor for the ObserverToCallback class.
//
// Arguments:
//   subj : The subject that the observer will observe.
//   cb   : The callback function to call when the subject updates.
//   cbd  : Data to pass to the callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:22:30 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ObserverToCallback::ObserverToCallback(Subject *subj,
    ObserverCallback *cb, void *cbd) : Observer(subj)
{
    cbFunction = cb;
    cbData = cbd;
}

// ****************************************************************************
// Method: ObserverToCallback::~ObserverToCallback
//
// Purpose: 
//   Destructor for the ObserverToCallback class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:23:50 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ObserverToCallback::~ObserverToCallback()
{
    cbFunction = 0;
    cbData = 0;
}

// ****************************************************************************
// Method: ObserverToCallback::Update
//
// Purpose: 
//   This method is called when the observer's subject is modified.
//
// Arguments:
//   subj : A pointer to the observer's subject.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:24:11 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ObserverToCallback::Update(Subject *subj)
{
    if(cbFunction)
    {
        (*cbFunction)(subj, cbData);
    }
}

// ****************************************************************************
// Method: ObserverToCallback::SetCallback
//
// Purpose: 
//   Sets the object's callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:25:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ObserverToCallback::SetCallback(ObserverCallback *cb)
{
    cbFunction = cb;
}

// ****************************************************************************
// Method: ObserverToCallback::SetCallbackData
//
// Purpose: 
//   Sets the object's callback function data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:25:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ObserverToCallback::SetCallbackData(void *cbd)
{
    cbData = cbd;
}
