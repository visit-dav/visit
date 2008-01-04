// ****************************************************************************
//
// Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400142
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************

package llnl.visit;

import java.lang.Thread;

// ****************************************************************************
// Class: SyncNotifier
//
// Purpose:
//   This class is used to observe SyncAttributes in the ViewerProxy. It
//   notifies the thread that called ViewerProxy's Synchronize method that
//   is doing a wait().
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:50:24 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class SyncNotifier implements SimpleObserver
{
    public SyncNotifier()
    {
        doUpdate = true;
        verbose = true;
        thread = null;
        syncValue = -1;
    }

    public synchronized void NotifyThreadOnValue(Thread t, int val)
    {
        thread = t;
        syncValue = val;
    }

    public void Update(AttributeSubject s)
    {
        if(s == null || thread == null)
            return;

        SyncAttributes syncAtts = (SyncAttributes)s;
        if(syncAtts.GetSyncTag() == syncValue)
        {
            synchronized(thread)
            {
                if(verbose)
                    System.out.println("Received viewer sync.");
                thread.notify();
            }
        }
    }

    public void SetVerbose(boolean val) { verbose = val; }
    public void SetUpdate(boolean val)  { doUpdate = val; }
    public boolean GetUpdate()          { return doUpdate; }

    private boolean doUpdate;
    private boolean verbose;
    private Thread  thread;
    private int     syncValue;
}
