/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <cerrno>
#include <cstdlib>
#include <cstring>
#if defined(_WIN32)
#include <win32commhelpers.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <pwd.h>
#include <unistd.h>
#endif

#include <visit-config.h>

#include <ParentProcess.h>
#include <DebugStream.h>
#include <SocketConnection.h>
#include <CommunicationHeader.h>
#include <CouldNotConnectException.h>
#include <IncompatibleVersionException.h>

// ****************************************************************************
// Method: ParentProcess::ParentProcess
//
// Purpose: 
//   Constructor for the ParentProcess class. This method reads
//   through the list of arguments given to it and removes any
//   having to do with the setup of sockets. It then uses the
//   information that it got to set up the sockets.
//
// Arguments:
//
// Returns:    
//
// Note:       
//    Note that in this class, -rport corresponds to write instead
//    of read and vice-versa. This is because we're now on tbe other
//    side of the socket and have to reverse.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 15:08:44 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//
//    Brad Whitlock, Thu Oct 5 18:57:05 PST 2000
//    Added code to initialize the arrays of SocketConnection pointers.
//
//    Brad Whitlock, Thu Apr 26 15:58:35 PST 2001
//    Added initialization of the version.
//
//    Brad Whitlock, Wed Jan 11 17:01:24 PST 2006
//    Added localUserName.
//
//    Jeremy Meredith, Wed Apr 30 13:14:53 EDT 2008
//    Added apparentHostName.
//
// ****************************************************************************

ParentProcess::ParentProcess() : version(VERSION), localUserName()
{
    // Set some default values.
    hostName = std::string("localhost");
    apparentHostName = "";

    // Zero out the SocketConnection pointers.
    readConnections = 0;
    writeConnections = 0;
    nReadConnections = 0;
    nWriteConnections = 0;
}

// ****************************************************************************
// Method: ParentProcess::~ParentProcess
//
// Purpose: 
//   Destructor for the ParentProcess class. It closes any open file
//   descriptors.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 15:15:02 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//
//    Brad Whitlock, Thu Oct 5 18:56:43 PST 2000
//    Reimplemented so it uses SocketConnections.
//
//    Brad Whitlock, Tue Mar 19 16:05:17 PST 2002
//    Made it work on Windows.
//
//    Brad Whitlock, Fri May 12 11:49:43 PDT 2006
//    We now copy the hostent on Windows, free it here.
//
// ****************************************************************************

ParentProcess::~ParentProcess()
{
    // Delete the read connections
    if(nReadConnections > 0)
    {
        for(int i = 0; i < nReadConnections; ++i)
        {
            // Delete the connection
            delete readConnections[i];
        }
        delete [] readConnections;
        readConnections = 0;
        nReadConnections = 0;
    }

    if(readConnections != 0)
    {
        delete [] readConnections;
        readConnections = 0;
        nReadConnections = 0;
    }

    // Delete the write connections
    if(nWriteConnections > 0)
    {
        for(int i = 0; i < nWriteConnections; ++i)
        {
            // Delete the connection
            delete writeConnections[i];
        }
        delete [] writeConnections;
        writeConnections = 0;
        nWriteConnections = 0;
    }

    if(writeConnections != 0)
    {
        delete [] writeConnections;
        writeConnections = 0;
        nWriteConnections = 0;
    }

#if defined(_WIN32)
    // On Windows, we make a copy. Free it now.
    if(hostInfo != NULL)
        FreeHostent((struct hostent *)hostInfo);
#endif
}

// ****************************************************************************
// Method: ParentProcess::SetVersion
//
// Purpose: 
//   Sets the version string reported back to RemoteProcess.
//
// Arguments:
//   ver : The version string reported back to RemoteProcess.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 26 15:59:26 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ParentProcess::SetVersion(const std::string &ver)
{
    version = ver;
}

