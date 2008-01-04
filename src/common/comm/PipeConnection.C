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

#include <PipeConnection.h>
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <CouldNotConnectException.h>
#include <snprintf.h>

#include <DebugStream.h>

#include <errno.h>

int PipeConnection::connectionCount = 0;

// ****************************************************************************
// Method: PipeConnection::PipeConnection
//
// Purpose: 
//   Constructor for the PipeConnection class.
//
// Notes:      This constructor creates a pipe.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 21 13:47:29 PST 2003
//
// Modifications:
//
// ****************************************************************************

PipeConnection::PipeConnection(bool wm) : SocketConnection(-1)
{
   writeMode = wm;

   char pname[100];
   SNPRINTF(pname, 100, "/tmp/fifo%08d.%d", getpid(), connectionCount++);
   pipeName = pname;
   ownsPipe = true;
   if(mknod(pname, S_IFIFO | 0600, 0) == -1)
   {
       debug1 << "Could not create pipe: " << pipeName << ", errno=" << errno << endl;
       EXCEPTION0(CouldNotConnectException);
   }
}

// ****************************************************************************
// Method: PipeConnection::PipeConnection
//
// Purpose: 
//   Constructor for the PipeConnection class.
//
// Arguments:
//   p0 : The second file descriptor.
//   p1 : The first file descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 21 14:33:50 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PipeConnection::PipeConnection(const std::string &pname, bool wm) :
    SocketConnection(-1)
{
    pipeName = pname;
    ownsPipe = false;
    writeMode = wm;
}

// ****************************************************************************
// Method: PipeConnection::~PipeConnection
//
// Purpose: 
//   Destructor for the PipeConnection class.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 21 14:25:32 PST 2003
//
// Modifications:
//
// ****************************************************************************

PipeConnection::~PipeConnection()
{
    if(ownsPipe)
    {
        unlink(pipeName.c_str());
    }
}

// ****************************************************************************
// Method: PipeConnection::GetDescriptorName
//
// Purpose: 
//   Returns the name of the connection's descriptor.
//
// Returns:    The name of the connection's descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 12:59:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
PipeConnection::GetDescriptorName(int = 0) const
{
    return pipeName;
}

// ****************************************************************************
// Method: PipeConnection::Initialize
//
// Purpose: 
//   This method completes the pipe connection's initialization, which in this
//   case means that it opens a handle to the named pipe.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 13:00:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PipeConnection::Initialize()
{
#if 1
    int block = 0;
#else
    int block = O_NDELAY;
#endif

    if(writeMode)
    {
        debug1 << "PipeConnection::Initialize: before open for write" << endl;
        descriptor = open(pipeName.c_str(), O_WRONLY | block);
    }
    else
    {
        debug1 << "PipeConnection::Initialize: before open for read" << endl;
        descriptor = open(pipeName.c_str(), O_RDONLY | block);

        debug1 << "PipeConnection::Initialize: before opening DUMMY for write" << endl;
        int dummy = open(pipeName.c_str(), O_WRONLY | O_NDELAY);
        debug1 << "PipeConnection::Initialize: after opening DUMMY for write. dummy="<<dummy << endl;
    }
}

// ****************************************************************************
// Method: PipeConnection::CloseDescriptor
//
// Purpose: 
//   Closes the specified descriptor.
//
// Arguments:
//   d : The descriptor to close.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 13:01:05 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
PipeConnection::CloseDescriptor(DESCRIPTOR d)
{
    close(d);
}

// ****************************************************************************
// Method: PipeConnection::SendBytes
//
// Purpose: 
//   Sends bytes into the pipe.
//
// Arguments:
//   d     : The descriptor being used to write to the pipe.
//   buf   : The buffer to be sent.
//   count : The length of the buffer to be sent.
//
// Returns:    The number of bytes that were written.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 13:01:33 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PipeConnection::SendBytes(DESCRIPTOR d, const unsigned char *buf, long count)
{
    debug1 << "PipeConnection::SendBytes: before write(): count=" << count
           << ", descriptor=" << d << endl;

    int a = write(d, buf, count);

    debug1 << "PipeConnection::SendBytes: write() returned " << a << ", descriptor=" << d << endl;

    debug1 << "PipeConnection::SendBytes: errno=" << errno << endl;

    if(a < 0) a = 0;

    return a;
}

// ****************************************************************************
// Method: PipeConnection::ReadBytes
//
// Purpose: 
//   Reads bytes from the pipe.
//
// Arguments:
//   d     : The descriptor being used to read from the pipe.
//   buf   : The destination buffer.
//   count : The length of the destination buffer.
//
// Returns:    The number of bytes that were read.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 13:01:33 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
PipeConnection::ReadBytes(DESCRIPTOR d, unsigned char *buf, long bufSize)
{
    debug1 << "PipeConnection::ReadBytes: before read()" << endl;

    int a = read(d, buf, bufSize);

    debug1 << "PipeConnection::ReadBytes: read() returned " << a << ", descriptor=" << d << endl;

    debug1 << "PipeConnection::ReadBytes: errno=" << errno << endl;

    if(a < 0) a = 0;

    return a;
}

