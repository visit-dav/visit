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

#include "ConstructDDFRPC.h"
#include <DebugStream.h>


// ****************************************************************************
//  Constructor: ConstructDDFRPC::ConstructDDFRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

ConstructDDFRPC::ConstructDDFRPC() : BlockingRPC("ia")
{
}

// ****************************************************************************
//  Destructor: ConstructDDFRPC::~ConstructDDFRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

ConstructDDFRPC::~ConstructDDFRPC()
{
}

// ****************************************************************************
//  Method: ConstructDDFRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    id        the network id
//    origData  the type of input 
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
//  Modifications:
// ****************************************************************************

void
ConstructDDFRPC::operator()(const int id_, const ConstructDDFAttributes *atts)
{
    SetID(id_);
    SetConstructDDFAtts(atts);

    Execute();
}

// ****************************************************************************
//  Method: ConstructDDFRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

void
ConstructDDFRPC::SelectAll()
{
    Select(0, (void*)&id);
    Select(1, (void*)&constructDDFAtts);
}


// ****************************************************************************
//  Method: ConstructDDFRPC::SetID
//
//  Purpose: 
//    This sets the id parameter.
//
//  Arguments:
//    id_         the id
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

void
ConstructDDFRPC::SetID(const int id_)
{
    id = id_;
    Select(0, (void*)&id);
}


// ****************************************************************************
//  Method: ConstructDDFRPC::GetID
//
//  Purpose: 
//    This returns network id.
//
//  Arguments:
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

int
ConstructDDFRPC::GetID() const
{
    return id;
}

// ****************************************************************************
//  Method: ConstructDDFRPC::SetConstructDDFAtts 
//
//  Purpose:
//    This sets the constructDDF atts.
//
//  Arguments:
//    atts      the constructDDF atts
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006 
//
// ****************************************************************************
 
void
ConstructDDFRPC::SetConstructDDFAtts(const ConstructDDFAttributes *atts)
{
    constructDDFAtts = *atts;
    Select(1, (void*)&constructDDFAtts);
}


// ****************************************************************************
//  Method: ConstructDDFRPC::GetConstructDDFAtts 
//
//  Purpose:
//    This returns the constructDDF atts.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006 
//
// ****************************************************************************
 
ConstructDDFAttributes *
ConstructDDFRPC::GetConstructDDFAtts()
{
    return &constructDDFAtts;
}
