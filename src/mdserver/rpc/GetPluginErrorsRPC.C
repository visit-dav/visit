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

#include <GetPluginErrorsRPC.h>
#include <DebugStream.h>
#include <string>

using std::string;

// *******************************************************************
// Method: GetPluginErrorsRPC::GetPluginErrorsRPC
//
// Purpose: 
//   Constructor for the GetPluginErrorsRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//
// *******************************************************************

GetPluginErrorsRPC::GetPluginErrorsRPC() : BlockingRPC("", &errors), errors()
{
}

// *******************************************************************
// Method: GetPluginErrorsRPC::~GetPluginErrorsRPC
//
// Purpose: 
//   Destructor for the GetPluginErrorsRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//
// *******************************************************************

GetPluginErrorsRPC::~GetPluginErrorsRPC()
{
}

// *******************************************************************
// Method: GetPluginErrorsRPC::operator()
//
// Purpose: 
//   Executes the RPC and returns the plugin errors.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//
// *******************************************************************

string
GetPluginErrorsRPC::operator()()
{
    debug3 << "Executing GetPluginErrorsRPC RPC\n";

    Execute();
    return errors.errorString;
}

// *******************************************************************
// Method: GetPluginErrorsRPC::SelectAll
//
// Purpose: 
//   Selects all the attributes that comprise the RPC's parameter list.
//
// Notes:
//   This RPC has no parameters so the no attributes are selected.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//
// *******************************************************************

void
GetPluginErrorsRPC::SelectAll()
{
    // no data sent
}

// *******************************************************************
// Method: GetPluginErrorsRPC::PluginErrors::PluginErrors
//
// Purpose: 
//   Constructor for the GetPluginErrorsRPC::PluginErrors class.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// *******************************************************************

GetPluginErrorsRPC::PluginErrors::PluginErrors() : AttributeSubject("s"),
    errorString()
{
}

// *******************************************************************
// Method: GetPluginErrorsRPC::PluginErrors::~PluginErrors
//
// Purpose: 
//   Destructor for the GetPluginErrorsRPC::PluginErrors class.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// *******************************************************************

GetPluginErrorsRPC::PluginErrors::~PluginErrors()
{
}

// *******************************************************************
// Method: GetPluginErrorsRPC::PluginErrors::SelectAll
//
// Purpose: 
//   Selects all the attributes in the object.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// *******************************************************************

void
GetPluginErrorsRPC::PluginErrors::SelectAll()
{
    Select(0, (void *)&errorString);
}
