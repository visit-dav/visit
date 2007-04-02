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

#include <CloseDatabaseRPC.h>

// ****************************************************************************
// Method: CloseDatabaseRPC::CloseDatabaseRPC
//
// Purpose: 
//   This is the constructor for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:25:02 PDT 2005
//   Added database member.
//
// ****************************************************************************

CloseDatabaseRPC::CloseDatabaseRPC() : BlockingRPC(""), database()
{
}

// ****************************************************************************
// Method: CloseDatabaseRPC::~CloseDatabaseRPC
//
// Purpose: 
//   This is the destructor for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

CloseDatabaseRPC::~CloseDatabaseRPC()
{
}

// ****************************************************************************
// Method: CloseDatabaseRPC::operator()
//
// Purpose: 
//   This is the invokation method for the CloseDatabaseRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:45:45 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:27:36 PDT 2005
//   Added database argument,
//
// ****************************************************************************

void
CloseDatabaseRPC::operator()(const std::string &db)
{
    SetDatabase(db);
    Execute();
}

// ****************************************************************************
// Method: CloseDatabaseRPC::SelectAll
//
// Purpose: 
//   Selects all components of the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:58:42 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:28:17 PDT 2005
//   Added database.
//
// ****************************************************************************

void
CloseDatabaseRPC::SelectAll()
{
    Select(0, (void *)&database);
}

// ****************************************************************************
// Method: CloseDatabase::SetDatabase
//
// Purpose: 
//   Sets the value to use for database.
//
// Arguments:
//   db : The database to close.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 4 08:29:13 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
CloseDatabaseRPC::SetDatabase(const std::string &db)
{
    database = db;
    SelectAll();
}