// ****************************************************************************
// Method: ParentProcess::Connect
//
// Purpose: 
//   Reads the command line arguments and removes -rhost, -rport,
//  -wport. Once these arguments have been read, it uses the
//   information that was passed to create sockets to talk to the
//   parent process.
//
// Arguments:
//    argc : A pointer to the number of arguments in the list.
//    argv : A pointer to the list of arguments.
//    createSockets : Set this to true if you want the routine to
//                    create sockets as it digests the command line
//                    arguments. This was added to support parallel.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 16:44:46 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.   
//
//    Brad Whitlock, Wed Nov 1 14:28:27 PST 2000
//    I made it ignore more than one well-formed occurrance of
//    -nwrite, -nread. This fixes a problem that causes the program
//    to hang or crash when more than one occurrance of those arguments
//    is encountered.
//
//    Jeremy Meredith, Fri Apr 27 15:28:42 PDT 2001
//    Added fail bit.
//
//    Brad Whitlock, Tue Oct 1 17:19:05 PST 2002
//    I made it throw CouldNotConnectException if it is supposed to be
//    creating sockets and it can't.
//
//    Brad Whitlock, Mon Dec 16 15:01:01 PST 2002
//    I added code to get the security key from the command line.
//
//    Brad Whitlock, Fri Apr 18 15:13:34 PST 2003
//    I added the -noconnect flag to help debug certain components.
//
//    Brad Whitlock, Tue Jul 29 10:56:34 PDT 2003
//    I added numRead and numWrite so we don't have to get the number of
//    connections to create from the command line anymore.
//
//    Brad Whitlock, Tue Jan 17 14:26:09 PST 2006
//    Added debug logging.
//
//    Kathleen Bonnell, Wed Aug  8 14:48:58 PDT 2007 
//    Search for '-rawmap' which can replace '-host' with some implementations
//    of openmpi (mpirun translates the -host arg passed to visit, and sends
//    out '-rawmap 1 <hostname>').
//
//    Kathleen Bonnell, Thu Aug  9 10:12:18 PDT 2007 
//    Moved '-rawmap' test to just after '-host' test. 
//
//    Brad Whitlock, Tue Apr 14 15:13:07 PDT 2009
//    I made it return true if any connections were created.
//
// ****************************************************************************

bool
ParentProcess::Connect(int numRead, int numWrite, int *argc, char **argv[],
    bool createSockets, int failCode)
{
    const char *mName = "ParentProcess::Connect: ";
    char **argv2 = *argv;
    bool rhostSpecified = false;
    bool nWriteSpecified = false, nReadSpecified = false;
    int  i, deleteCount = 0, port = 0;

    // Log the arguments.
    debug5 << mName << "Called with (numRead=" << numRead 
           << ", numWrite=" << numWrite
           << ", argc=" << *argc
           << ", argv={";
    for (i = 0; i < *argc ; ++i)
    {
        debug5 << argv2[i];
        if(i < *argc-1)
           debug5 << ", ";
    }
    debug5 << "})" << endl;

    //
    // Go through the arguments and override the defaults.
    //
    for (i = 1; i < *argc ; ++i)
    {
        deleteCount = 0;
        if (std::string(argv2[i]) == std::string("-host"))
        {
            if(i + 1 < *argc)
            {
                rhostSpecified = true;
                hostName = std::string(argv2[i + 1]);
                debug5 << mName << "hostName = " << hostName.c_str() << endl;
                GetHostInfo();
                deleteCount = 2;
            }
        }
        else if (std::string(argv2[i]) == std::string("-rawmap"))
        {
            if(i + 1 < *argc)
            {
                rhostSpecified = true;
                hostName = std::string(argv2[i + 2]);
                debug5 << mName << "hostName = " << hostName.c_str() << endl;
                GetHostInfo();
                deleteCount = 3;
            }
        }
        else if(std::string(argv2[i]) == std::string("-port"))
        {
            if(rhostSpecified && (i + 1 < *argc))
            {
                port = atoi(argv2[i + 1]);
                debug5 << mName << "port = " << port << endl;
                deleteCount = 2;
            }
        }
        else if(std::string(argv2[i]) == std::string("-key"))
        {
            if(i + 1 < *argc)
            {
                securityKey = std::string(argv2[i + 1]);
                debug5 << mName << "securityKey = " << securityKey.c_str() << endl;
                deleteCount = 2;
            }
        }
        else if(std::string(argv2[i]) == std::string("-noconnect"))
        {
            createSockets = false;
            deleteCount = 1;
        }

        //
        // Delete some arguments.
        //
        if(deleteCount > 0)
        {
            for(int j = i; j < *argc; ++j)
            {
                argv2[j] = (j + deleteCount < *argc) ?
                    argv2[j + deleteCount] : NULL;
            }
            (*argc) -= deleteCount;
            --i;
        }
    } // end for i

    //
    // Now that we have connection information, create the connections.
    //
    bool createdConnections = false;
    if(rhostSpecified && createSockets)
    {
        debug5 << mName << "Creating sockets" << endl;
        if(numRead > 0)
        {
            debug5 << mName << "Creating read sockets" << endl;
            nReadSpecified = true;
            writeConnections = new Connection*[numRead];
            for(int j = 0; j < numRead; ++j)
            {
                int desc = GetClientSocketDescriptor(port);
                if(desc != -1)
                {
                    writeConnections[nWriteConnections] = new SocketConnection(desc);
                    ++nWriteConnections;
                }
            }
        }

        if(numWrite > 0)
        {
            debug5 << mName << "Creating write sockets" << endl;
            nWriteSpecified = true;
            readConnections = new Connection*[numWrite];
            for(int j = 0; j < numWrite; ++j)
            {
                int desc = GetClientSocketDescriptor(port);
                if(desc != -1)
                {
                    readConnections[nReadConnections] = new SocketConnection(desc);
                    ++nReadConnections;
                }
            }
        }

        if(nReadConnections == 0 && nWriteConnections == 0)
        {
            //
            // If we are supposed to create sockets but cannot, then we
            // throw a CouldNotConnectException so we know that we could not
            // connect back to the parent process.
            //
            EXCEPTION0(CouldNotConnectException);
        }
        else
        {
            //
            // Now that the sockets are open, exchange type representation info
            // and set that info in the socket connections.
            //
            debug5 << mName << "Exchanging type representations." << endl;
            ExchangeTypeRepresentations(failCode);
            createdConnections = true;
        }
    }

    debug5 << mName << "done" << endl;
    return createdConnections;
}

