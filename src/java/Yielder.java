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

// ****************************************************************************
// Class: Yielder
//
// Purpose:
//   This class is used to yield the thread's cpu time until it ultimately
//   starts sleeping and waking on a timeout.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:50:24 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
/**
 * Yield the thread's cpu time until it ultimately starts sleeping and 
 * waking on a timeout.
 *
 * @author Brad Whitlock
 */
class Yielder
{ 
   /**
     * Constructor for the Yielder class.
     *
     * @param maxt The maximum timeout
     */
    public Yielder(int maxt)
    {
        idlecount = 0;
        maxtimeout = maxt;
    }

   /**
     * Yields the CPU so the thread does not do any work.
     *
     * @param maxt The maximum timeout
     */
    public void yield() throws java.lang.InterruptedException
    {
        ++idlecount;
        if(idlecount < 10)
        {
            // Yield to other threads.
            Thread.currentThread().yield();
        }
        else
        {
            // The thread has been idle for a while, sleep
            // instead of yield because sleep does not require
            // any work.
            int timeout = maxtimeout / 10;
            if(idlecount > 50 && idlecount < 100)
                timeout = maxtimeout / 4;
            else if(idlecount >= 100)
                timeout = maxtimeout / 2;
            else if(idlecount >= 500)
                timeout = maxtimeout;

            Thread.currentThread().sleep(timeout);
        }
    }

   /**
     * Resets the idle count.
     *
     * @param maxt The maximum timeout
     */
    public void reset()
    {
        idlecount = 0;
    }

    int idlecount;
    int maxtimeout;
}
