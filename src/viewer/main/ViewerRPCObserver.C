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

// ************************************************************************* //
//                             ViewerRPCObserver.C                           //
// ************************************************************************* //

#include <ViewerRPCObserver.h>

// ****************************************************************************
//  Method: ViewerRPCObserver constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Oct 27 14:47:26 PST 2000
//    I removed the code to this function since it is no longer needed.
//
//    Brad Whitlock, Mon Sep 17 11:11:59 PDT 2001
//    I changed the argument from ViewerRPC to Subject.
//
// ****************************************************************************

ViewerRPCObserver::ViewerRPCObserver(Subject *subj) : Observer(subj)
{
}

// ****************************************************************************
//  Method: ViewerRPCObserver destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerRPCObserver::~ViewerRPCObserver()
{
}

// ****************************************************************************
//  Method: ViewerRPCObserver::Update
//
//  Purpose:
//      Execute an RPC in the subject.
//
//  Arguments:
//      s       The subject which contains the rpc information.
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//      Eric Brugger, Wed Aug 16 12:19:04 PDT 2000
//      Modify the RPC callbacks to take a VisApplication as a second
//      argument.
//
//      Brad Whitlock, Fri Oct 27 14:45:29 PST 2000
//      I changed the code so it emits a Qt signal instead of calling a
//      callback function in the ViewerSubject.
//
//      Brad Whitlock, Mon Sep 17 11:11:11 PDT 2001
//      I generalized the function so it the subject does not have to be
//      a viewerRPC object.
//
// ****************************************************************************

void ViewerRPCObserver::Update(Subject *)
{
    // Tell the ViewerSubject to execute an RPC.
    emit executeRPC();
}