// ****************************************************************************
// Method: ParentProcess::ExchangeTypeRepresentations
//
// Purpose: 
//   Exchanges the machine's type representation with that of the
//   machine on the other end of the sockets. It then puts that
//   type representation into the write sockets. Conversion is disabled
//   if the type representations are the same.
//
// Notes:
//   This differs from RemoteProcess's code in that the read/write are
//   reversed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:25:19 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Apr 25 12:04:09 PDT 2001
//   Moved code that reads/writes the communication header into the
//   CommunicationHeader class.
//
//   Jeremy Meredith, Fri Apr 27 15:28:42 PDT 2001
//   Added fail bit.
//
//   Brad Whitlock, Mon Mar 25 14:13:34 PST 2002
//   Made it pass a connection to the communication header object.
//
//   Brad Whitlock, Mon Dec 16 15:28:40 PST 2002
//   I passed security keys to the WriteHeader method.
//
// ****************************************************************************

void
ParentProcess::ExchangeTypeRepresentations(int failCode)
{
    // We can only exchange type representations if we opened
    // a read and a write socket.
    if(nReadConnections > 0 && nWriteConnections > 0)
    {
        TypeRepresentation  local;
        CommunicationHeader header;
        bool                throwVersionException = false;

        TRY
        {
            // Read the remote component and platform information.
            header.ReadHeader(readConnections[0], version, false);
        }
        CATCH(IncompatibleVersionException)
        {
            throwVersionException = true;
        }
        ENDTRY

        // Send the local component and platform information.
        header.WriteHeader(writeConnections[0], version, securityKey,
                           header.GetSocketKey(), failCode);

        // Now that we have the type representation for the remote machine,
        // if it is the same as the local type representation, turn off 
        // conversion in the write connections. Otherwise, set it into all
        // of the write sockets.
        if(local == header.GetTypeRepresentation())
        {
            for(int i = 0; i < nReadConnections; ++i)
                readConnections[i]->EnableConversion(false);
        }
        else
        {
            for(int i = 0; i < nReadConnections; ++i)
            {
                readConnections[i]->SetDestinationFormat(
                    header.GetTypeRepresentation());
            }
        }

        // If an IncompatibleVersionException was thrown, we caught it so
        // we could send this side's communication header. Now that we've done
        // that, we can re-throw the exception.
        if(throwVersionException)
        {
            EXCEPTION0(IncompatibleVersionException);
        }
    }
}

// ****************************************************************************
// Method: ParentProcess::GetHostName
//
// Purpose: 
//   Returns the host name on which the parent process is running.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 15:26:34 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const std::string &
ParentProcess::GetHostName() const
{
    return hostName;
}

