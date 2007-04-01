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
// ****************************************************************************
MakePlotRPC::MakePlotRPC() 
    : BlockingRPC("ad*", &networkID)
{
    atts = NULL;
    preparePlotRPC.SetMakePlotRPC(this);
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
// ****************************************************************************
void
MakePlotRPC::SelectAll()
{
    Select(1, (void*)&dataExtents);
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
// ****************************************************************************
int
MakePlotRPC::operator()(const string &n, const AttributeSubject *a,
                        const std::vector<double> &extents)
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
