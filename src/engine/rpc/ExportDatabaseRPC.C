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

#include "ExportDatabaseRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: ExportDatabaseRPC::ExportDatabaseRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

ExportDatabaseRPC::ExportDatabaseRPC() : BlockingRPC("ia")
{
}

// ****************************************************************************
//  Destructor: ExportDatabaseRPC::~ExportDatabaseRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

ExportDatabaseRPC::~ExportDatabaseRPC()
{
}

// ****************************************************************************
//  Method: ExportDatabaseRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//    origData  the type of input 
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
//  Modifications:
// ****************************************************************************

void
ExportDatabaseRPC::operator()(const int id_, const ExportDBAttributes *atts)
{
    SetID(id_);
    SetExportDBAtts(atts);

    Execute();
}

// ****************************************************************************
//  Method: ExportDatabaseRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

void
ExportDatabaseRPC::SelectAll()
{
    Select(0, (void*)&id);
    Select(1, (void*)&exportDBAtts);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::SetID
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

void
ExportDatabaseRPC::SetID(const int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005
//
// ****************************************************************************

int
ExportDatabaseRPC::GetID() const
{
    return id;
}

// ****************************************************************************
//  Method: ExportDatabaseRPC::SetExportDBAtts 
//
//  Purpose:
//    This sets the exportDB atts.
//
//  Arguments:
//    atts      the exportDB atts
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005 
//
// ****************************************************************************
 
void
ExportDatabaseRPC::SetExportDBAtts(const ExportDBAttributes *atts)
{
    exportDBAtts = *atts;
    Select(1, (void*)&exportDBAtts);
}


// ****************************************************************************
//  Method: ExportDatabaseRPC::GetExportDBAtts 
//
//  Purpose:
//    This returns the exportDB atts.
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2005 
//
// ****************************************************************************
 
ExportDBAttributes *
ExportDatabaseRPC::GetExportDBAtts()
{
    return &exportDBAtts;
}
