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

#if !defined(_WIN32)
#ifdef AIX
#include <strings.h>
#endif
#include <unistd.h> // alarm
#include <signal.h>
#endif

#include <ConnectionGroup.h>
#include <DebugStream.h>
#include <MDServerApplication.h>
#include <MDServerConnection.h>
#include <LostConnectionException.h>
#include <Utility.h>

// Static member.
MDServerApplication *MDServerApplication::instance = NULL;

// ****************************************************************************
// Method: MDServerApplication::MDServerApplication
//
// Purpose: 
//   Constructor for the MDServerApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:13:34 PST 2000
//
// Modifications:
//     Sean Ahern, Thu Feb 21 15:37:56 PST 2002
//     Added a default timeout of 0.  Without being set through SetTimeout,
//     no timeout will happen.
//   
// ****************************************************************************

MDServerApplication::MDServerApplication() : clients()
{
    keepGoing = true;
    timeout = 0;

#if !defined(_WIN32)
    // Set up an alarm signal handler to exit gracefully.
    signal(SIGALRM, MDServerApplication::AlarmHandler);
#endif
}

// ****************************************************************************
// Method: MDServerApplication::~MDServerApplication
//
// Purpose: 
//   Destructor for the MDServerApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:20:24 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Apr 18 14:58:09 PST 2003
//   I added code to delete the connections.
//
// ****************************************************************************

MDServerApplication::~MDServerApplication()
{
    instance = NULL;

    for(int i = 0; i < clients.size(); ++i)
        delete clients[i];
}

// ****************************************************************************
// Method: MDServerApplication::Instance
//
// Purpose: 
//   Returns a pointer to the only instance of this class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:20:48 PST 2000
//
// Modifications:
//   
// ****************************************************************************

MDServerApplication *
MDServerApplication::Instance()
{
    if(instance == NULL)
        instance = new MDServerApplication;

    return instance;
}

// ****************************************************************************
// Method: MDServerApplication::AlarmHandler
//
// Purpose: 
//   Gracefully exits the MDServer if an SIGALRM signal was received.
//
// Programmer: Sean Ahern
// Creation:   Thu Feb 21 15:36:04 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
MDServerApplication::AlarmHandler(int signal)
{
    debug1 << "MDSERVER exited due to an inactivity timeout of "
           << MDServerApplication::Instance()->timeout << " minutes." << endl;
    exit(0);
}

// ****************************************************************************
// Method: MDServerApplication::AddConnection
//
// Purpose: 
//   Connects the MDServer to a remote process.
//
// Arguments:
//   argc : A pointer to the number of items in argv.
//   argv : A pointer to the argv argument list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:21:15 PST 2000
//
// Modifications:
//    Jeremy Meredith, Wed Dec 31 11:49:51 PST 2003
//    Added code to only trust the first client host seen.  This is to
//    fix issues with -guesshost when running through a NAT firewall.
//    See '4287 and '4288 for more details.
//
// ****************************************************************************

void
MDServerApplication::AddConnection(int *argc, char **argv[])
{
    if (clientHost == "")
    {
        for (int i=0; i<*argc; i++)
        {
            if (!strcmp((*argv)[i], "-host"))
            {
                clientHost = (*argv)[i+1];
                break;
            }
        }
    }
    else
    {
        for (int i=0; i<*argc; i++)
        {
            if (!strcmp((*argv)[i], "-host"))
            {
                (*argv)[i+1] = CXX_strdup(clientHost.c_str());
                break;
            }
        }
    }

    MDServerConnection *newConnection = new MDServerConnection(argc, argv);
    clients.push_back(newConnection);

#ifdef DEBUG
    debug3 << "Added new client. total=" << clients.size() << endl;
#endif
}

// ****************************************************************************
// Method: MDServerApplication::Execute
//
// Purpose: 
//   This is the main event loop of the MDServer program. It executes a loop
//   that checks input on the MDServerConnections until one of them tells the
//   application to quit.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 13:27:19 PST 2000
//
// Modifications:
//    Brad Whitlock, Thu Mar 15 12:34:02 PDT 2001
//    Added code to remove dead clients.
//
//    Brad Whitlock, Wed Apr 25 17:02:26 PST 2001
//    Modified code so the loop is not entered if there are zero clients.
//
//    Jeremy Meredith, Wed Oct 10 14:17:32 PDT 2001
//    Removed the sleep(1) and made it a blocking select.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Sean Ahern, Thu Feb 21 14:33:54 PST 2002
//    Added timeout support.
//
//    Brad Whitlock, Tue Mar 26 11:59:20 PDT 2002
//    I rewrote it so it uses a ConnectionGroup object to determine which
//    connections have input to be processed.
//
// ****************************************************************************

