/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <win32commhelpers.h>
#include <DebugStream.h>

#if defined(_WIN32)
static void
CopyHostentString(char **dest, char *src, const char *name, const char *mName)
{
    if(src == NULL)
    {
        *dest = NULL;
        debug5 << mName << name << " = NULL" << endl;
    }
    else
    {
        int len = strlen(src)+1;
        *dest = (char*)malloc(len);
        memset(*dest, 0, len);
        strcpy(*dest, src);
        debug5 << mName << name << " = " << src << endl;
    }
}

static void
CopyHostentStringList(char ***destp, char **src, const char *name, const char *mName)
{
    if(src == NULL)
    {
       debug5 << mName << name << " = NULL" << endl;
       *destp = NULL;
       return;
    }

    // Count the number of items in the list.
    int numEntries = 0;
    for(char **ptr = src; *ptr != NULL; ++ptr)
        ++numEntries;

    char **dest = (char **)malloc(numEntries + 1);
    debug5 << mName << name << " = {" << endl;
    for(int i = 0; src[i] != NULL; ++i)
        CopyHostentString(&dest[i], src[i], "", mName);
    dest[numEntries] = NULL;
    debug5 << mName << "NULL" << endl;
    debug5 << mName << "}" << endl;

    *destp = dest;
}

// ****************************************************************************
// Method: CopyHostent
//
// Purpose: 
//   Copies a hostent structure and returns a pointer to the new one.
//
// Arguments:
//   h : The hostent structure to copy.
//
// Returns:    A pointer to a new hostent structure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 12 12:15:26 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

struct hostent *
CopyHostent(struct hostent *h)
{
    const char *mName = "CopyHostent: ";

    if(h == NULL)
    {
        debug5 << mName << "Returning NULL. This means gethostbyname failed" << endl;
        return NULL;
    }

    struct hostent *h2 = (struct hostent *)malloc(sizeof(struct hostent));


    CopyHostentString(&h2->h_name, h->h_name, "hostent->h_name", mName);

    CopyHostentStringList(&h2->h_aliases, (char **)h->h_aliases, "hostent->h_aliases", mName);

    h2->h_addrtype = h->h_addrtype;
    debug5 << mName << "hostent->h_addrtype = " << h2->h_addrtype << endl;

    h2->h_length = h->h_length;
    debug5 << mName << "hostent->h_length = " << h2->h_length << endl;

    CopyHostentStringList(&h2->h_addr_list, (char **)h->h_addr_list, "hostent->h_addr_list", mName);

    return h2;
}

// ****************************************************************************
// Method: FreeHostent
//
// Purpose: 
//   Frees a hostent structure.
//
// Arguments:
//   h : The hostent structure to free.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 12 12:16:08 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
FreeHostent(struct hostent *h)
{
    if(h->h_name != NULL)
        free(h->h_name);

    if(h->h_aliases != NULL)
    {
        for(int i = 0; h->h_aliases[i] != NULL; ++i)
            free(h->h_aliases[i]);
        free(h->h_aliases);
    }

    if(h->h_addr_list != NULL)
    {
        for(int i = 0; h->h_addr_list[i] != NULL; ++i)
            free(h->h_addr_list[i]);
        free(h->h_addr_list);
    }

    free(h);
} 

