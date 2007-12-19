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

#include <CommunicationHeader.h>
#include <Connection.h>
#include <DebugStream.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <CancelledConnectException.h>
#include <CouldNotConnectException.h>
#include <Utility.h>
#include <string.h>       // for strcmp
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// This is the size of the buffer that gets communicated.
#define BUFFER_SIZE 100

//
// Uncomment if we're debugging the communication header code.
//
//#define DEBUG_COMMUNICATION_HEADER


// ****************************************************************************
// Method: CommunicationHeader::CommunicationHeader
//
// Purpose: 
//   Constructor for the CommunicationHeader class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 11:47:21 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Dec 16 15:39:51 PST 2002
//   Added initialization for securityKey and socketKey.
//
// ****************************************************************************

CommunicationHeader::CommunicationHeader() : rep(), securityKey(), socketKey()
{
}

// ****************************************************************************
// Method: CommunicationHeader::~CommunicationHeader
//
// Purpose: 
//   Destructor for the CommunicationHeader class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 11:48:11 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

CommunicationHeader::~CommunicationHeader()
{
    // nothing here.
}

// ****************************************************************************
// Method: CommunicationHeader::WriteHeader
//
// Purpose: 
//   Writes a message header using the specified file descriptor.
//
// Arguments:
//   conn : The connection object to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 11:49:02 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Fri Apr 27 15:27:42 PDT 2001
//   Added fail bit.
//   
//   Brad Whitlock, Fri May 11 13:22:07 PST 2001
//   Fixed a problem that caused the remote machines type representation to
//   be served right back to it.
//
//   Brad Whitlock, Tue Mar 19 17:00:11 PST 2002
//   Changed it so it uses a connection object.
//
//   Brad Whitlock, Mon Dec 16 14:41:49 PST 2002
//   I made it pass some security keys.
//
// ****************************************************************************

void
CommunicationHeader::WriteHeader(Connection *conn, const std::string &version,
    const std::string &key1, const std::string &key2, int failCode)
{
    // Store the security keys. securityKey is the key that was sent as a 
    // command line argument. socketKey is the key that is sent over the
    // socket. When we read back from the other process, it must have returned
    // both of the keys for us to believe it is a friendly process.
    securityKey = key1;
    socketKey = key2;

    // Zero out the buffer that we will be sending.
    unsigned char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // The first 4 bytes of the header are for the type representation.
    TypeRepresentation localRep;
    buffer[0] = localRep.IntFormat;
    buffer[1] = localRep.LongFormat;
    buffer[2] = localRep.FloatFormat;
    buffer[3] = localRep.DoubleFormat;

    // Set the failure code
    //   0 : no failure
    //   1 : IncompatibleVersionException
    //   2 : IncompatibleSecurityTokenException
    //   3 : CouldNotConnectException
    //   4 : CancelledConnectException
    //
    buffer[4] = (failCode >= 0 && failCode < 5) ? ((unsigned char)failCode) : 3;

    // The next 10 bytes are for a NULL terminated version string.
    strncpy((char *)(buffer+5), version.c_str(), 10);

    // The next 21 bytes are for securityKey.
    strncpy((char *)(buffer+5+10), securityKey.c_str(), 21);

    // The next 21 bytes are for socketKey.
    strncpy((char *)(buffer+5+10+21), socketKey.c_str(), 21);

#ifdef DEBUG_COMMUNICATION_HEADER
    debug1 << "CommunicationHeader::WriteHeader: HEADER={";
    for(int i = 0; i < BUFFER_SIZE; ++i)
    {
        char str[10];
        if(buffer[i] < 32 || buffer[i] > 'z')
            sprintf(str, "x%d, ", buffer[i]);
        else
            sprintf(str, "%c, ", buffer[i]);
        debug1 << str;
    }
    debug1 << "}\n";
#endif

    // The rest of the bytes are reserved for future use.

    // Write the message header to the file descriptor.
    conn->DirectWrite(buffer, BUFFER_SIZE);
}

// ****************************************************************************
// Method: CommunicationHeader::ReadHeader
//
// Purpose: 
//   Reads a communication header from the specified file descriptor.
//
// Arguments:
//   conn : The connection object to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 11:50:01 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Apr 27 15:27:42 PDT 2001
//    Added fail bit.
//
//    Brad Whitlock, Mon Mar 25 14:09:10 PST 2002
//    Made it use a connection object.
//
//    Brad Whitlock, Mon Dec 16 15:27:52 PST 2002
//    I added security code.
//
//    Brad Whitlock, Mon Oct 8 18:31:49 PST 2007
//    Made it use VersionsCompatible function.
//
//    Jeremy Meredith, Mon Dec 10 17:00:06 EST 2007
//    Renamed VersionsCompatible to VisItVersionsCompatible because I moved
//    the functions to a global namespace in -lutility.
//
//    Hank Childs, Wed Dec 19 08:48:23 PST 2007
//    Add debug statement.
//
// ****************************************************************************