// ****************************************************************************
//  Method:  ParentProcess::GetApparentHostName
//
//  Purpose:
//    Returns the externally visible hostname for the host on which
//    the parent process is running.  (I.e. it will try not to return
//    "localhost" if possible, though it will do that instead of
//    returning an error.)
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 30, 2008
//
// ****************************************************************************

const std::string &
ParentProcess::GetApparentHostName()
{
    if (apparentHostName == "")
    {
        // Determine as best we can the actual host name
        apparentHostName = hostName;
        if (hostName == "localhost" ||
            hostName == "127.0.0.1")
        {
            // We're using the loopback device; try harder
            char localHostStr[256];
            if (gethostname(localHostStr, 256) != -1)
            {
                struct hostent *localHostEnt = gethostbyname(localHostStr);
                if (localHostEnt)
                {
                    apparentHostName = std::string(localHostEnt->h_name);
                }
                else
                {
                    apparentHostName = std::string(localHostStr);
                }
            }
        }
    }

    return apparentHostName;
}

// ****************************************************************************
// Method: ParentProcess::GetUserName
//
// Purpose: 
//   Returns the user name.
//
// Returns:    The user name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 11 15:43:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

std::string
ParentProcess::GetTheUserName() const
{
    std::string retval;

    //
    // Get the local user name.
    //
#if defined(_WIN32)
    char username[100];
    DWORD maxLen = 100;
    GetUserName((LPTSTR)username, (LPDWORD)&maxLen);
    retval = std::string(username);
#else
    struct passwd *users_passwd_entry = NULL;
    if((users_passwd_entry = getpwuid(getuid())) != NULL)
        retval = std::string(users_passwd_entry->pw_name);
#endif

    return retval;
}

// ****************************************************************************
// Method: ParentProcess::GetReadConnection
//
// Purpose: 
//   Gets a pointer to the i'th read connection.
//
// Arguments:
//   i : The index of the connection we want.
//
// Returns:    A pointer to the i'th read connection, or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:36:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
ParentProcess::GetReadConnection(int i) const
{
    return (i >= 0 && i < nReadConnections) ? readConnections[i] : 0;
}

// ****************************************************************************
// Method: ParentProcess::GetWriteConnection
//
// Purpose: 
//   Gets a pointer to the i'th write connection.
//
// Arguments:
//   i : The index of the connection we want.
//
// Returns:    A pointer to the i'th write connection, or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:36:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
ParentProcess::GetWriteConnection(int i) const
{
    return (i >= 0 && i < nWriteConnections) ? writeConnections[i] : 0;
}

// ****************************************************************************
// Method: ParentProcess::GetClientSocketDescriptor
//
// Purpose: 
//   Connects to a socket on the parent process. It uses the hostname
//   that was specified with the -rhost argument and the port number
//   is passed in.
//
// Arguments:
//
// Returns:    
//   A socket descriptor if successful, -1 if not successful.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 15:11:55 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 19 16:09:30 PST 2002
//   Made it work on Windows.
//
//   Brad Whitlock, Tue Jan 17 14:37:35 PST 2006
//   Added debug logging.
//
//   Brad Whitlock, Fri May 12 11:50:15 PDT 2006
//   Added more extensive debug logging for the Windows platform.
//
//   Jeremy Meredith, Thu May 24 11:22:55 EDT 2007
//   Added errno to unix output to determine the actual error.
//
//   Hank Childs, Wed Dec 19 08:51:45 PST 2007
//   Added print statement.
//
//   Kathleen Bonnell, Thu Mar 20 08:02:44 PDT 2008 
//   Added possbile IP resolution failure to the error message for non-windows.
//
//   Jeremy Meredith, Tue Jun 24 11:09:25 EDT 2008
//   Added error text to connect failure message via strerror.
//
//   Kathleen Bonnell, Wed Sep 10 11:16:00 PDT 2008 
//   Restructured to allow looping over all ips in h_addr_list until a 
//   connection is completed, or there are no more ips.
// 
// ****************************************************************************

