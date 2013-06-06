//
// g++ -I/usr/gapps/visit/2.1.2/linux-x86_64/include/visit/include -o simfe simfe.cpp
//

#ifdef _WIN32
#if _MSC_VER < 1600
#define _WIN32_WINNT 0x0502
#endif
#include <winsock2.h>
#include <direct.h>
#include <sys/stat.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <visit-config.h>

//******************************************************************************
// CODE BORROWED FROM LIBSIM CONTROL INTERFACE
//******************************************************************************

#ifdef _WIN32
#define VISIT_SOCKET         SOCKET
#define VISIT_INVALID_SOCKET INVALID_SOCKET
#else
#define VISIT_SOCKET         int
#define VISIT_INVALID_SOCKET -1
#endif

VISIT_SOCKET AcceptConnection(VISIT_SOCKET &listenSocket, 
    struct sockaddr_in &listenSockAddr)
{
    VISIT_SOCKET desc = VISIT_INVALID_SOCKET;
    int opt = 1;

    /* Wait for the socket to become available on the other side. */
    do
    {
#ifdef HAVE_SOCKLEN_T
        socklen_t len;
#else
#ifdef __APPLE__
        unsigned int len;
#else
        int len;
#endif
#endif
        len = sizeof(struct sockaddr);
        desc = accept(listenSocket, (struct sockaddr *)&listenSockAddr, &len);
    }
    while (desc == VISIT_INVALID_SOCKET
#ifndef _WIN32
           && errno != EWOULDBLOCK
#endif
           );

    /* Disable Nagle algorithm. */
#if defined(_WIN32)
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY,
               (const char FAR*)&opt, sizeof(int));
#else
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif
    
    return desc;
}

bool
GetLocalhostName(std::string &lhost)
{
    char localhostStr[256], localhost[256];
    struct hostent *localhostEnt = NULL;

    if (gethostname(localhostStr, 256) == -1)
    {
        /* Couldn't get the hostname, it's probably invalid */
        return false;
    }

    localhostEnt = gethostbyname(localhostStr);
    if (localhostEnt == NULL)
    {
        /* Couldn't get the full host entry; it's probably invalid */
        
        strcpy(localhostStr, "localhost");
        localhostEnt = gethostbyname(localhostStr);
        if(localhostEnt != NULL)
        {
            sprintf(localhost, "%s", localhostEnt->h_name);
        }
        else
        {
            strcpy(localhost, "localhost");
        }
    }
    else
    {
        sprintf(localhost, "%s", localhostEnt->h_name);
    }

    lhost = localhost;
    return true;
}

bool
StartListening(int &listenSocket, int &listenPort,
    struct sockaddr_in &listenSockAddr)
{
    bool portFound = false;
    int on = 1;
    int err;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
    {
        /* Cannot open a socket. */
        fprintf(stderr, "StartListening: socket() failed.\n");
        return false;
    }

    /*
     * Look for a port that can be used.
     */
    listenSockAddr.sin_family = AF_INET;
    listenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenPort = 5609;
    while (!portFound && listenPort < 32767)
    {
        listenSockAddr.sin_port = htons(listenPort);
#if !defined(_WIN32)
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

        err = bind(listenSocket, (struct sockaddr *)&listenSockAddr,
                   sizeof(listenSockAddr));
        if (err)
        {
            listenPort++;
        }
        else
        {
            portFound = true;
        }
    }
    if (!portFound)
    {
        /* Cannot find unused port. */
       listenSocket = VISIT_INVALID_SOCKET;
       fprintf(stderr, "StartListening: port not found\n");
       return false;
    }

    err = listen(listenSocket, 5);
    if (err)
    {
       listenSocket = VISIT_INVALID_SOCKET;
       fprintf(stderr, "StartListening: listen() failed\n");
       return false;
    }

    return true;
}

void ReceiveSingleLineFromSocket(char *buffer, size_t maxlen, VISIT_SOCKET desc)
{
    char *buf = buffer;
    char *ptr = buffer;
    char *tmp = NULL;
    int n;

    strcpy(buffer, "");
    tmp = strstr(buf, "\n");
    while (!tmp)
    {
        n = recv(desc, (void*)ptr, maxlen, 0);
        ptr += n;
        *ptr = '\0';
        tmp = strstr(buf, "\n");
    }
    *tmp = '\0';
}

char *ReceiveContinuousLineFromSocket(char *buffer, size_t maxlen, VISIT_SOCKET desc)
{
    char *buf = buffer;
    char *ptr = buffer;
    char *tmp = NULL;
    int n;

    tmp = strstr(buf, "\n");
    while (!tmp)
    {
        n = recv(desc, (void*)ptr, maxlen, 0);
        ptr += n;
        *ptr = '\0';
        tmp = strstr(buf, "\n");
    }
    *tmp = '\0';

    return tmp+1;
}