void
CommunicationHeader::ReadHeader(Connection *conn, const std::string &version,
    bool checkKeys)
{
    // Zero out the buffer that we will be reading.
    unsigned char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Write the message header using the file descriptor.
    conn->DirectRead(buffer, BUFFER_SIZE);

    // Fill the local type representation "rep".
    rep.IntFormat = buffer[0];
    rep.LongFormat = buffer[1];
    rep.FloatFormat = buffer[2];
    rep.DoubleFormat = buffer[3];

#ifdef DEBUG_COMMUNICATION_HEADER
    debug1 << "CommunicationHeader::ReadHeader: HEADER={";
    for(int i = 0; i < BUFFER_SIZE; ++i)
    {
        char str[10];
        if(buffer[i] < 32 || buffer[i] > 'z')
            sprintf(str, "x%d, ", buffer[i]);
        else
            sprintf(str, "%c, ", buffer[i]);
        debug1 << str;
    }
    debug1 << "}\n";
#endif

    // Check to see if the version numbers are compatible.
    if(!VisItVersionsCompatible((const char *)(buffer+5), version.c_str()))
    {
        debug1 << "Versions are " << buffer << "(" << buffer+5 << ")," 
               << version << endl;
        EXCEPTION0(IncompatibleVersionException);
    }

    // Check to see if any failure codes are set.
    if(buffer[4] == 1)
    {
        EXCEPTION0(IncompatibleVersionException);
    }
    else if(buffer[4] == 2)
    {
        EXCEPTION0(IncompatibleSecurityTokenException);
    }
    else if(buffer[4] == 3)
    {
        EXCEPTION0(CouldNotConnectException);
    }
    else if(buffer[4] == 4)
    {
        EXCEPTION0(CancelledConnectException);
    }

    // Check to see if the keys that were passed back in the header are
    // the same as the keys that were sent to the client.
    if(checkKeys)
    {
        if((strcmp((const char *)(buffer+5+10), securityKey.c_str()) != 0) ||
           (strcmp((const char *)(buffer+5+10+21), socketKey.c_str()) != 0))
        {
            EXCEPTION0(IncompatibleSecurityTokenException);
        }
    }
    securityKey = std::string((const char *)(buffer+5+10));
    socketKey = std::string((const char *)(buffer+5+10+21));

#ifdef DEBUG_COMMUNICATION_HEADER
    debug1 << "CommunicationHeader::ReadHeader: securityKey=" << securityKey.c_str() << endl;
    debug1 << "CommunicationHeader::ReadHeader: socketKey=" << socketKey.c_str() << endl;
#endif

    // Add other checks in the future as we have more items in the
    // message header.
}

// ****************************************************************************
// Method: CommunicationHeader::GetTypeRepresentation
//
// Purpose: 
//   Returns a reference to the header's type representation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 11:51:23 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const TypeRepresentation &
CommunicationHeader::GetTypeRepresentation() const
{
    return rep;
}

// ****************************************************************************
// Method: CommunicationHeader::CreateRandomKey
//
// Purpose: 
//   Creates a random key string with the specified length.
//
// Arguments:
//   len : The number of 2 digit hex numbers to create.
//
// Returns:    A string containing len 2-digit hex numbers.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 16 14:16:06 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Jan 21 11:06:36 PDT 2003
//   I added code to seed the random number generator so the keys are
//   different when the program is run.
//
//   Brad Whitlock, Mon Feb 10 11:30:11 PDT 2003
//   I made the random number stuff work on Windows.
//
//   Brad Whitlock, Mon May 19 17:25:31 PST 2003
//   Made it use lrand48 because it's slightly faster.
//
// ****************************************************************************

std::string
CommunicationHeader::CreateRandomKey(int len)
{
    // Seed the random number generator based on the time.
#if defined(_WIN32)
    srand((unsigned)time(0));
#else
    srand48(long(time(0)));
#endif

    std::string key;
    for(int i = 0; i < len; ++i)
    {
        char str[3];
#if defined(_WIN32)
        double d = double(rand()) / double(RAND_MAX);
        sprintf(str, "%02x", int(d * 255.));
#else
        sprintf(str, "%02x", int(lrand48() % 256));
#endif
        key += std::string(str);
    }

    return key;
}
