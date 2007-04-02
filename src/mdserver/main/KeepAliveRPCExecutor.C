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

#include <KeepAliveRPCExecutor.h>
#include <KeepAliveRPC.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: KeepAliveRPCExecutor::KeepAliveRPCExecutor
//
// Purpose: 
//   Constructor for the KeepAliveRPCExecutor class.
//
// Arguments:
//   s : A pointer to the KeepAliveRPC object that this executor observes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:22:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

KeepAliveRPCExecutor::KeepAliveRPCExecutor(Subject *s) : Observer(s)
{
}

// ****************************************************************************
// Method: KeepAliveRPCExecutor::~KeepAliveRPCExecutor
//
// Purpose: 
//   Destructor for the KeepAliveRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:22:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

KeepAliveRPCExecutor::~KeepAliveRPCExecutor()
{
}

// ****************************************************************************
// Method: KeepAliveRPCExecutor::Update
//
// Purpose: 
//   Does the work for the KeepAliveRPC.
//
// Arguments:
//   s : A pointer to the KeepAliveRPC that caused this method to be called.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:22:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
KeepAliveRPCExecutor::Update(Subject *s)
{
    debug3 << "KeepAliveRPCExecutor::Update" << endl;
    KeepAliveRPC *keepAlive = (KeepAliveRPC *)s;
    keepAlive->SendReply();
}
