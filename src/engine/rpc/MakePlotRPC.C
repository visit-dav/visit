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

#include "MakePlotRPC.h"

#include <Xfer.h>

#include <string>
using std::string;

// ----------------------------------------------------------------------------
//                         class PreparePlotRPC
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Method:  PreparePlotRPC::PreparePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PreparePlotRPC::PreparePlotRPC() 
    : BlockingRPC("s")
{
    id = "unknown";
    makePlotRPC = NULL;
}

// ****************************************************************************
//  Method:  PreparePlotRPC::~PreparePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PreparePlotRPC::~PreparePlotRPC() 
{
}

// ****************************************************************************
//  Method:  PreparePlotRPC::SetMakePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PreparePlotRPC::SetMakePlotRPC(MakePlotRPC *rpc)
{
    makePlotRPC = rpc;
}

// ****************************************************************************
//  Method:  PreparePlotRPC::operator
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PreparePlotRPC::operator()(const std::string &n)
{
    id = n;
    Select(0, (void*)&id);
    Execute();
}

// ****************************************************************************
//  Method:  PreparePlotRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PreparePlotRPC::SelectAll()
{
    Select(0, (void*)&id);
}

// ****************************************************************************
//  Method:  PreparePlotRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
string
PreparePlotRPC::GetID()
{
    return id;
}

// ****************************************************************************
//  Method:  PreparePlotRPC::GetMakePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
MakePlotRPC*
PreparePlotRPC::GetMakePlotRPC()
{
    return makePlotRPC;
}


// ----------------------------------------------------------------------------
//                          class MakePlotRPC
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:  MakePlotRPC::MakePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Nov  9 10:16:51 PST 2001
//    Added a NetworkID as a return type of the rpc.
//
//    Jeremy Meredith, Fri Mar 19 15:00:21 PST 2004
//    Modified the rpc to pass the data extents.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added windowID
//
// ****************************************************************************
MakePlotRPC::MakePlotRPC() 
    : BlockingRPC("ad*i", &networkID)
{
    atts = NULL;
    preparePlotRPC.SetMakePlotRPC(this);
    windowID = -1;
}

// ****************************************************************************
//  Method:  MakePlotRPC::~MakePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************

MakePlotRPC::~MakePlotRPC()
{
    if (atts)
        delete atts;
}

// ****************************************************************************
//  Method:  MakePlotRPC::SetAtts
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
MakePlotRPC::SetAtts(AttributeSubject *a)
{
    if (atts)
        delete atts;
    atts = a;
    Select(0, (void*)atts);
}

// ****************************************************************************
//  Method:  MakePlotRPC::SetDataExtents
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 19, 2004
//
// ****************************************************************************
void
MakePlotRPC::SetDataExtents(const std::vector<double> &extents)
{
    dataExtents = extents;
    Select(1, (void*)&dataExtents);
}

// ****************************************************************************
//  Method:  MakePlotRPC::SetWindowID
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 15, 2004 
//
// ****************************************************************************
void
MakePlotRPC::SetWindowID(int id)
{
    windowID = id;
    Select(2, (void*)&windowID);
}

// ****************************************************************************
//  Method:  MakePlotRPC::SetXfer
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
MakePlotRPC::SetXfer(Xfer *x)
{
    VisItRPC::SetXfer(x);
    xfer->Add(&preparePlotRPC);
}

// ****************************************************************************
//  Method:  MakePlotRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 19 15:00:21 PST 2004
//    Modified the rpc to pass the data extents.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added windowID
//
// ****************************************************************************
void
MakePlotRPC::SelectAll()
{
    Select(1, (void*)&dataExtents);
    Select(2, (void*)&windowID);
}

// ****************************************************************************
//  Method:  MakePlotRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
std::string
MakePlotRPC::GetID()
{
    return preparePlotRPC.GetID();
}

// ****************************************************************************
//  Method:  MakePlotRPC::GetAtts
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
AttributeSubject*
MakePlotRPC::GetAtts()
{
    return atts;
}

// ****************************************************************************
//  Method:  MakePlotRPC::GetDataExtents
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 19, 2004
//
// ****************************************************************************
const std::vector<double> &
MakePlotRPC::GetDataExtents() const
{
    return dataExtents;
}

// ****************************************************************************
//  Method:  MakePlotRPC::GetWindowID
//
//  Programmer:  Mark C. Miller 
//  Creation:    December 15, 2004 
//
// ****************************************************************************
int
MakePlotRPC::GetWindowID() const
{
    return windowID;
}

// ****************************************************************************
//  Method:  MakePlotRPC::GetPreparePlotRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PreparePlotRPC&
MakePlotRPC::GetPreparePlotRPC()
{
    return preparePlotRPC;
}

// ****************************************************************************
//  Method:  MakePlotRPC::operator()
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Nov  9 10:16:51 PST 2001
//    Return the networkID for this RPC.
//
//    Jeremy Meredith, Thu Nov 21 11:09:54 PST 2002
//    Added propogation of error from preparatory RPC.
//
//    Jeremy Meredith, Fri Mar 19 15:00:21 PST 2004
//    Modified the rpc to pass the data extents.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added windowID
//
// ****************************************************************************
int
MakePlotRPC::operator()(const string &n, const AttributeSubject *a,
                        const std::vector<double> &extents, int winID)
{
    preparePlotRPC(n);
    if (preparePlotRPC.GetStatus() == VisItRPC::error)
    {
        reply.SetStatus(VisItRPC::error);
        reply.SetMessage(preparePlotRPC.Message());
        reply.SetExceptionType(preparePlotRPC.GetExceptionType());
        return -1;
    }

    if (a)
        Select(0, (void*)a);
    Select(1, (void*)&extents);
    Select(2, (void*)&winID);

    Execute();

    return networkID.id;
}


// ****************************************************************************
//  Constructor:  MakePlotRPC::NetworkID::NetworkID
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
// ****************************************************************************
MakePlotRPC::NetworkID::NetworkID() : AttributeSubject("i")
{
    id = -1;
}

// ****************************************************************************
//  Constructor:  MakePlotRPC::NetworkID::NetworkID
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
// ****************************************************************************
MakePlotRPC::NetworkID::NetworkID(int id_) : AttributeSubject("i")
{
    id = id_;
}

// ****************************************************************************
//  Destructor:  MakePlotRPC::NetworkID::~NetworkID
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
// ****************************************************************************
MakePlotRPC::NetworkID::~NetworkID()
{
}

// ****************************************************************************
//  Method:  MakePlotRPC::NetworkID::SelectAll
//
//  Purpose:
//    Select all attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
// ****************************************************************************
void
MakePlotRPC::NetworkID::SelectAll()
{
    Select(0, (void *)&id);
}
