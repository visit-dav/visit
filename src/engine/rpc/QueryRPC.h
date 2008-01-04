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

#ifndef QUERY_RPC_H 
#define QUERY_RPC_H 
#include <vector>
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <QueryAttributes.h>

// ****************************************************************************
//  Class:  QueryRPC
//
//  Purpose:
//    Implements an RPC to perform a query. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    September 6, 2002 
//
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:22:29 PDT 2003
//    Allow for queries to involve multiple networks.
//
// ****************************************************************************

class ENGINE_RPC_API QueryRPC : public NonBlockingRPC
{
public:
    QueryRPC();
    virtual ~QueryRPC() { };

    virtual const std::string TypeName() const { return "QueryRPC"; }

    // Invokation method
    void operator() (const std::vector<int> &netids, const QueryAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void    SetNetworkIds(const std::vector<int> &netIds)
        { networkIds = netIds; Select(0, (void *)&networkIds); };

    void SetQueryAtts(const QueryAttributes*);

    // Property getting methods
    const std::vector<int>    &GetNetworkIds() const { return networkIds; };

    // Property getting methods
    QueryAttributes *GetQueryAtts();

    // Property getting methods
    QueryAttributes GetReturnAtts() { return returnAtts; } ; 

    QueryAttributes returnAtts; 

private:
    std::vector<int>  networkIds;
    QueryAttributes   queryAtts; 
};

#endif
