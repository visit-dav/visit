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

#include <DefineVirtualDatabaseRPC.h>

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::DefineVirtualDatabaseRPC
//
// Purpose: 
//   Constructor for the DefineVirtualDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:48:46 PST 2003
//
// Modifications:
//
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

DefineVirtualDatabaseRPC::DefineVirtualDatabaseRPC() : NonBlockingRPC("ssss*i"),
    fileFormat(), databaseName(), databasePath(), databaseFiles()
{
    time = 0;
}

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::~DefineVirtualDatabaseRPC
//
// Purpose: 
//   Destructor for the DefineVirtualDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:48:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DefineVirtualDatabaseRPC::~DefineVirtualDatabaseRPC()
{
}

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::operator()
//
// Purpose: 
//   Executes the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:47:52 PST 2003
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

void
DefineVirtualDatabaseRPC::operator()(const std::string &fileFormatType,
    const std::string &wholeDBName, const std::string &pathToTimesteps, 
    const stringVector &dbFiles, int timestep)
{
    fileFormat = fileFormatType;
    databaseName = wholeDBName;
    databasePath = pathToTimesteps;
    databaseFiles = dbFiles;
    time = timestep;
    SelectAll();
    Execute();
}

// ****************************************************************************
// Method: DefineVirtualDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all of the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:47:42 PST 2003
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
// ****************************************************************************

void
DefineVirtualDatabaseRPC::SelectAll()
{
    Select(0, (void*)&fileFormat);
    Select(1, (void*)&databaseName);
    Select(2, (void*)&databasePath);
    Select(3, (void*)&databaseFiles);
    Select(4, (void*)&time);
}

