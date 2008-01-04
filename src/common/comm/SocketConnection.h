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

// ************************************************************************* //
//                              SocketConnection.h                           //
// ************************************************************************* //

#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H
#include <comm_exports.h>

#include <Connection.h>
#include <deque>

// ****************************************************************************
// Class: SocketConnection
//
// Purpose:
//   This is a buffered connection that can write itself onto a
//   socket.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:11:22 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 20 17:23:43 PST 2002
//   Added Fill, DirectRead, DirectWrite, NeedsRead methods.
//
// ****************************************************************************

class COMM_API SocketConnection : public Connection
{
public:
    SocketConnection(DESCRIPTOR descriptor_);
    virtual ~SocketConnection();

    virtual int  Fill();
    virtual void Flush();
    virtual long Size();
    virtual void Write(unsigned char value);
    virtual void Read(unsigned char *address);
    virtual void Append(const unsigned char *buf, int count);
    virtual long DirectRead(unsigned char *buf, long len);
    virtual long DirectWrite(const unsigned char *buf, long len);
    virtual bool NeedsRead(bool blocking = false) const;
    virtual int  GetDescriptor() const;
private:
    std::deque<unsigned char> buffer;
    DESCRIPTOR                descriptor;
    int                       zeroesRead;
};

#endif
