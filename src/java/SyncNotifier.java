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
