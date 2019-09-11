// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SOCKET_BRIDGE_H
#define SOCKET_BRIDGE_H
#include <comm_exports.h>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#endif
#include <stdio.h>

// ****************************************************************************
// Class: SocketBridge
//
// Purpose:
//   This class bridges two local ports.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   May 23, 2007
//
// Modifications:
//    Thomas R. Treadway, Mon Jun  4 09:54:17 PDT 2007
//    Added types.h for fd_set definition.
//
//    Cyrus Harrison, Thu Jun  7 11:44:07 PDT 2007
//    Added <sys/select.h> for AIX fd_set definition 
//
//    Gunther H. Weber, Thu Jan 14 11:38:27 PST 2010
//    Added ability to connect bridge to other host than localhost.
//
//    Brad Whitlock, Thu Oct 16 10:23:37 PDT 2014
//    Debugging improvements, set the buffer size.
//
// ****************************************************************************

class COMM_API SocketBridge
{
  public:
          SocketBridge(int from, int to, const char* toHost="localhost");
         ~SocketBridge();

    void  SetLogging(bool);
    void  SetBufferSize(int);
    void  Bridge();

  protected:
    int   NumActiveBridges();
    bool  GetListenActivity();
    int   GetOriginatingActivity();
    int   GetTerminatingActivity();
    void  WaitForActivity();
    void  StartNewBridge();
    void  CloseBridge(int index);
    void  ForwardOrigToTerm(int index);
    void  ForwardTermToOrig(int index);

  private:
    int                from_port;
    int                to_port;
    const char        *to_host;
    struct sockaddr_in listen_sock;
    int                listen_fd;
    int                originating_fd[1000];
    int                terminating_fd[1000];
    int                num_bridges;
    fd_set             activity;
    char               *buff;
    int                 buffSize;
    FILE               *log;
    bool                logging;
};

#endif
