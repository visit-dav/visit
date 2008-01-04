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

import java.io.File;
import java.io.IOException;
import java.lang.Integer;
import java.lang.Process;
import java.lang.Runtime;
import java.util.Vector;

// ****************************************************************************
// Class: RemoteProcess
//
// Purpose:
//   This class contains the code to actually launch a VisIt process and
//   exchange information with it via sockets.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:48:26 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 12 12:26:30 PDT 2002
//   Added verbose printing to help debug the client.
//
//   Brad Whitlock, Thu Jan 2 13:55:40 PST 2003
//   I added a security key.
//
//   Brad Whitlock, Tue Jul 29 11:16:19 PDT 2003
//   I removed the code that put -nread and -nwrite on the command line.
//
// ****************************************************************************

class RemoteProcess implements Runnable
{
//
// public methods
//

    public RemoteProcess(String exename)
    {
        exeName = new String(exename);
        binPath = new String("");
        conn = new Connection();
        args = new Vector();
        viewerLaunched = false;
        waitingForLaunch = true;
        verbose = false;
    }

    public void AddArgument(String arg)
    {
        args.addElement(arg);
    }

    public void SetBinPath(String path)
    {
        binPath = new String(path);
    }

    public void SetVerbose(boolean val)
    {
        verbose = val;
    }

    public void Close()
    {
        conn.Close();

        if(viewerLaunched)
            viewer.destroy();

        viewerLaunched = false;
    }

    public int DirectWrite(byte[] buf) throws IOException
    {
        conn.DirectWrite(buf);
        return buf.length;
    }

    public byte[] DirectRead(int ntotal) throws LostConnectionException, IOException
    {
        return conn.DirectRead(ntotal);
    }
   
    public int CanRead()
    {
        return (viewerLaunched && !waitingForLaunch) ? conn.CanRead() : 0;
    }

    public boolean Open(int port)
    {
        if(!StartMakingConnection(port))
            return false;

        if(!LaunchLocal())
        {
            Close();
            return false;
        }

        if(!FinishMakingConnections())
        {
            Close();
            return false;
        }

        return true;
    }

//
// private methods
//

    private boolean StartMakingConnection(int port)
    {
        PrintMessage("Starting to make connection to viewer.");
        return conn.StartConnection(port);
    }

    private boolean FinishMakingConnections()
    {
        PrintMessage("Finishing making connection to viewer.");
        return conn.CreateSockets();
    }

    private synchronized void PrintMessage(String msg)
    {
        if(verbose)
            System.out.println(msg);
    }

    private boolean LaunchLocal()
    {
        args.addElement("-host");
        args.addElement(conn.GetLocalHostName());
        args.addElement("-port");
        args.addElement(new Integer(conn.GetPort()));
        args.addElement("-key");
        args.addElement(conn.GetSecurityKey());

        // Call LaunchHelper on another thread.
        new Thread(this).start();

        // Wait to see if the launch was a success and get the
        // message that was returned.
        String msg = WaitForLaunch();

        // Compare the return message against known VisIt launch messages.
        // If the message has the word "Running" at the start then the
        // viewer was successfully launched. Otherwise we did not really
        // run the viewer.
        boolean success = msg.startsWith("Running");
        if(success)
            System.out.print(msg);
        else
            PrintMessage("The viewer could not launch: " + msg);

        return success;
    }

    private void LaunchHelper() throws java.io.IOException
    {
        // Create a command string.
        String command = new String("");
        if(binPath.length() > 0)
        {
            command = binPath + File.separator;
        }
        command = command + exeName;

        for(int i = 0; i < args.size(); ++i)
            command = command + " " + args.elementAt(i);

        PrintMessage("Launching viewer...");

        // Launch the viewer.
        viewer = Runtime.getRuntime().exec(command);
        viewerLaunched = true;

        // Wait for the viewer to terminate.
        try
        {
            viewer.waitFor();
        }
        catch(java.lang.InterruptedException e)
        {
            System.out.println("Interrupted!");
        }
    }

    // This is a thread callback function that launches the viewer.
    public void run()
    {
        try
        {
            LaunchHelper();
        }
        catch(java.io.IOException e)
        {
            // Terminate the loop where we are trying to read from the
            // process object.
            waitingForLaunch = false;

            System.out.println("Cannot launch VisIt.\n");
        }
    }

    private String WaitForLaunch()
    {
        String retval = new String("");

        PrintMessage("Waiting for the viewer to launch.");

        while(waitingForLaunch)
        {
            try
            {
                if(viewer.getErrorStream().available() > 0)
                {
                    char[] c = new char[1];
                    c[0] = (char)viewer.getErrorStream().read();
                    retval = retval + String.copyValueOf(c);
                    if(c[0] == '\n')
                        waitingForLaunch = false;
                }
            }
            catch(java.io.IOException e)
            {
                // System.out.println("WaitForLaunch");
            }
            catch(java.lang.NullPointerException e2)
            {
                // System.out.println("WaitForLaunch: waiting...");
                try
                {
                    // The error stream is still a NULL pointer which
                    // seems to indicate that the process object is not
                    // fully available yet. Wait half a second before
                    // trying to read it again.
                    wait(500);
                }
                catch(java.lang.InterruptedException e3)
                {
                    // do nothing
                }
                catch(java.lang.IllegalMonitorStateException e4)
                {
                    // do nothing
                }
            }
        }

        return retval;
    }

    private Process    viewer;
    private String     exeName;
    private String     binPath;
    private Vector     args;
    private boolean    viewerLaunched;
    private boolean    waitingForLaunch;
    private boolean    verbose;
    private Connection conn;
}
