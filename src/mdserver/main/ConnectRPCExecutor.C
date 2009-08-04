/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <stdio.h>               // for sprintf
#include <ConnectRPCExecutor.h>
#include <ConnectRPC.h>
#include <MDServerApplication.h>
#include <VisItException.h>

#include <cstring>

// ****************************************************************************
// Method: ConnectRPCExecutor::ConnectRPCExecutor
//
// Purpose: 
//   Constructor for the ConnectRPCExecutor class.
//
// Arguments:
//   s : A pointer to the RPC that will call this RPC executor.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:53:17 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConnectRPCExecutor::ConnectRPCExecutor(Subject *s) : Observer(s)
{
}

// ****************************************************************************
// Method: ConnectRPCExecutor::~ConnectRPCExecutor
//
// Purpose: 
//   Destructor for the ConnectRPCExecutor class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:53:56 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConnectRPCExecutor::~ConnectRPCExecutor()
{
}

// ****************************************************************************
// Method: ConnectRPCExecutor::StrDup
//
// Purpose: 
//   Duplicates a string.
//
// Arguments:
//   str : The string to duplicate.
//
// Returns:   a copy of the passed-in string. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:54:20 PST 2000
//
// Modifications:
//   
// ****************************************************************************

char *
ConnectRPCExecutor::StrDup(const char *str)
{
    if(str == NULL)
        return NULL;

    int len = strlen(str);
    char *retval = new char[len + 1];
    for(int i = 0; i < len; ++i)
        retval[i] = str[i];
    retval[len] = '\0';
    
    return retval;
}

// ****************************************************************************
// Method: ConnectRPCExecutor::Update
//
// Purpose: 
//   Tells the MDServerApplication to connect to another process.
//
// Arguments:
//   s : A pointer to the ConnectRPC that called this method.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:55:02 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 16:06:06 PST 2002
//   I added support for a security key.
//
//   Brad Whitlock, Mon Jun 16 13:51:43 PST 2003
//   I made the code more generic so different kinds of connections can be
//   used to talk to the client.
//
// ****************************************************************************

void
ConnectRPCExecutor::Update(Subject *s)
{
    ConnectRPC *rpc = (ConnectRPC *)s;

    // Create a fake argv that we can use to make a new MDServerConnection.
    const stringVector &args = rpc->GetArguments();
    int i, argc = args.size();
    int argc2 = argc;
    char **argv = new char *[argc + 1];
    char **argv2 = new char *[argc + 1];
    for(i = 0; i < argc; ++i)
    {
        argv[i] = StrDup(args[i].c_str());
        argv2[i] = argv[i];
    }
    argv[argc]  = NULL;
    argv2[argc2] = NULL;

    TRY
    {
        // Make the MDServerApplication connect to another program.
        MDServerApplication::Instance()->AddConnection(&argc2, &argv2);
    }
    CATCH(VisItException)
    {
        ; // The connection probably could not be created.
    }
    ENDTRY

    // Delete the temporary arguments.
    for(i = 0; i < argc; ++i)
        delete [] argv[i];
    delete [] argv;
    delete [] argv2;
}