// ****************************************************************************
// Method: LogWindowsSocketError
//
// Purpose: 
//   Writes Windows socket error messages to the debug5 log file.
//
// Arguments:
//   mName : The method that called networking functions.
//   fName : The name of the networking function that failed.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 12 12:16:37 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
LogWindowsSocketError(const char *mName, const char *fName)
{
    debug5 << mName << "Call to \"" << fName << "\" failed:";
    switch(WSAGetLastError())
    {
    case WSANOTINITIALISED:
        debug5 << "WSAENOTINITIALISED: WSAStartup() must be called before using this API.";
        break;
    case WSAENETDOWN:
        debug5 << "WSAENETDOWN: The network subsystem or the associated service provider has failed.";
        break;
    case WSAEAFNOSUPPORT:
        debug5 << "WSAEAFNOSUPPORT: The specified address family is not supported.";
        break;
    case WSAEINPROGRESS:
        debug5 << "WSAEINPROGRESS: A blocking Winsock 1.1 call is in progress, or the service provider is still processing a callback function.";
        break;
    case WSAEFAULT:
        debug5 << "WSAEFAULT: See documentation for: " << fName;
        break;
    case WSAEINTR:
        debug5 << "WSAEINTR: A blocking WinSock 1.1 call was canceled via WSACancelBlockingCall.";
        break;
    case WSAEMFILE:
        debug5 << "WSAEMFILE: No more socket descriptors are available.";
        break;
    case WSAENOBUFS:
        debug5 << "WSAENOBUFS: No buffer space is available. The socket cannot be created.";
        break;
    case WSAEPROTONOSUPPORT:
        debug5 << "WSAEPROTONOSUPPORT: The specified protocol is not supported.";
        break;
    case WSAEPROTOTYPE:
        debug5 << "WSAEPROTOTYPE: The specified protocol is the wrong type for this socket.";
        break;
    case WSAESOCKTNOSUPPORT:
        debug5 << "WSAESOCKTNOSUPPORT: The specified socket type is not supported in this address family.";
        break;
    case WSAHOST_NOT_FOUND:
        debug5 << "WSAHOST_NOT_FOUND: Authoratiative Answer Host not found.";
        break;
    case WSATRY_AGAIN:
        debug5 << "WSATRY_AGAIN: Non-Authoratative Host not found, or server failure.";
        break;
    case WSANO_RECOVERY:
        debug5 << "WSANO_RECOVERY: Non-recoverable error occurred.";
        break;
    case WSANO_DATA:
        debug5 << "WSANO_DATA: Valid name, no data record of requested type.";
        break;
    case WSAEINVAL:
        debug5 << "WSAEINVAL: See documentation for: " << fName;
        break;
    case WSAENETRESET:
        debug5 << "WSAENETRESET: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.";
        break;
    case WSAENOPROTOOPT:
        debug5 << "WSAENOPROTOOPT: The option is unknown or unsupported for the specified provider or socket.";
        break;
    case WSAENOTCONN:
        debug5 << "WSAENOTCONN: Connection has been reset when SO_KEEPALIVE is set.";
        break;
    case WSAENOTSOCK:
        debug5 << "WSAENOTSOCK: The descriptor is not a socket.";
        break;
    case WSAEADDRINUSE:
        debug5 << "WSAEADDRINUSE: The socket's local address space is already in use and the socket was not marked to allow address reuse.";
        break;
    case WSAEALREADY:
        debug5 << "WSAEALREADY: A non-blocking connect() call is in progress or the service provider is still processing a callback function.";
        break;
    case WSAEADDRNOTAVAIL:
        debug5 << "WSAADDRNOTAVAIL: The remote address is not valid (e.g. ADDR_ANY).";
        break;
    case WSAECONNREFUSED:
        debug5 << "WSAECONNREFUSED: The attempt to connect was forcefully rejected.";
        break;
    case WSAEISCONN:
        debug5 << "WSAEISCONN: The socket is already connected.";
        break;
    case WSAENETUNREACH:
        debug5 << "WSAENETUNREACH: The network can't be reached from this host at this time.";
        break;
    case WSAETIMEDOUT:
        debug5 << "WSAETIMEDOUT: Attempt to connect timed out without establishing a connection.";
        break;
    case WSAEWOULDBLOCK:
        debug5 << "WSAEWOULDBLOCK: The socket it marked as non-blocking and the connection cannot be completed immediately.";
        break;
    case WSAEACCES:
        debug5 << "WSAEACCES: Attempt to connect datagram socket to broadcast address failed.";
        break;
    case WSAEOPNOTSUPP:
        debug5 << "WSAENOTSUPP: The referenced socket is not of a type that supports listen().";
        break;
    default:
        debug5 << "WSA error: " << WSAGetLastError();
    }
    debug5 << endl;
}

#endif
