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

#ifndef CONNECT_RPC_H
#define CONNECT_RPC_H
#include <mdsrpc_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
// Class: ConnectRPC
//
// Purpose:
//   This class encodes an RPC that tells the MDserver to connect to another
//   process.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:49:14 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 15:58:30 PST 2002
//   I added a security key feature.
//
//   Brad Whitlock, Mon Jun 16 13:39:48 PST 2003
//   I changed the definition so all fields are encapsulated in a string vector.
//
//   Brad Whitlock, Fri Dec  7 11:09:08 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class MDSERVER_RPC_API ConnectRPC : public NonBlockingRPC
{
public:
    ConnectRPC();
    virtual ~ConnectRPC();

    // Invokation method
    void operator()(const stringVector &args);

    // Property selection methods
    virtual void SelectAll();

    // Access methods
    const stringVector &GetArguments() const;

    virtual const std::string TypeName() const;
private:
    stringVector arguments;
};


#endif
