// ****************************************************************************
//
// Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-442911
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
// Class: EventLoop
//
// Purpose:
//   This class serves as an event loop that can be used to read input from
//   the viewer. It also provides a method for synchronization with the viewer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 25 15:37:49 PST 2011
//
// Modifications:
//   
// ****************************************************************************

public class EventLoop extends java.lang.Object implements SimpleObserver
{
    /**
     * Constructor for the EventLoop class.
     * @param x The xfer object that will be used to talk to the viewer.
     * @param m The message attributes object we'll use to detect error messages.
     * @param s The sync attributes
     */
    public EventLoop(Xfer x, MessageAttributes m, SyncAttributes s)
    {
        super();
        keepGoing = true;
        waitValue = -1;
        multithread = false;

        xfer = x;
        messageAtts = m;
        syncAtts = s;
    }

    /**
     * Copy constructor that we can use to initialize a new object.
     * @param obj The event loop that will be used to initialize the new one.
     */
    public EventLoop(EventLoop obj)
    {
        super();
        keepGoing = true;
        waitValue = -1;

        multithread = obj.multithread;
        xfer = obj.xfer;
        messageAtts = obj.messageAtts;
        syncAtts = obj.syncAtts;
    }

    /**
     * Execute the event loop, reading input from the viewer as it comes in.
     */
    public void Execute()
    {
        boolean processing = true;
        keepGoing = true;
        while(processing && keepGoing)
        {
            try
            {
                // Let Xfer process its input.
                if(!Process())
                {
                    // No input so sleep a little so we don't hog the CPU.
                    try
                    {
                        Thread.currentThread().sleep(100);
                    }
                    catch(java.lang.InterruptedException e)
                    {
                        // ignore it.
                    }
                }
            }
            catch(LostConnectionException e3)
            {
                processing = false;
            }
        }
    }

    /**
     * Expose Process method so we can call it periodically from event-driven
     * programs without having to do another thread. This method does nothing
     * if we've started processing input on another thread.
     *
     * @return True if there was input to read; otherwise false.
     */
    public synchronized boolean Process() throws LostConnectionException
    {
        return multithread ? false : xfer.Process();
    }

    /**
     * Send a synchronize message to the viewer and wait for it to come back.
     * If running a multithreaded event loop, this method should only be called
     * on thread 1 and never on the read thread, thread 2.
     *
     * @return True if the synchronize had no error; False if there was an error.
     */
    public synchronized boolean Synchronize()
    {
        return multithread ? SynchronizeMT() : SynchronizeST();
    }

    /**
     * Send a synchronize message to the viewer and process wait for it to come back.
     * @return True if the synchronize had no error; False if there was an error.
     */
    private boolean SynchronizeST()
    {
        // Send the wait value to the viewer.
        waitValue = syncCount++;
        syncAtts.SetSyncTag(waitValue);
        syncAtts.Notify();
        syncAtts.SetSyncTag(-1);

        errorFlag = false;

        messageAtts.Attach(this);
        syncAtts.Attach(this);

        // Wait for the wait value to return from the viewer.
        Execute();

        messageAtts.Detach(this);
        syncAtts.Detach(this);
 
        return !errorFlag;
    }

    /**
     * Start processing input from the viewer on a 2nd thread so it automatically
     * comes in and we don't have to explicitly do it ourselves.
     */
    public void StartProcessing()
    {
        multithread = true;
        xfer.StartProcessing();
    }

    /**
     * Stop processing input from the viewer on a 2nd thread.
     */
    public void StopProcessing()
    {
        multithread = false;
        xfer.StopProcessing();
    }

    /**
     * Send a synchronize message to the viewer and process wait for it to come back.
     * This method works when we are automatically reading viewer input on a 2nd 
     * thread. In that case, we send the synchronization and block this thread until
     * thread 2 notifies this thread that the value has returned.
     *
     * @return True if the synchronize had no error; False if there was an error.
     */
    private boolean SynchronizeMT()
    {
        waitValue = syncCount++;

        // Create a sync notifier that will execute on thread 2 and notify
        // this thread when it gets the wait value.
        SyncNotifier syncNotifier = new SyncNotifier();
        syncNotifier.NotifyThreadOnValue(Thread.currentThread(), waitValue);
        syncNotifier.SetUpdate(false);
        syncAtts.Attach(syncNotifier);
 
        // Send the wait value to the viewer.
        syncAtts.SetSyncTag(waitValue);
        syncAtts.Notify();
        syncAtts.SetSyncTag(-1);

        // Wait until the syncNotifier notifies us that it's okay to proceed.
        errorFlag = false;
        try
        {
            synchronized(Thread.currentThread())
            {
                Thread.currentThread().wait();
            }
        }
        catch(java.lang.InterruptedException e)
        {
            errorFlag = true;
        }

        syncAtts.Detach(syncNotifier);

        return !errorFlag;
    }

    //
    // SimpleObserver interface methods
    //

    /**
     * This method is called when syncAttributes or messageAttributes update
     * from the viewer.
     * @param subj The subject that caused the update.
     */
    public void Update(AttributeSubject subj)
    {
        if(subj == messageAtts)
        {
            if(messageAtts.GetSeverity() == MessageAttributes.SEVERITY_ERROR)
                errorFlag = true;
        }
        else if(subj == syncAtts)
        {
            if(syncAtts.GetSyncTag() == waitValue)
                keepGoing = false;
        }
    }
    public void SetUpdate(boolean val) { }
    public boolean GetUpdate() { return true; }

    private boolean           keepGoing;
    private boolean           errorFlag;
    private int               waitValue;
    private Xfer              xfer;
    private MessageAttributes messageAtts;
    private SyncAttributes    syncAtts;
    private boolean           multithread;

    private static int        syncCount = 100;
}
