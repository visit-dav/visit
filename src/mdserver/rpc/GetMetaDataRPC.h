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

#ifndef _GETMETADATA_RPC_H_
#define _GETMETADATA_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <vector>
#include <string>
#include <avtDatabaseMetaData.h>

// ****************************************************************************
// Class: GetMetaDataRPC
//
// Purpose:
//   This class encapsulates a call to get the metadata for a database
//   from a remote file system.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:18:10 PST 2003
//   I added an optional timestate argument.
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added forceReadAllCyclesAndTimes  
//
//   Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//   Added ability to force using a specific plugin when reading
//   the metadata from a file (if it causes the file to be opened).
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
// ****************************************************************************

class MDSERVER_RPC_API GetMetaDataRPC : public BlockingRPC
{
public:
    GetMetaDataRPC();
    virtual ~GetMetaDataRPC();

    virtual const std::string TypeName() const;

    // Invokation method
    const avtDatabaseMetaData *operator()(const std::string&, int timeState=0,
                                          bool forceReadAllCyclesAndTimes=false,
                                          const std::string &forcedFileType="",
					  bool treatAllDBsAsTimeVarying=false);

    // Property setting methods
    void SetFile(const std::string&);
    void SetTimeState(int ts);
    void SetForceReadAllCyclesAndTimes(bool force);
    void SetForcedFileType(const std::string&);
    void SetTreatAllDBsAsTimeVarying(bool set);


    // Property getting methods
    std::string GetFile() const;
    int GetTimeState() const;
    bool GetForceReadAllCyclesAndTimes() const;
    std::string GetForcedFileType() const;
    bool GetTreatAllDBsAsTimeVarying() const;

    // Property selection methods
    virtual void SelectAll();
private:
    avtDatabaseMetaData  metaData;
    std::string          file;
    int                  timeState;
    bool                 forceReadAllCyclesAndTimes;
    std::string          forcedFileType;
    bool                 treatAllDBsAsTimeVarying;
};


#endif
