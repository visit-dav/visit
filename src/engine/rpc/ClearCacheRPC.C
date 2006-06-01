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

#include <ClearCacheRPC.h>

// ****************************************************************************
// Method: ClearCacheRPC::ClearCacheRPC
//
// Purpose: 
//   Constructor for the ClearCacheRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:58:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

ClearCacheRPC::ClearCacheRPC() : BlockingRPC("sb"), dbName("")
{
    clearAll = true;
}

// ****************************************************************************
// Method: ClearCacheRPC::~ClearCacheRPC
//
// Purpose: 
//   Destructor for the ClearCacheRPC class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:58:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

ClearCacheRPC::~ClearCacheRPC()
{
}

// ****************************************************************************
// Method: ClearCacheRPC::operator()
//
// Purpose: 
//   Invokation method for the ClearCacheRPC class. Clears the caches that 
//   relate to the specified database.
//
// Arguments:
//   filename : The name of the database for which we're clearing information.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:58:56 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ClearCacheRPC::operator()(const std::string &filename, bool clearAllCaches)
{
    dbName = filename;
    clearAll = clearAllCaches;
    Execute();
}

// ****************************************************************************
// Method: ClearCacheRPC::SelectAll
//
// Purpose: 
//   Selects all components in the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:59:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ClearCacheRPC::SelectAll()
{
    Select(0, (void*)&dbName);
    Select(1, (void*)&clearAll);
}
