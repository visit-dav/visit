package llnl.visit;

import java.io.IOException;

// ****************************************************************************
// Class: Xfer
//
// Purpose:
//   This class is a special observer that observes the state objects that
//   we use to communicate with the viewer. When the state objects change in
//   Java, Xfer is notified and it sends the state object that changed to the
//   viewer. When the viewer sends a state object to Java, Xfer makes sure
//   the state object reads itself and that it notifies all of its observers.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:38:47 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 12 12:47:12 PDT 2002
//   Added verbose flag to allow us to print trace information.
//
//   Brad Whitlock, Fri Jan 7 17:03:14 PST 2005
//   Added support for inserting null entries into the subjects list so
//   we can maintain the same object ordering as the viewer without
//   having to write a state object if we don't want to.
//
//   Brad Whitlock, Mon Jun 6 10:19:14 PDT 2005
//   I added code code to make the working thread sleep a little to reduce
//   workload on the CPU if nothing's getting processed.
//
// ****************************************************************************

class Xfer implements SimpleObserver, Runnable
{
    public Xfer()
    {
        subjects = new AttributeSubject[100];
        for(int i = 0; i < 100; ++i)
            subjects[i] = null;
        nSubjects = 0;

        doUpdate = true;
        header = new CommunicationBuffer();
        message = new CommunicationBuffer();
        input = new CommunicationBuffer();
        viewerInit = false;
        haveStoredHeader = false;
        length = -1;
        opcode = 0;
        processing = false;
        verbose = false;
    }

    public void Add(AttributeSubject subject)
    {
        subject.Attach(this);
        subjects[nSubjects] = subject;
        subject.SetAttributeId(nSubjects);
        ++nSubjects;
    }

    // This is a temporary method.
    public void Add(int index, AttributeSubject subject)
    {
        subject.Attach(this);
        subjects[index] = subject;
        subject.SetAttributeId(index);
    }

    // This is a temporary method that lets us add an empty item
    // into the subjects array so we have the same object ordering
    // as the viewer without having to necessarily write a state
    // object to do the job.
    public void AddDummy()
    {
        subjects[nSubjects] = null;
        ++nSubjects;
    }

    public void Remove(AttributeSubject subject)
    {
        subject.Detach(this);
    }

    public void SetRemoteProcess(RemoteProcess rp)
    {
        viewer = rp;
        viewerInit = true;
    }

    public void SetRemoteInfo(CommunicationHeader remoteInfo)
    {
        header.SetRemoteInfo(remoteInfo);
        message.SetRemoteInfo(remoteInfo);
        input.SetRemoteInfo(remoteInfo);
    }

    public void SetVerbose(boolean val)
    {
        verbose = val;
    }

    private synchronized void PrintMessage(String msg)
    {
        if(verbose)
            System.out.println(msg);
    }

    public synchronized void Update(AttributeSubject subject)
    {
        // Write the message
        subject.Write(message);

        // Write the message header
        header.WriteInt(subject.GetAttributeId());
        header.WriteInt(message.length);

        // Write to the socket.
        if(viewerInit)
        {
            try
            {
                int hlen = viewer.DirectWrite(header.GetBytes());
                int mlen = viewer.DirectWrite(message.GetBytes());
                PrintMessage("Sent " + (hlen+mlen) + " bytes to viewer.");
            }
            catch(IOException e)
            {
            }
        }

        header.Flush();
        message.Flush();
    }

    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }

    public void SendInterruption()
    {
        header.WriteInt(-1);
        header.WriteInt(0);
        try
        {
            int hlen = viewer.DirectWrite(header.GetBytes());
            PrintMessage("Sent " + hlen + " bytes to viewer.");
        }
        catch(IOException e)
        {
        }

        header.Flush();
    }

    public synchronized boolean Process() throws LostConnectionException
    {
        boolean retval = false;

        // Try and read from the viewer.
        int nbytes = viewer.CanRead();
        if(nbytes > 0)
        {
            try
            {
                byte[] b = viewer.DirectRead(nbytes);
                input.WriteByteArray(b, false);
            }
            catch(IOException e)
            {
            }
        }

        // While there are complete messages, read and process them.
        while(ReadHeader())
        {
//            if(opcode < nSubjects)
            if(opcode < 100 && subjects[opcode] != null)
            {
                 PrintMessage("Xfer::Process: "+
                              "class="+subjects[opcode].GetClassName()+
                              " opcode="+opcode+
                              " length="+length+ " bytes");

                 // Read the object into its local copy.
                 subjects[opcode].Read(input);

                 // Indicate that we want Xfer to ignore update messages if
                 // it gets them while processing the Notify.
                 SetUpdate(false);
                 subjects[opcode].Notify();
            }
            else
            {
                // Dispose of the message.
                PrintMessage("Xfer::Process: opcode="+opcode+
                             " disposed of "+length+ " bytes");
                input.Shift(length);
            }

            retval = true;
        }

        return retval;
    }

    public void StartProcessing()
    {
        if(!processing)
        {
            processing = true;
            new Thread(this).start();
        }
    }

    public void StopProcessing()
    {
        processing = false;
    }

    public boolean IsProcessing()
    {
        return processing;
    }

    private boolean ReadHeader()
    {
        boolean retval = false;

        // If the connection has more than the header in its buffer, try
        // reading the header. Also try if there is a stored header. That
        // means that we've tried to read for this operation before. Maybe
        // the message is all there this time.
        int sizeof_int = 4;
        if((input.Size() >= (2 * sizeof_int)) || haveStoredHeader)
        {
            if(!haveStoredHeader)
            {
                opcode = input.ReadInt();
                length = input.ReadInt();
            }

            // Check the message size against the size of what's actually
            // arrived in the buffer.
            retval = (length <= input.Size());
            haveStoredHeader = !retval;
        }

       return retval;
    }

    // This is a thread callback function that reads back from the viewer
    // and fills the input buffer. If we read any input from the viewer,
    // we process it too.
    public void run()
    {
        int idlecount = 0;
        while(processing && viewerInit)
        {
            try
            {
                if(!Process())
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
                        int timeout = 200;
                        if(idlecount > 50 && idlecount < 100)
                            timeout = 500;
                        else if(idlecount >= 100)
                            timeout = 1000;
                        else if(idlecount >= 500)
                            timeout = 2000;

                        try
                        {
                            Thread.currentThread().sleep(timeout);
                        }
                        catch(java.lang.InterruptedException e)
                        {
                            processing = false;
                        }
                    }
                }
                else
                    idlecount = 0;
            }
            catch(LostConnectionException e3)
            {
                processing = false;
            }
        }
    }

    private boolean             doUpdate;
    private CommunicationBuffer header;
    private CommunicationBuffer message;
    private CommunicationBuffer input;
    private AttributeSubject[]  subjects;
    private int                 nSubjects;
    private RemoteProcess       viewer;
    private boolean             viewerInit;
    private boolean             haveStoredHeader;
    private int                 length;
    private int                 opcode;
    private boolean             processing;
    private boolean             verbose;
}
