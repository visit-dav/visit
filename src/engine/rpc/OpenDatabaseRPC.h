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

#ifndef OPEN_DATABASE_RPC_H
#define OPEN_DATABASE_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: OpenDatabaseRPC
//
// Purpose:
//   Tells the engine to open a database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 13:51:36 PST 2002
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 11:13:32 PST 2004
//   Added a format as an argument.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added args for createMeshQualityExpressions and 
//   createTimeDerivativeExpresisons. 
//
// ****************************************************************************

class ENGINE_RPC_API OpenDatabaseRPC : public NonBlockingRPC
{
public:
    OpenDatabaseRPC();
    virtual ~OpenDatabaseRPC();

    virtual const std::string TypeName() const { return "OpenDatabaseRPC"; }

    void operator()(const std::string &, const std::string &, int, 
                    bool, bool);

    virtual void SelectAll();

    const std::string &GetFileFormat() const { return fileFormat; };
    const std::string &GetDatabaseName() const { return databaseName; };
    int                GetTime() const { return time; };
    bool               GetCreateMeshQualityExpressions() const 
                           { return createMeshQualityExpressions; };
    bool               GetCreateTimeDerivativeExpressions() const 
                           { return createTimeDerivativeExpressions; };
private:
    std::string fileFormat;
    std::string databaseName;
    int         time;
    bool        createMeshQualityExpressions;
    bool        createTimeDerivativeExpressions;
};

#endif
