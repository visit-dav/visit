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

#ifndef _EXPANDPATH_RPC_H_
#define _EXPANDPATH_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: ExpandPathRPC
//
// Purpose:
//   This RPC expands pathnames to their fullest.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:38:19 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec  7 11:34:01 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class MDSERVER_RPC_API ExpandPathRPC : public BlockingRPC
{
public:
    struct MDSERVER_RPC_API PathName : public AttributeSubject
    {
        std::string name;
    public:
        PathName();
        ~PathName();
        virtual void SelectAll();
        virtual const std::string TypeName() const;
    };
public:
    ExpandPathRPC();
    virtual ~ExpandPathRPC();

    // Invocation method
    std::string operator()(const std::string &);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPath(const std::string &p);

    // Property getting methods
    std::string GetPath() const { return path.name; };

    virtual const std::string TypeName() const;
private:
    PathName path;
};


#endif
