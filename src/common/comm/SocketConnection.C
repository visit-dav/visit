/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <SocketConnection.h>
#include <AttributeSubject.h>
#include <AttributeSubjectSerialize.h>
#include <visit-config.h>
#if defined(_WIN32)
#include <winsock2.h>
#include <win32commhelpers.h>
#else
#include <strings.h>             // bzero by way of FD_ZERO
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#endif
#ifdef HAVE_SELECT
#include <SysCall.h>
#endif

#include <DebugStream.h>
#include <LostConnectionException.h>

#include <cstdlib>
#include <string.h>

//#define VISIT_DEBUG_SOCKETS
#ifdef VISIT_DEBUG_SOCKETS
#define DEBUG_SOCKETS(CODE) if(DebugStream::Level5()){CODE}
#define DEBUG_SOCKETS_CODE(CODE) CODE
#else
#define DEBUG_SOCKETS(CODE)
#define DEBUG_SOCKETS_CODE(CODE)
#endif

// This value determines the size of the buffer exchanged when we send fixed
// buffer sizes over the network.
const int SocketConnection::FIXED_BUFFER_SIZE = VISIT_SOCKET_BUFFER_SIZE;

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
    fixedBufferMode = true;
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

long
SocketConnection::ReadHeader(unsigned char *buf, long len)
{
    return DirectRead(buf,len);
}

long
SocketConnection::WriteHeader(const unsigned char *buf, long len)
{
    return DirectWrite(buf,len);
}

// ****************************************************************************
// Method: SocketConnection::DecodeSize
//
// Purpose:
//   Takes the first bytes of a buffer and decodes the size header from it.
//
// Arguments:
//   buf    : The buffer in which the header and data are stored.
//   offset : The size of the header.
//
// Returns:    The number of bytes in the buffer.
//
// Note:       This only needed in fixed buffer mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 14:46:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
SocketConnection::DecodeSize(const unsigned char *buf, int &offset) const
{
    int a = int(buf[0]);
    int b = int(buf[1]);
    offset = 2;
    return a*256 + b;
}

// ****************************************************************************
// Method: SocketConnection::EncodeSize
//
// Purpose:
//   Takes a size and encodes it as a header size in the provided buffer.
//
// Arguments:
//   sz     : The size.
//   buf    : The buffer in which to store the header.
//   offset : The header size.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 14:46:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SocketConnection::EncodeSize(int sz, unsigned char *buf, int &offset) const
{
    int a = sz / 256;
    int b = sz % 256;
    buf[0] = (unsigned char)(a);
    buf[1] = (unsigned char)(b);
    offset = 2;
}

// ****************************************************************************
// Method: SocketConnection::SetFixedBufferMode
//
// Purpose:
//   Set the socket's fixed buffer mode.
//
// Arguments:
//   val : Whether to do fixed buffer mode.
//
// Returns:    
//
// Note:       Fixed buffer mode is a communication style where we send 
//             fixed-size chunks of data that may contain smaller variable-length
//             payloads.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 15:50:00 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SocketConnection::SetFixedBufferMode(bool val)
{
    fixedBufferMode = val;
#ifndef _WIN32
    if(fixedBufferMode)
    {
        debug5 << "Socket " << descriptor << " was changed to fixed buffer mode."  << endl;
    }
#endif
}

