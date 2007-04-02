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

#include "UpdatePlotAttsRPC.h"

#include <Xfer.h>

#include <string>
using std::string;

// ----------------------------------------------------------------------------
//                         class PrepareUpdatePlotAttsRPC
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::PrepareUpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
PrepareUpdatePlotAttsRPC::PrepareUpdatePlotAttsRPC() 
    : BlockingRPC("s")
{
    id = "unknown";
    updatePlotAttsRPC = NULL;
}

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::~PrepareUpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
PrepareUpdatePlotAttsRPC::~PrepareUpdatePlotAttsRPC() 
{
}

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::SetUpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
PrepareUpdatePlotAttsRPC::SetUpdatePlotAttsRPC(UpdatePlotAttsRPC *rpc)
{
    updatePlotAttsRPC = rpc;
}

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::operator
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
PrepareUpdatePlotAttsRPC::operator()(const std::string &n)
{
    id = n;
    Select(0, (void*)&id);
    Execute();
}

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
PrepareUpdatePlotAttsRPC::SelectAll()
{
    Select(0, (void*)&id);
}

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
string
PrepareUpdatePlotAttsRPC::GetID()
{
    return id;
}

// ****************************************************************************
//  Method:  PrepareUpdatePlotAttsRPC::GetUpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
UpdatePlotAttsRPC*
PrepareUpdatePlotAttsRPC::GetUpdatePlotAttsRPC()
{
    return updatePlotAttsRPC;
}


// ----------------------------------------------------------------------------
//                          class UpdatePlotAttsRPC
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::UpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
UpdatePlotAttsRPC::UpdatePlotAttsRPC() 
    : BlockingRPC("ia")
{
    index = -1;
    atts = NULL;
    prepareUpdatePlotAttsRPC.SetUpdatePlotAttsRPC(this);
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::~UpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************

UpdatePlotAttsRPC::~UpdatePlotAttsRPC()
{
    if (atts)
        delete atts;
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::SetAtts
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
UpdatePlotAttsRPC::SetAtts(AttributeSubject *a)
{
//    if (atts)
//        delete atts;
    atts = a;
    Select(1, (void*)atts);
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::SetXfer
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
UpdatePlotAttsRPC::SetXfer(Xfer *x)
{
    VisItRPC::SetXfer(x);
    xfer->Add(&prepareUpdatePlotAttsRPC);
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
UpdatePlotAttsRPC::SelectAll()
{
    Select(0, (void*)&index);
    // atts will be done when they are set.
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
std::string
UpdatePlotAttsRPC::GetID()
{
    return prepareUpdatePlotAttsRPC.GetID();
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::GetAtts
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
const AttributeSubject*
UpdatePlotAttsRPC::GetAtts()
{
    return atts;
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::GetPrepareUpdatePlotAttsRPC
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
PrepareUpdatePlotAttsRPC&
UpdatePlotAttsRPC::GetPrepareUpdatePlotAttsRPC()
{
    return prepareUpdatePlotAttsRPC;
}

// ****************************************************************************
//  Method:  UpdatePlotAttsRPC::operator()
//
//  Purpose:
//    
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
void
UpdatePlotAttsRPC::operator()(const string &n, int ind, 
                              const AttributeSubject *a)
{
    prepareUpdatePlotAttsRPC(n);
    Select(0, (void *)&ind);
    if (a)
       Select(1, (void *)a);

    Execute();
}