int
ParentProcess::GetClientSocketDescriptor(int port)
{
    const char *mName = "ParentProcess::GetClientSocketDescriptor: ";
    int                s;
    bool               connected = false;
    struct hostent     *hp;
    struct sockaddr_in server;

    //
    // Set up the structures for opening the sockets.
    //
    debug5 << mName << "Set up using port " << port << endl;
    hp = (struct hostent *)hostInfo;
    if (hp == NULL)
        return -1;

    // 
    // Create a socket.
    // 
    debug5 << mName << "Creating a socket" << endl;
#ifdef _WIN32
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        LogWindowsSocketError(mName, "socket");
        return -1;
    }

    // Disable the Nagle algorithm 
    debug5 << mName << "Setting socket options" << endl;
    int opt = 1;
    if(setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&opt, 
                  sizeof(int)) == SOCKET_ERROR)
    {
        LogWindowsSocketError(mName, "setsockopt");
    }
#else
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        return -1;
    }

    // Disable the Nagle algorithm 
    debug5 << mName << "Setting socket options" << endl;
    int opt = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    //
    // Try connection
    //
    memset(&server, 0, sizeof(server));

    for (int i = 0; hp->h_addr_list[i] != NULL && !connected; ++i)
    {
        memcpy(&(server.sin_addr), hp->h_addr_list[i], hp->h_length);
        server.sin_family = hp->h_addrtype;
        server.sin_port = htons(port);
    

        debug5 << mName << "Calling connect" << endl;
        debug5 << "(If you see no messages after this one, VisIt was not\n"
               << "able to connect to the client machine.  Nine times out\n"
               << "of ten, this is a firewall issue on the client machine.\n"
               << "It could also mean that VisIt was unable to resolve the\n"
               << "IP address for the client machine.";
#ifndef _WIN32
        debug5 << "  You may need to verify the contents of /etc/hosts.";
#endif
        debug5 << ")" << endl;

        connected =(connect(s, (struct sockaddr *)&server, sizeof(server)) ==0);
        if (!connected)
        {
#ifdef _WIN32
            LogWindowsSocketError(mName, "connect");
            debug5 << mName << "Could not connect!" << endl;
            closesocket(s);
#else
            debug5 << mName << "Could not connect! "
                   << "(error="<<errno<<": "<<strerror(errno)<<")" << endl;
            close(s);
#endif
            continue;
        }
    }

    if (!connected)
        return -1;

    debug5 << mName << "Connected socket" << endl;

    return s;
}

// ****************************************************************************
// Method: ParentProcess::GetHostInfo
//
// Purpose: 
//   Gets the host information using the hostName string. This is
//   broken out from GetClientSocketDescriptor so the host information
//   only has to be queried one time.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 15:24:10 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri May 12 11:50:15 PDT 2006
//   Added more extensive debug logging for the Windows platform.
//   
// ****************************************************************************

void
ParentProcess::GetHostInfo()
{
    const char *mName = "ParentProcess::GetHostInfo: ";
    debug5 << mName << "Calling gethostbyname(\""
           << hostName.c_str() << "\")\n";

#if defined(_WIN32)
    // Create a copy of the hostent struct in case other WinSock
    // functions want to modify it.
    hostInfo = (void *)CopyHostent(gethostbyname(hostName.c_str()));
    if(hostInfo == NULL)
        LogWindowsSocketError(mName, "gethostbyname");
#else
    hostInfo = (void *)gethostbyname(hostName.c_str());
#endif
}

// ****************************************************************************
// Method: ParentProcess::GetLocalUserName
//
// Purpose: 
//   Gets the local user's name.
//
// Returns:    A reference to the local user name.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 11 17:02:38 PST 2006
//
// Modifications:
//  Brad Whitlock, Tue Jan 17 14:38:35 PST 2006
//  Added debug logging.
//
// ****************************************************************************

const std::string &
ParentProcess::GetLocalUserName()
{
    //
    // Get the local user name.
    //
    if(localUserName.size() == 0)
    {
        debug5 << "Getting local user name: ";
#if defined(_WIN32)
        char username[100];
        DWORD maxLen = 100;
        GetUserName((LPTSTR)username, (LPDWORD)&maxLen);
        localUserName = std::string(username);
#else
        struct passwd *users_passwd_entry = NULL;
        if((users_passwd_entry = getpwuid(getuid())) != NULL)
            localUserName = std::string(users_passwd_entry->pw_name);
#endif
        debug5 << localUserName.c_str() << endl;
    }

    return localUserName;
}
