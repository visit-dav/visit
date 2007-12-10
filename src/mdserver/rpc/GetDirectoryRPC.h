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

#ifndef _GETDIRECTORY_RPC_H_
#define _GETDIRECTORY_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// *******************************************************************
// Class: GetDirectoryRPC
//
// Purpose:
//   This RPC gets the current directory from a remote file system.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:56:10 PDT 2000
//
// Notes:
//   The cwd attribute contains that data returned from executing
//   the RPC.
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 10:57:00 PDT 2000
//   I moved DirectoryName's implementation to the .C file.
//
//   Brad Whitlock, Fri Dec  7 11:39:33 PST 2007
//   Added TypeName override.
//
// *******************************************************************

class MDSERVER_RPC_API GetDirectoryRPC : public BlockingRPC
{
public:
    struct MDSERVER_RPC_API DirectoryName : public AttributeSubject
    {
        std::string name;
    public:
        DirectoryName();
        ~DirectoryName();
        virtual void SelectAll();
        virtual const std::string TypeName() const;
    };
public:
    GetDirectoryRPC();
    virtual ~GetDirectoryRPC();

    // Invokation method
    std::string operator()();

    // Property selection methods
    virtual void SelectAll();

    virtual const std::string TypeName() const;
private:
    DirectoryName cwd;
};


#endif