void
MDServerApplication::Execute()
{
    // Only go into the event loop if we have at least 1 client.
    keepGoing = (clients.size() > 0);

    // The application's main loop
    while(keepGoing)
    {
#if !defined(_WIN32)
        // Reset the alarm
        alarm(timeout*60);
#endif
        // Create a connection group that we will use to check if any
        // connections have input to be read.
        ConnectionGroup connGroup;
        for(int i = 0; i < clients.size(); ++i)
            connGroup.AddConnection(clients[i]->GetWriteConnection());

        // Check the connections for input that needs to be processed.
        if(connGroup.CheckInput())
        {
            // Test all of the clients' file descriptors to see if they
            // can be read.
            std::vector<int> deadList;
            for(int i = 0; i < clients.size(); ++i)
            {
                if(connGroup.NeedsRead(i))
                {
#if !defined(_WIN32)
                    // We've got some input.  Turn off the alarm so we can't
                    // time out.
                    alarm(0);
#endif

                    TRY
                    {
                        // Process input.
                        clients[i]->ProcessInput();
                    }
                    CATCH(LostConnectionException)
                    {
                        // Add the current index to the dead list.
                        deadList.push_back(i);
                    }
                    ENDTRY
                }
            }

            // Disconnect dead connections or ones that quit.
            DisconnectDeadConnections(deadList);
        }
    }

    //
    // Go through the list of MDServerConnections and close them down
    // by deleting the MDServerConnection objects.
    //
    for(int i = 0; i < clients.size(); ++i)
        delete clients[i];
    clients.clear();
}

// ****************************************************************************
// Method: MDServerApplication::ExecuteDebug
//
// Purpose: 
//   Reads and prints the file list before exiting.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 18 14:59:32 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Jul 29 12:23:34 PDT 2004
//   Assumes extra smart file grouping.
//
//   Brad Whitlock, Fri Feb 4 15:22:11 PST 2005
//   Changed how file grouping options are passed.
//
// ****************************************************************************

void
MDServerApplication::ExecuteDebug()
{
    // Get the filtered file list.
    GetFileListRPC::FileList files;
    clients[0]->SetFileGroupingOptions("*", true);
    clients[0]->GetFilteredFileList(files);
    // Print the file list to the debug logs.
    debug1 << "FILELIST = " << files << endl;
}

// ****************************************************************************
// Method: EngineApplication::DisconnectDeadConnections
//
// Purpose: 
//   Removes dead connections from the list of clients. This keeps the server
//   responsive if some of its clients fail to disconnect before they die.
//
// Arguments:
//   deadList : The list of dead clients.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 15 12:27:16 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
MDServerApplication::DisconnectDeadConnections(const std::vector<int> &deadList)
{
    int  i;
    bool deletedConnections = (deadList.size() > 0);
    MDServerConnectionVector newClientList;

    // Delete any connections in the dead list.
    for(i = 0; i < deadList.size(); ++i)
    {
        // Delete the connection.
        delete clients[deadList[i]];
        clients[deadList[i]] = 0;
    }

    // Delete any connections whose quit event indicates they wanted
    // to quit.
    for(i = 0; i < clients.size(); ++i)
    {
        if(clients[i] == 0)
            continue;
        
        if(!clients[i]->KeepGoing())
        {
            deletedConnections = true;

            // Delete the connection.
            delete clients[i];
            clients[i] = 0;
        }
    }
    
    // If there were dead connections or connections that quit, we need to
    // get rid of the gaps in the clients vector.
    if(deletedConnections)
    {
        // If the first client is not dead or quit, keep going.
        keepGoing = (clients[0] != 0);

        // Make the new client vector
        for(i = 0; i < clients.size(); ++i)
        {
            if(clients[i] != 0)
                newClientList.push_back(clients[i]);
#ifdef DEBUG
            else
            {
                debug1 << "MDSERVER: Disconnected client " << i << endl;
            }
#endif
        }

        // Copy the modified list into the real list.
        clients = newClientList;
    }
}