bool SendStringOverSocket(char *buffer, VISIT_SOCKET desc)
{
    size_t      nleft, nwritten;
    const char *sptr;

    /* Send it! */
    sptr = (const char*)buffer;
    nleft = strlen(buffer);
    while (nleft >= 1)
    {
        if ((nwritten = send(desc, (const char *)sptr, nleft, 0)) == 0)
        {
            return false;
        }
        nleft -= nwritten;
        sptr  += nwritten;
    }

    return true;
}

void VerifySecurityKeys(VISIT_SOCKET desc)
{
    int offeredKeyLen;
    char offeredKey[2000] = "";

    /* The first thing the VCL sends is the key */
    ReceiveSingleLineFromSocket(offeredKey, 2000, desc);

    /* Sure, tell VCL that they match. */
    SendStringOverSocket("success\n", desc);
}

void GetConnectionParameters(VISIT_SOCKET desc, 
    std::vector<std::string> &engine_argv)
{
    char buf[2000] = "";
    char *tmpbuf;
    char *nxtbuf;
    int i;

    /* Receive the ARGV over the socket */
    tmpbuf = buf;
    while (1)
    {
        nxtbuf = ReceiveContinuousLineFromSocket(tmpbuf, 2000, desc);

        if (strlen(tmpbuf) == 0)
            break;

        engine_argv.push_back(std::string(tmpbuf));
        tmpbuf = nxtbuf;
    }
}

//******************************************************************************
// CODE BORROWED FROM VCL
//******************************************************************************

#if 0
//
// Mimic what VCL sends to the simulation.
//
int
ConnectSimulation(const std::string &simHost, int simPort, 
    const std::string &simSecurityKey, const std::vector<std::string> &launchArgs)
{
    int                s;
    struct hostent     *hp;
    struct sockaddr_in server;
    size_t             i;

    // This is based on code from VCL.

    //
    // Get the simulation host information
    //
    void *hostInfo = (void *)gethostbyname(simHost.c_str());

    //
    // Set up the structures for opening the sockets.
    //
    hp = (struct hostent *)hostInfo;
    if (hp == NULL)
    {
        std::cerr << "Error: Could not get host info for: " << simHost << std::endl;
        return -1;
    }

    memset(&server, 0, sizeof(server));
    memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons(simPort);
    
    // 
    // Create a socket.
    // 
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        std::cerr << "Error: Could not create socket" << std::endl;
        return -2;
    }

    // Disable the Nagle algorithm 
    int opt = 1;
