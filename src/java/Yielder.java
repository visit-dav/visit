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

class Yielder
{
    public Yielder(int maxt)
    {
        idlecount = 0;
        maxtimeout = maxt;
    }

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

    public void reset()
    {
        idlecount = 0;
    }

    int idlecount;
    int maxtimeout;
}