// ****************************************************************************
// Method: SocketConnection::GetFixedBufferMode
//
// Purpose:
//   Get the socket's fixed buffer mode.
//
// Arguments:
//   val : Whether to do fixed buffer mode.
//
// Returns:
//
// Note:       Fixed buffer mode is a communication style where we send
//             fixed-size chunks of data that may contain smaller variable-length
//             payloads.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 15:50:00 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SocketConnection::GetFixedBufferMode()
{
    return fixedBufferMode;
}
// ****************************************************************************
// Method: SocketConnection::WaitForDescriptor
//
// Purpose:
//   Wait for the file descriptor to be ready.
//
// Arguments:
//   input : True if we're receiving input; False otherwise.
//
// Returns:    
//
// Note:       This is only used for fixed buffer mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 14:44:59 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SocketConnection::WaitForDescriptor(bool input)
{
#if !defined(_WIN32) && !defined(__APPLE__)
    DEBUG_SOCKETS_CODE(const char *mName = "SocketConnection::WaitForDescriptor: ";)
    int rc = 0;
    while (rc == 0)
    {
        struct pollfd ufds[1];
        ufds[0].fd = descriptor;
        ufds[0].events = input ? POLLIN : POLLOUT;
        ufds[0].revents = 0;
        DEBUG_SOCKETS(debug5 << mName << "before poll" << endl;)
        rc = poll(ufds, 1, 100);
        DEBUG_SOCKETS(debug5 << mName << "poll returned " << rc << endl;)

        if (rc == -1)
        {
            DEBUG_SOCKETS(debug5 << mName << "Error polling socket" << endl;)
            return;
        }
        else if (rc == 0)
        {
            // We need to wait longer, 
        }
        else
        {
            if(input)
            {
                if(ufds[0].revents & POLLIN)
                {
                    // We have input.
                    DEBUG_SOCKETS(debug5 << mName << "we expect input" << endl;)
                    break;
                }
            }
            else
            {
                if(ufds[0].revents & POLLOUT)
                {
                    // We can output.
                    DEBUG_SOCKETS(debug5 << mName << "we expect output" << endl;)
                    break;
                }
            }
        }
    }
#endif
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
//   Brad Whitlock, Tue Oct 14 14:02:15 PDT 2014
//   I added fixedBufferMode where we always wait for the same size recv buffer
//   but the data encoded therein uses the first bytes of the buffer to encode
//   the length of the data payload.
//
//   Brad Whitlock, Wed Oct 29 10:08:14 PDT 2014
//   Add lost connection exception coding for fixed buffer mode so we don't
//   have programs hanging when there's no way they'll get more input.
//
// ****************************************************************************

int
SocketConnection::Fill()
{
    DEBUG_SOCKETS_CODE(const char *mName = "SocketConnection::Fill: ";)
    DEBUG_SOCKETS(debug5 << mName << "begin" << endl;)

    if(destFormat.Format == TypeRepresentation::ASCIIFORMAT) {

        buffer.clear();

        AttributeSubjectSerialize s;
        s.SetConnection(this);
        int amt = s.Fill();
        return amt;
    }

    /// The code below should move into AttributeSubjectSerialize
    const int iteration_limit = 100;
    unsigned char tmp[VISIT_SOCKET_BUFFER_SIZE];

    // If we're in fixed buffer mode, make sure we get VISIT_SOCKET_BUFFER_SIZE bytes
    // before we process the message coming through. Most of the time, we
    // should get the buffer size amount but, just in case, we loop until
    // we get the right message size.
    int totalRead = 0;
    int amountRead = 0;
    do
    {
        unsigned char *ptr = tmp + totalRead;

        if(fixedBufferMode)
        {
            WaitForDescriptor(true);
        }

        DEBUG_SOCKETS(debug5 << mName << "recv(bufferSize="
                             << VISIT_SOCKET_BUFFER_SIZE << ")" << endl;)

#if defined(_WIN32)
        amountRead = recv(descriptor, (char FAR *)ptr, VISIT_SOCKET_BUFFER_SIZE-totalRead, 0);
        if(amountRead == SOCKET_ERROR)
        {
            LogWindowsSocketError("SocketConnection", "Fill");
            if(WSAGetLastError() == WSAEWOULDBLOCK)
                return -1;
        }
#else
        amountRead = recv(descriptor, (void *)ptr, VISIT_SOCKET_BUFFER_SIZE-totalRead, 0);
#endif

        if(fixedBufferMode)
        {
            if(amountRead == 0)
            {
                ++zeroesRead;
                if(zeroesRead > iteration_limit)
                {
                    EXCEPTION0(LostConnectionException);
                }
            }
            else
                zeroesRead = 0; 
        }

        totalRead += amountRead;
    }
    while(fixedBufferMode && totalRead < VISIT_SOCKET_BUFFER_SIZE);

    if(totalRead > 0)
    {
        zeroesRead = 0;

        DEBUG_SOCKETS(
            debug5 << mName << " recv {";
            int n = std::min(10, totalRead);
            for(int i = 0; i < n; ++i)
                debug5 << int(tmp[i]) << ", ";
            if(n < totalRead)
                debug5 << "...";
            debug5 << "}" << endl;
        )

        if(fixedBufferMode)
        {
            DEBUG_SOCKETS(debug5 << mName << "from socket, totalRead="
                                 << totalRead << endl;)

            // Get the real number of bytes from the header.
            int offset = 0;
            int msgLen = DecodeSize(tmp, offset);

            DEBUG_SOCKETS(debug5 << mName << "msgLen=" << msgLen << endl;)

            // Add the new bytes to the buffer.
            for(int i = 0; i < msgLen; ++i)
                buffer.push_back(tmp[i + offset]);
        }
        else
        {
            // Add the new bytes to the buffer.
            for(int i = 0; i < totalRead; ++i)
                buffer.push_back(tmp[i]);
        }
    }
    else
        ++zeroesRead;

    // If we have had a certain number of zero length reads in a row,
    // assume the connection died.
    if(zeroesRead > iteration_limit)
    {
         EXCEPTION0(LostConnectionException);
    }

    DEBUG_SOCKETS(debug5 << mName << "end: amountRead=" << amountRead << endl;)

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
//   Brad Whitlock, Thu Jan 25 18:42:50 PST 2007
//   I made it use MSG_NOSIGNAL so we don't get a signal in the event that
//   we can't write to the socket.
//
//   Eric Brugger, Tue Mar 13 09:18:48 PDT 2007
//   I made the use of MSG_NOSIGNAL conditional on its definition.
//
//   Brad Whitlock, Tue Oct 14 14:31:13 PDT 2014
//   I added a mode where we can encode data in fixed length buffers that we
//   send.
//
// ****************************************************************************

void
SocketConnection::Flush()
{
    DEBUG_SOCKETS_CODE(const char *mName = "SocketConnection::Flush: ";)
    unsigned char buf[VISIT_SOCKET_BUFFER_SIZE];

    if(fixedBufferMode)
    {
        memset(buf, 0, VISIT_SOCKET_BUFFER_SIZE * sizeof(unsigned char));

        // Get the header size.
        int headerSize = 0;
        EncodeSize(0, buf, headerSize);

        int offset = 0;
        int unsent = (int)buffer.size();
        DEBUG_SOCKETS(debug5 << mName << "fixed buffer send. buffer.size=" << buffer.size() << endl;)

        int maxData = VISIT_SOCKET_BUFFER_SIZE - headerSize;

        while(unsent > 0)
        {
            // Get the desired number of bytes for this chunk.
            int thisChunkSize = (unsent < maxData) ? unsent : maxData;
            int ci = 0;
            EncodeSize(thisChunkSize, buf, ci);

            // Fill this chunk.
            for(int i = 0; i < thisChunkSize; ++i)
                buf[ci++] = buffer[offset + i];

            // Wait for the descriptor to be ready.
            WaitForDescriptor(false);

            DEBUG_SOCKETS(debug5 << mName << "sending " << VISIT_SOCKET_BUFFER_SIZE 
                                 << " bytes (" << thisChunkSize 
                                 << " are real) {";
                int n = std::min(10, thisChunkSize);
                for(int i = 0; i < n; ++i)
                    debug5 << int(buf[i]) << ", ";
                if(n < thisChunkSize)
                    debug5 << "...";
                debug5 << "}" << endl;
            )

            // Send this fixed size chunk.
#if defined(_WIN32)
            DEBUG_SOCKETS_CODE(int sz = ) send(descriptor, (const char FAR *)buf, VISIT_SOCKET_BUFFER_SIZE, 0);
#else
#ifdef MSG_NOSIGNAL
            DEBUG_SOCKETS_CODE(ssize_t sz = ) send(descriptor, (const void *)buf, VISIT_SOCKET_BUFFER_SIZE, MSG_NOSIGNAL);
#else
            DEBUG_SOCKETS_CODE(ssize_t sz = ) send(descriptor, (const void *)buf, VISIT_SOCKET_BUFFER_SIZE, 0);
#endif
#endif
            DEBUG_SOCKETS(debug5 << mName << "sent " << sz << " bytes" << endl;)

            unsent -= thisChunkSize;
            offset += thisChunkSize;
        }
    }
    else
    {
        DEBUG_SOCKETS(debug5 << mName << "normal buffer send. buffer.size=" << buffer.size() << endl;)

        const int bufSize = VISIT_SOCKET_BUFFER_SIZE;
        int count = 0;
        // Write out the entire buffer, in pieces, to the socket descriptor.
        for(size_t bufindex = 0; bufindex < buffer.size(); ++bufindex)
        {
            buf[count++] = buffer[bufindex];

            // Write the chunk to the socket descriptor            
            if((count == bufSize) || (bufindex == buffer.size() - 1))
            {

                DEBUG_SOCKETS(
                    debug5 << mName << "sending " << count 
                           << " bytes {";
                    int n = std::min(10, count);
                    for(int i = 0; i < n; ++i)
                        debug5 << int(buf[i]) << ", ";
                    if(n < count)
                        debug5 << "...";
                    debug5 << "}" << endl;
                )
#if defined(_WIN32)
                DEBUG_SOCKETS_CODE(int sz = ) send(descriptor, (const char FAR *)buf, count, 0);
#else
#ifdef MSG_NOSIGNAL
                DEBUG_SOCKETS_CODE(ssize_t sz = ) send(descriptor, (const void *)buf, count, MSG_NOSIGNAL);
#else
                DEBUG_SOCKETS_CODE(ssize_t sz = ) send(descriptor, (const void *)buf, count, 0);
#endif
#endif
                DEBUG_SOCKETS(debug5 << mName << "sent " << sz << " bytes" << endl;)

                count = 0;
            }
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
// Method: SocketConnection::Reset
//
// Purpose:
//   Resets the connection's buffer.
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
SocketConnection::Reset()
{
    buffer.clear();
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
    DEBUG_SOCKETS_CODE(const char *mName = "SocketConnection::DirectRead: ";)
    DEBUG_SOCKETS(debug5 << mName << "begin: ntotal=" << ntotal << endl; )

    char *b = (char*)buf;
    long nread = 0;

    while (nread < ntotal)
    {
        DEBUG_SOCKETS(debug5 << mName << "recv: size=" << (ntotal-nread) << endl; )
        int n = recv(descriptor, b, ntotal-nread, 0);
        DEBUG_SOCKETS(debug5 << mName << "recv returned " << n << " bytes." << endl; )

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

    DEBUG_SOCKETS(debug5 << mName << "end" << endl;)

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
    DEBUG_SOCKETS_CODE(const char *mName = "SocketConnection::DirectWrite: ";)
    DEBUG_SOCKETS(debug5 << mName << "begin: ntotal=" << ntotal << endl;)

    size_t      nleft, nwritten;
    const char *ptr = (const char*)buf;
    nleft = ntotal;
    while (nleft > 0)
    {
        DEBUG_SOCKETS(debug5 << mName << "send " << nleft << " bytes" << endl;)
        if((nwritten = send(descriptor, (const char *)ptr, nleft, 0)) <= 0)
            return nwritten;
        DEBUG_SOCKETS(debug5 << mName << "sent " << nwritten << " bytes" << endl;)
        nleft -= nwritten;
        ptr   += nwritten;
    }

    DEBUG_SOCKETS(debug5 << mName << "end" << endl;)

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
//    Tom Fogal, Sat Feb 16 15:47:15 EST 2008
//    Restart the system call if it gets interrupted.
//
//    Brad Whitlock, Thu Jun 11 15:14:50 PST 2009
//    Don't call select if we don't have it.
//
// ****************************************************************************

bool
SocketConnection::NeedsRead(bool blocking) const
{
#ifdef HAVE_SELECT
    // Set up a file descriptor set that only consists of the descriptor
    // used by this connection.
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(descriptor, &readSet);

    int ret = 0;
    if(blocking)
    {
        ret = RESTART_SELECT(descriptor+1, &readSet,
                             static_cast<fd_set*>(NULL),
                             static_cast<fd_set*>(NULL),
                             static_cast<struct timeval *>(NULL));
    }
    else
    {
        // Create a null timeout that will cause select to poll.
        struct timeval timeout = {0,0};
        ret = RESTART_SELECT(descriptor+1, &readSet,
                             static_cast<fd_set*>(NULL),
                             static_cast<fd_set*>(NULL),
                             &timeout);
    }

    return (ret > 0);
#else
    // Assume that read will block for input.
    return true;
#endif
}

std::string
AttributeSubjectSocketConnection::serializeMetaData(AttributeSubject *subject)
{
    JSONNode meta;
    JSONNode node;

    subject->WriteAPI(meta);

    node["id"] = subject->GetGuido();
    node["typename"] = subject->TypeName();
    node["api"] = meta; //.ToJSONNode(false,false);

    return node.ToString();
}

std::string
AttributeSubjectSocketConnection::serializeAttributeSubject(AttributeSubject *subject) {
    JSONNode child, metadata;
    JSONNode node;

    subject->Write(child);
    subject->WriteMetaData(metadata);
    node["id"] = subject->GetGuido();
    node["typename"] = subject->TypeName();
    node["contents"] = child; //.ToJSONNode(false,true);
    node["metadata"] = metadata;

    return node.ToString();
}
