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

#include <SocketConnection.h>
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <strings.h>             // bzero by way of FD_ZERO
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#endif
#include <LostConnectionException.h>

// ****************************************************************************
// Method: SocketConnection::SocketConnection
//
// Purpose: 
//   Constructor for the SocketConnection class.
//
// Arguments:
//   _descriptor : The file descriptor of the socket we want to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:16:31 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 25 12:33:03 PDT 2002
//   Initialized the zeroesRead member.
//
// ****************************************************************************

SocketConnection::SocketConnection(DESCRIPTOR descriptor_) : buffer()
{
    descriptor = descriptor_;
    zeroesRead = 0;
}

// ****************************************************************************
// Method: SocketConnection::~SocketConnection
//
// Purpose: 
//   Destructor for the SocketConnection class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:17:12 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 22 17:07:16 PST 2002
//   Made it close the socket.
//
// ****************************************************************************

SocketConnection::~SocketConnection()
{
#if defined(_WIN32)
     closesocket(descriptor);
#else
     close(descriptor);
#endif
}

// ****************************************************************************
// Method: SocketConnection::GetDescriptor
//
// Purpose: 
//   Returns the socket descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:30:32 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
SocketConnection::GetDescriptor() const
{
    return int(descriptor);
}

// ****************************************************************************
// Method: SocketConnection::Fill
//
// Purpose: 
//   Tries to fill up the connection with new data.
//
// Returns:    The number of bytes read.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 20 17:28:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
SocketConnection::Fill()
{
    unsigned char tmp[1000];
#if defined(_WIN32)
    int amountRead = recv(descriptor, (char FAR *)tmp, 1000, 0);
#else
    int amountRead = recv(descriptor, (void *)tmp, 1000, 0);
#endif

    if(amountRead > 0)
    {
        zeroesRead = 0;

        // Add the new bytes to the buffer.
        for(int i = 0; i < amountRead; ++i)
            buffer.push_back(tmp[i]);
    }
    else
        ++zeroesRead;

    // If we have had a certain number of zero length reads in a row,
    // assume the connection died.
    if(zeroesRead > 100)
    {
         EXCEPTION0(LostConnectionException);
    }

    return amountRead;
}

// ****************************************************************************
// Method: SocketConnection::Flush
//
// Purpose: 
//   Writes the entire contents of the connection's buffer onto the
//   socket file descriptor in chunks. It then clears the buffer.
//
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:17:37 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 26 13:29:20 PST 2002
//   Made it use socket functions so it is more portable.
//
// ****************************************************************************

void
SocketConnection::Flush()
{
    const int bufSize = 500;
    unsigned char buf[bufSize];
    int count = 0;

    // Write out the entire buffer, in pieces, to the socket descriptor.
    for(int bufindex = 0; bufindex < buffer.size(); ++bufindex)
    {
        buf[count++] = buffer[bufindex];

        // Write the chunk to the socket descriptor            
        if((count == bufSize) || (bufindex == buffer.size() - 1))
        {
#if defined(_WIN32)
            send(descriptor, (const char FAR *)buf, count, 0);
#else
            send(descriptor, (const void *)buf, count, 0);
#endif

            count = 0;
        }
    }

    // Clear the buffer
    buffer.clear();
}

// ****************************************************************************
// Method: SocketConnection::Size
//
// Purpose: 
//   Returns the size of the connection's buffer. This is how many
//   bytes can still be consumed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:18:32 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

long
SocketConnection::Size()
{
    return (long)buffer.size();
}

// ****************************************************************************
// Method: SocketConnection::Write
//
// Purpose: 
//   Appends a character to the connection's buffer.
//
// Arguments:
//   value : The character to append.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:19:23 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
SocketConnection::Write(unsigned char value)
{
    buffer.push_back(value);
}

// ****************************************************************************
// Method: SocketConnection::Read
//
// Purpose: 
//   Reads a character from the start of the connection's buffer.
//
// Arguments:
//   address : The memory location in which to store the character.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:20:03 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
SocketConnection::Read(unsigned char *address)
{
   *address = buffer.front();
    buffer.pop_front();
}

