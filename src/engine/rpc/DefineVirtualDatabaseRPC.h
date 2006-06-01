/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef DEFINE_VIRTUAL_DATABASE_RPC_H
#define DEFINE_VIRTUAL_DATABASE_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>
#include <vectortypes.h>

// ****************************************************************************
// Class: DefineVirtualDatabaseRPC
//
// Purpose:
//   Tells the engine to define a virtual database with the specified files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:45:41 PST 2003
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

class ENGINE_RPC_API DefineVirtualDatabaseRPC : public NonBlockingRPC
{
public:
    DefineVirtualDatabaseRPC();
    virtual ~DefineVirtualDatabaseRPC();

    void operator()(const std::string &fileFormat,
                    const std::string &wholeName,
                    const std::string &pathToTimesteps,
                    const stringVector &timeSteps, int);

    virtual void SelectAll();

    const std::string  &GetFileFormat() const { return fileFormat; };
    const std::string  &GetDatabaseName() const { return databaseName; };
    const std::string  &GetDatabasePath() const { return databasePath; };
    const stringVector &GetDatabaseFiles() const { return databaseFiles; };
    int                GetTime() const { return time; };
private:
    std::string  fileFormat;
    std::string  databaseName;
    std::string  databasePath;
    stringVector databaseFiles;
    int          time;
};

#endif
