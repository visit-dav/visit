/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include "NamedSelectionRPC.h"
#include <DebugStream.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Constructor: NamedSelectionRPC::NamedSelectionRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
//  Modifications:
//    Brad Whitlock, Tue Dec 14 12:05:51 PST 2010
//    I added selection properties and summary return value.
//
//    Brad Whitlock, Mon Aug 22 10:15:03 PDT 2011
//    I removed items from the type string and added intializers.
//
// ****************************************************************************

NamedSelectionRPC::NamedSelectionRPC() : NonBlockingRPC("iisba", &summary),
    selOperation(NamedSelectionRPC::NS_CREATE), plotId(-1), selName(), 
    properties(), allowCache(false), summary()
{
}

// ****************************************************************************
//  Destructor: NamedSelectionRPC::~NamedSelectionRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

NamedSelectionRPC::~NamedSelectionRPC()
{
}

// ****************************************************************************
// Method: CreateNamedSelection
//
// Purpose: 
//   Invocation method for apply.
//
// Arguments:
//   id     : The network id of the plot that creates our selection.
//   p      : The selection properties.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 12:09:51 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Sep  7 14:24:21 PDT 2011
//   Added allowCache.
//
// ****************************************************************************

const SelectionSummary &
NamedSelectionRPC::CreateNamedSelection(int id, const SelectionProperties &p)
{
    SetPlotID(id);
    SetSelectionName(p.GetName());
    SetSelectionProperties(p);
    SetNamedSelectionOperation(NS_CREATE);
    SetAllowCache(false);

    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(ImproperUseException, GetReply()->Message());
    }

    return summary;
}

// ****************************************************************************
// Method: NamedSelectionRPC::UpdateNamedSelection
//
// Purpose: 
//   Update the specified named selection.
//
// Arguments:
//   id     : The network id of the plot that creates our selection.
//   p      : The selection properties.
//   cache  : Whether cached intermediate selection data can be considered.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 14:25:05 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

const SelectionSummary &
NamedSelectionRPC::UpdateNamedSelection(int id, const SelectionProperties &p, bool cache)
{
    SetPlotID(id);
    SetSelectionName(p.GetName());
    SetSelectionProperties(p);
    SetNamedSelectionOperation(NS_UPDATE);
    SetAllowCache(cache);

    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(ImproperUseException, GetReply()->Message());
    }

    return summary;
}

// ****************************************************************************
// Method: DeleteNamedSelection
//
// Purpose: 
//   Invocation method for delete selection.
//
// Arguments:
//   selName : The name of the selection to delete.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 12:09:51 PST 2010
//
// ****************************************************************************

void
NamedSelectionRPC::DeleteNamedSelection(const std::string &selName)
{
    SetSelectionName(selName);
    SetNamedSelectionOperation(NS_DELETE);

    Execute();
}

// ****************************************************************************
// Method: LoadNamedSelection
//
// Purpose: 
//   Invocation method for load selection.
//
// Arguments:
//   selName : The name of the selection to load.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 12:09:51 PST 2010
//
// ****************************************************************************

void
NamedSelectionRPC::LoadNamedSelection(const std::string &selName)
{
    SetSelectionName(selName);
    SetNamedSelectionOperation(NS_LOAD);

    Execute();
}

// ****************************************************************************
// Method: SaveNamedSelection
//
// Purpose: 
//   Invocation method for save selection.
//
// Arguments:
//   selName : The name of the selection to save.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 12:09:51 PST 2010
//
// ****************************************************************************

void
NamedSelectionRPC::SaveNamedSelection(const std::string &selName)
{
    SetSelectionName(selName);
    SetNamedSelectionOperation(NS_SAVE);

    Execute();
}

// ****************************************************************************
//  Method: NamedSelectionRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
//  Modifications:
//    Brad Whitlock, Tue Dec 14 12:05:07 PST 2010
//    I added selection properties.
//
//    Brad Whitlock, Mon Aug 22 10:18:58 PDT 2011
//    I removed plotIDs.
//
//    Brad Whitlock, Wed Sep  7 14:26:33 PDT 2011
//    I added allowCache.
//
// ****************************************************************************

void
NamedSelectionRPC::SelectAll()
{
    Select(0, (void*)&selOperation);
    Select(1, (void*)&plotId);
    Select(2, (void*)&selName);
    Select(3, (void*)&allowCache);
    Select(4, (void*)&properties);
}

// ****************************************************************************
//  Method: NamedSelectionRPC::SetNamedSelectionOperation
//
//  Purpose: 
//    This sets the selection type.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetNamedSelectionOperation(NamedSelectionOperation t)
{
    selOperation = t;
    Select(0, (void*)&selOperation);
}

// ****************************************************************************
//  Method: NamedSelectionRPC::SetPlotID
//
//  Purpose: 
//    This sets the plot (network) ID.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetPlotID(int id)
{
    plotId = id;
    Select(1, (void*)&plotId);
}

// ****************************************************************************
//  Method: NamedSelectionRPC::SetSelectionName
//
//  Purpose: 
//    This sets the selection name.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2009
//
// ****************************************************************************

void
NamedSelectionRPC::SetSelectionName(const std::string &n)
{
    selName = n;
    Select(2, (void*)&selName);
}

// ****************************************************************************
// Method: NamedSelectionRPC::SetAllowCache
//
// Purpose: 
//   Set whether caching of intermediate selection results is allowed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 12:04:33 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
NamedSelectionRPC::SetAllowCache(bool c)
{
    allowCache = c;
    Select(3, (void*)&allowCache);
}

// ****************************************************************************
// Method: NamedSelectionRPC::SetSelectionProperties
//
// Purpose: 
//   Set the selection properties.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 12:04:33 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
NamedSelectionRPC::SetSelectionProperties(const SelectionProperties &p)
{
    properties = p;
    Select(4, (void*)&properties);
}