// ****************************************************************************
// Method: SocketConnection::Append
//
// Purpose: 
//   Adds characters to the end of the connection's buffer.
//
// Arguments:
//   buf   : An array of characters to append.
//   count : The length of the array.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:20:50 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
SocketConnection::Append(const unsigned char *buf, int count)
{
    const unsigned char *temp = buf;
    for(int i = 0; i < count; ++i)
        buffer.push_back(*temp++);
}

// ****************************************************************************
// Method: SocketConnection::DirectRead
//
// Purpose: 
//   Reads the specified number of bytes from the connection.
//
// Arguments:
//   buf    : The destination buffer.
//   ntotal : The number of bytes to read.
//
// Returns:    The number of bytes read.
//
// Note:       This method does not return until the specified number of
//             bytes have been read. This method was adapted from Jeremy's
//             read_n function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:28:39 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jul 19 11:41:13 PDT 2004
//   I fixed a problem reading large messages on Windows.
//
// ****************************************************************************

long
SocketConnection::DirectRead(unsigned char *buf, long ntotal)
{
    char *b = (char*)buf;
    long nread = 0;

    while (nread < ntotal)
    {
        int n = recv(descriptor, b, ntotal-nread, 0);

        if (n < 0)
        {
#if defined(_WIN32)
            //
            // If the last socket error was any of the errors that are
            // typically set by recv then return. For some large messages
            // that VisIt reads, recv returns -1 indicating some kind of
            // failure but the error code does match anything fatal so
            // don't bother returning. In the situations I've observed,
            // it's been okay to continue calling recv until it succeeds.
            //
            int err = WSAGetLastError();
            if(err == WSANOTINITIALISED ||
               err == WSAENETDOWN ||
               err == WSAEFAULT ||
               err == WSAENOTCONN ||
               err == WSAEINTR ||
               err == WSAEINPROGRESS ||
               err == WSAENETRESET ||
               err == WSAENOTSOCK ||
               err == WSAEOPNOTSUPP ||
               err == WSAESHUTDOWN ||
               err == WSAEWOULDBLOCK ||
               err == WSAEMSGSIZE ||
               err == WSAEINVAL ||
               err == WSAECONNABORTED ||
               err == WSAETIMEDOUT ||
               err == WSAECONNRESET ||
               err == WSAEDISCON)
            {
                return long(n);
            }
#else
            return long(n);
#endif
        }
        else if(n == 0)
        {
            ++zeroesRead;

            if(zeroesRead > 100)
            {
                EXCEPTION0(LostConnectionException);
            }
        }
        else
        {
            nread += n;
            b += n;

            // Reset the flag.
            zeroesRead = 0;
        }
    }

    return ntotal;
}

// ****************************************************************************
// Method: SocketConnection::DirectWrite
//
// Purpose: 
//   Writes a buffer to the connection without doing any buffering.
//
// Arguments:
//   buf    : The buffer that we want to write.
//   ntotal : The length of the buffer.
//
// Returns:    The number of bytes written.
//
// Note:       This method was adapted from Jeremy's write_n function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:33:40 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jan 30 17:39:47 PST 2004
//    Fixed a bug where failed sends would restart from the beginning.
//
// ****************************************************************************

long
SocketConnection::DirectWrite(const unsigned char *buf, long ntotal)
{
    size_t          nleft, nwritten;
    const char      *ptr;

    ptr = (const char*)buf;
    nleft = ntotal;
    while (nleft > 0)
    {
        if((nwritten = send(descriptor, (const char *)ptr, nleft, 0)) <= 0)
            return nwritten;

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return ntotal;
}

// ****************************************************************************
// Method: SocketConnection::NeedsRead
//
// Purpose: 
//   Returns whether or not the connection has data that could be read.
//
// Arguments:
//   blocking : Specifies whether we should block until input needs to be
//              read or whether we should poll.
//
// Returns:  True if there is data, false otherwise.  
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 16:01:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
SocketConnection::NeedsRead(bool blocking) const
{
    // Set up a file descriptor set that only consists of the descriptor
    // used by this connection.
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(descriptor, &readSet);

    int ret = 0;
    if(blocking)
    {
        ret = select(descriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
                     NULL);
    }
    else
    {
        // Create a null timeout that will cause select to poll.
        struct timeval timeout = {0,0};
        ret = select(descriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
                     &timeout);
    }

    return (ret > 0);
}
