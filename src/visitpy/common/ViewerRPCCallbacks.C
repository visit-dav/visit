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
#include <ViewerRPCCallbacks.h>

// ****************************************************************************
// Method: ViewerRPCCallbacks::ViewerRPCCallbacks
//
// Purpose: 
//   Constructor for the ViewerRPCCallbacks class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:46:12 PST 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerRPCCallbacks::ViewerRPCCallbacks()
{
    for(int r = 0; r < (int)ViewerRPC::MaxRPC; ++r)
        pycb[r] = 0;
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::~ViewerRPCCallbacks
//
// Purpose: 
//   Destructor for the ViewerRPCCallbacks class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:46:12 PST 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerRPCCallbacks::~ViewerRPCCallbacks()
{
    for(int r = 0; r < (int)ViewerRPC::MaxRPC; ++r)
    {
        if(pycb[r] != 0)
            Py_DECREF(pycb[r]);
    }
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::GetCallbackNames
//
// Purpose: 
//   Get the callback names.
//
// Arguments:
//   names : The return vector for the callback names.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:46:40 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
ViewerRPCCallbacks::GetCallbackNames(stringVector &names)
{
    for(int r = 0; r < (int)ViewerRPC::MaxRPC; ++r)
        names.push_back(ViewerRPC::ViewerRPCType_ToString((ViewerRPC::ViewerRPCType)r));
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::RegisterCallback
//
// Purpose: 
//   Registers a callback for a specific viewerrpc (by name).
//
// Arguments:
//   rpcName : The name of the ViewerRPC for which we're installing a callback.
//   cb      : The user-provided callback.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:47:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPCCallbacks::RegisterCallback(const std::string &rpcName, PyObject *cb)
{
    bool retval = false;
    ViewerRPC::ViewerRPCType r;
    if(ViewerRPC::ViewerRPCType_FromString(rpcName, r))
    {
        if(pycb[r] != 0)
            Py_DECREF(pycb[r]);
        pycb[r] = cb;
        Py_INCREF(pycb[r]);
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::GetCallback
//
// Purpose: 
//   Returns the user-defined callback for the specified rpc.
//
// Arguments:
//   r : The rpc whose callback we want to access.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:48:55 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
ViewerRPCCallbacks::GetCallback(ViewerRPC::ViewerRPCType r)
{
    return pycb[r];
}
