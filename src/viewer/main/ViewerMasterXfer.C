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

#include <ViewerMasterXfer.h>

// ****************************************************************************
// Method: ViewerMasterXfer::ViewerMasterXfer
//
// Purpose: 
//   Constructor for the ViewerMasterXfer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:11:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ViewerMasterXfer::ViewerMasterXfer() : Xfer()
{
    updateCallback = 0;
    updateCallbackData = 0;
}

// ****************************************************************************
// Method: ViewerMasterXfer::~ViewerMasterXfer
//
// Purpose: 
//   Destructor for the ViewerMasterXfer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:11:38 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ViewerMasterXfer::~ViewerMasterXfer()
{
}

// ****************************************************************************
// Method: ViewerMasterXfer::Update
//
// Purpose: 
//   This method is called when the subjects attached to Xfer call their
//   Notify method.
//
// Arguments:
//   subj : A pointer to the subject that caused this method to get called.
//
// Note:       We call a special callback if one is defined. Otherwise we
//             call the base class's Update method.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:11:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMasterXfer::Update(Subject *subj)
{
    if(updateCallback != 0)
    {
        updateCallback(updateCallbackData, subj);
    }
    else
    {
        // Call the parent's Update method if we're not replacing it
        // with a callback function.
        Xfer::Update(subj);
    }
}

// ****************************************************************************
// Method: ViewerMasterXfer::SetUpdateCallback
//
// Purpose: 
//   Set up a special callback for the Update function.
//
// Arguments:
//   updateCB     : The callback function pointer.
//   updateCBData : The callback function data.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:13:24 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMasterXfer::SetUpdateCallback(void (*updateCB)(void *, Subject *),
    void *updateCBData)
{
    updateCallback = updateCB;
    updateCallbackData = updateCBData;
}

