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

#ifndef COMMUNICATION_HEADER_H
#define COMMUNICATION_HEADER_H
#include <comm_exports.h>
#include <TypeRepresentation.h>
#include <string>

class Connection;

// ****************************************************************************
// Class: CommunicationHeader
//
// Purpose:
//   This class contains the information that is exchanged between components
//   when they first connect to each other.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 13:57:58 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Apr 27 15:28:07 PDT 2001
//    Added fail bit to WriteHeader.
//
//    Brad Whitlock, Mon Mar 25 14:06:21 PST 2002
//    I made ReadHeader and WriteHeader use Connection objects.
//
//    Brad Whitlock, Mon Dec 16 13:11:30 PST 2002
//    I added a method to return a random key.
//
// ****************************************************************************

class COMM_API CommunicationHeader
{
public:
    CommunicationHeader();
    ~CommunicationHeader();
    void WriteHeader(Connection *conn, const std::string &version,
                     const std::string &key1, const std::string &key2,
                     int failCode = 0);
    void ReadHeader(Connection *conn, const std::string &version,
                    bool checkKeys = true);
    const TypeRepresentation &GetTypeRepresentation() const;

    static std::string CreateRandomKey(int len = 10);
    void SetSecurityKey(const std::string &key) { securityKey = key; }
    const std::string &GetSocketKey() const { return socketKey; }
private:
    TypeRepresentation rep;
    std::string        securityKey, socketKey;
};

#endif