#if defined(_WIN32)
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&opt, sizeof(int));
#else
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    int success = connect(s, (struct sockaddr *)&server, sizeof(server));
    if (success < 0)
    {
#if defined(_WIN32)
        closesocket(s);
#else
        close(s);
#endif
        std::cerr << "Error: Could not connect" << std::endl;
        return -3;
    }

    //
    // Send the security key and launch information to the simulation
    //
    char tmp[2000];
    size_t          nleft, nwritten;
    const char      *ptr;

    sprintf(tmp, "%s\n", simSecurityKey.c_str());

    ptr = (const char*)tmp;
    nleft = strlen(tmp);
    while (nleft > 0)
    {
        if((nwritten = send(s, (const char *)ptr, nleft, 0)) <= 0)
        {
            std::cerr << "Error: Could not send security key: " << simSecurityKey << std::endl;
            return -4;
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }

    //
    // Receive a reply
    //
    strcpy(tmp,"");
    char *tbuf = tmp;
    char *tptr = tmp;
    int n;
    char *newline = strstr(tbuf, "\n");
    while (!newline)
    {
        n = recv(s, tptr, 2000, 0);
        tptr += n;
        *tptr = 0;
        newline = strstr(tbuf, "\n");
    }
    *newline = 0;
    if (strcmp(tmp, "success") != 0)
    {
        std::cerr << "Error: The key match from the sim did not return "
                     "\"success\". It returned " << tmp << std::endl;
        return -5;
    }

    // Create the Launch args
    strcpy(tmp, "");
    for (size_t i=0; i<launchArgs.size(); i++)
    {
        strcat(tmp, launchArgs[i].c_str());
        strcat(tmp, "\n");
    }
    strcat(tmp, "\n");

    // Send it!
    ptr = (const char*)tmp;
    nleft = strlen(tmp);
    while (nleft > 0)
    {
        if((nwritten = send(s, (const char *)ptr, nleft, 0)) <= 0)
        {
            std::cerr << "Error: Could not send launch arguments." << std::endl;
            return -6;
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }

    return 0;
}
#endif

//******************************************************************************
// NEW CODE
//******************************************************************************

void
ReadSim2File(const std::string &filename, 
    std::string &host, int &port, std::string &key, 
    std::string &path, std::string &comment)
{
    host = key = path = comment = "";
    port = -1;

    char buff[256];
    std::ifstream in(filename.c_str());
    while (in >> buff)
    {
        if (strcmp(buff, "host")==0)
        {
            in >> host;
        }
        else if (strcmp(buff, "port")==0)
        {
            in >> port;
        }
        else if (strcmp(buff, "key")==0)
        {
            in >> key;
        }
        else if (strcmp(buff, "path")==0)
        {
            in >> path;
        }
        else if (strcmp(buff, "comment")==0)
        {
            in.get(); // assume one delimiter character

            char val[2048];
            in.getline(val, 2048);
            comment = std::string(val);
        }
        else
        {
            // Skip
            char val[2048];
            in.getline(val, 2048);
        }
    }

    in.close();
}

void
WriteSim2File(const std::string &filename,
    const std::string &host, int port, const std::string &key, 
    const std::string &path, const std::string &comment)
{
    std::ofstream out(filename.c_str());
    out << "host " << host << std::endl;
    out << "port " << port << std::endl;
    out << "key " << key << std::endl;
    out << "path " << path << std::endl;
    out << "comment " << comment << std::endl;
    out.close();
}

int
main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: simfe file.sim2\n");
        return -1;
    }

    // Read the other sim's sim2 file so we know where to connect.
    std::string simhost, simkey, simpath, simcomment;
    int simport = -1;
    ReadSim2File(argv[1], simhost, simport, simkey, simpath, simcomment);

    // Get localhost
    std::string localhost;
    if (!GetLocalhostName(localhost))
    {
        return -2;
    }
    std::cout << "Localhost=" << localhost << std::endl;

    // Create the listen socket.
    VISIT_SOCKET listenSocket = VISIT_INVALID_SOCKET;
    int listenPort = -1;
    struct sockaddr_in listenSockAddr;
    if (!StartListening(listenSocket, listenPort, listenSockAddr))
    {
        return -3;
    }
    std::cout << "Created a listen socket" << std::endl;

    // Write a new sim file based on the old one. We write different
    // host and port but otherwise copy the old sim file. This is good
    // so the same key will be used.
    std::string simFile(argv[1]);
    std::string simFileNoExt(simFile.substr(0, simFile.size()-5));
    std::string newFileName(simFileNoExt + "_FE.sim2");
    WriteSim2File(newFileName,
        localhost, listenPort, simkey, simpath, simcomment);
    std::cout << "Wrote new sim2 file: " << newFileName << std::endl;

    while(true)
    {
        // Block until VisIt connects.
        fd_set readSet;
        FD_SET(listenSocket, &readSet);
        int status = select(listenSocket+1,
                            &readSet, (fd_set*)NULL, (fd_set*)NULL, NULL);

        if(status > 0 && FD_ISSET(listenSocket, &readSet))
        {
            std::cout << "Received simulation connection request" << std::endl;

            // VisIt has requested a connection to this program. Let's
            // simulate VisItAttemptToCompleteConnection except that we'll
            // try to connect outbound to the real sim so we can send it
            // connection information that was sent here first. Once that
            // happens, the real sim should skip connecting to this program
            // and connect directly to VCL.

            VISIT_SOCKET socket = AcceptConnection(listenSocket, listenSockAddr);
            if (socket >= 0)
            {
                std::cout << "Accepted connection to VCL" << std::endl;
                // Talk to VCL.
                VerifySecurityKeys(socket);
                std::vector<std::string> engine_argv;
                GetConnectionParameters(socket, engine_argv);

                std::cout << "Arguments: " << std::endl;
                for(int i = 0; i < engine_argv.size(); ++i)
                    std::cout << "    argv[" << i << "] = " << engine_argv[i] << std::endl;
#if 1
                // Write the arguments to a text file that the sim is waiting for.
                std::string connectFile(simFile + ".connect");
                std::ofstream out(connectFile.c_str());
                for(int i = 0; i < engine_argv.size(); ++i)
                    out << engine_argv[i] << std::endl;
                out << "END" << std::endl;
                out.close();
#else
                // Now that we've communicated to VCL, relay the information
                // to the real simulation so it can start connecting back to VCL.
                std::cout << "Connecting to real simulation at: host=" << simhost
                          << ", simport=" << simport << ", simkey=" << simkey << std::endl;

                if(ConnectSimulation(simhost, simport, simkey, engine_argv) < 0)
                {
                    std::cout << "There was an error connecting to the sim" << std::endl;
                }
#endif
            }            
        }
    }    

    return 0;
}
