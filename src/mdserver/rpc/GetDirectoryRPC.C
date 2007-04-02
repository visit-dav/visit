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

#include <GetDirectoryRPC.h>
#include <DebugStream.h>
#include <string>

using std::string;

// *******************************************************************
// Method: GetDirectoryRPC::GetDirectoryRPC
//
// Purpose: 
//   Constructor for the GetDirectoryRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 10:52:47 PDT 2000
//   I changed the format string to be empty.
//
// *******************************************************************

GetDirectoryRPC::GetDirectoryRPC() : BlockingRPC("", &cwd), cwd()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::~GetDirectoryRPC
//
// Purpose: 
//   Destructor for the GetDirectoryRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//
// *******************************************************************

GetDirectoryRPC::~GetDirectoryRPC()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::operator()
//
// Purpose: 
//   Executes the RPC and returns the current directory.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//
//    Jeremy Meredith, Fri Nov 17 16:30:46 PST 2000
//    Made output go to log file instead of cout or cerr.
//
// *******************************************************************

string
GetDirectoryRPC::operator()()
{
    debug3 << "Executing GetDirectoryRPC RPC\n";

    Execute();
    return cwd.name;
}

// *******************************************************************
// Method: GetDirectoryRPC::SelectAll
//
// Purpose: 
//   Selects all the attributes that comprise the RPC's parameter list.
//
// Notes:
//   This RPC has no parameters so the no attributes are selected.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//
// *******************************************************************

void
GetDirectoryRPC::SelectAll()
{
    // no data sent
}

// *******************************************************************
// Method: GetDirectoryRPC::DirectoryName::DirectoryName
//
// Purpose: 
//   Constructor for the GetDirectoryRPC::DirectoryName class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 11:47:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

GetDirectoryRPC::DirectoryName::DirectoryName() : AttributeSubject("s"),
    name()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::DirectoryName::~DirectoryName
//
// Purpose: 
//   Destructor for the GetDirectoryRPC::DirectoryName class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 11:47:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

GetDirectoryRPC::DirectoryName::~DirectoryName()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::DirectoryName::SelectAll
//
// Purpose: 
//   Selects all the attributes in the object.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 11:47:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

void
GetDirectoryRPC::DirectoryName::SelectAll()
{
    Select(0, (void *)&name);
}
