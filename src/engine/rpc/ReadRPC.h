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

#ifndef READ_RPC_H
#define READ_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>
#include <CompactSILRestrictionAttributes.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>

// ****************************************************************************
//  Class: ReadRPC
//
//  Purpose:
//    Initiate an engine computation by reading a database.
//    Sends other attributes needed for computation as well.
//
//  Modifications:
//    Hank Childs, Fri Dec 14 17:30:17 PST 2001
//    Modified all methods to operate on a "compact" sil restriction instead
//    of a normal sil restriction.
//
//    Jeremy Meredith, Thu Oct 24 11:18:20 PDT 2002
//    Added material interface reconstruction attributes.
//
//    Hank Childs, Tue Mar  9 14:27:31 PST 2004
//    Added file format type.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes 
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
// ****************************************************************************

class ENGINE_RPC_API ReadRPC : public BlockingRPC
{
public:
    ReadRPC();
    virtual ~ReadRPC();

    virtual const std::string TypeName() const { return "ReadRPC"; }

    // Invokation method
    void operator()(const std::string &format, const std::string& filename,
                    const std::string &var, int time,
                    const CompactSILRestrictionAttributes &,
                    const MaterialAttributes &,
                    const MeshManagementAttributes &,
		    bool);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetFormat(const std::string&);
    void SetFile(const std::string&);
    void SetVar(const std::string&);
    void SetTime(int);
    void SetCSRAttributes(const CompactSILRestrictionAttributes &);
    void SetMaterialAttributes(const MaterialAttributes &);
    void SetMeshManagementAttributes(const MeshManagementAttributes &);
    void SetTreatAllDBsAsTimeVarying(bool);

    // Property getting methods
    std::string GetVar()  const;
    std::string GetFile() const;
    std::string GetFormat() const;
    int         GetTime() const;
    const CompactSILRestrictionAttributes &GetCSRAttributes() const;
    const MaterialAttributes &GetMaterialAttributes() const;
    const MeshManagementAttributes &GetMeshManagementAttributes() const;
    bool        GetTreatAllDBsAsTimeVarying() const;

private:
    std::string file;
    std::string format;
    std::string var;
    int         time;
    CompactSILRestrictionAttributes silr_atts;
    MaterialAttributes materialAtts;
    MeshManagementAttributes meshManagementAtts;
    bool treatAllDBsAsTimeVarying;
};

#endif
