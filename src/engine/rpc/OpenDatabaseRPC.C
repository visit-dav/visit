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

#include <OpenDatabaseRPC.h>

// ****************************************************************************
// Method: OpenDatabaseRPC::OpenDatabaseRPC
//
// Purpose: 
//   Constructor for the OpenDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:05:36 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 24 11:14:57 PDT 2003
//   I made it inherit from NonBlockingRPC.
//
//   Hank Childs, Fri Mar  5 11:13:32 PST 2004
//   Added string for 'format'
//
// ****************************************************************************

OpenDatabaseRPC::OpenDatabaseRPC() : NonBlockingRPC("ssi"), databaseName("")
{
    time = 0;
}

// ****************************************************************************
// Method: OpenDatabaseRPC::~OpenDatabaseRPC
//
// Purpose: 
//   Destructor for the OpenDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:05:54 PST 2002
//
// Modifications:
//   
// ****************************************************************************

OpenDatabaseRPC::~OpenDatabaseRPC()
{
}

// ****************************************************************************
// Method: OpenDatabaseRPC::operator()
//
// Purpose: 
//   Executes the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:05:54 PST 2002
//
// Modifications:
//   
//   Hank Childs, Fri Mar  5 11:13:32 PST 2004
//   Added 'format'.
//
// ****************************************************************************

void
OpenDatabaseRPC::operator()(const std::string &f,
                            const std::string &dbName, int timestep)
{
    fileFormat = f;
    databaseName = dbName;
    time = timestep;
    SelectAll();
    Execute();
}

// ****************************************************************************
// Method: OpenDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all of the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 14:06:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
OpenDatabaseRPC::SelectAll()
{
    Select(0, (void*)&fileFormat);
    Select(1, (void*)&databaseName);
    Select(2, (void*)&time);
}

