#include "ApplyOperatorRPC.h"

#include <Xfer.h>

#include <string>
using std::string;

// ----------------------------------------------------------------------------
//                           class PrepareOperatorRPC
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Method:  PrepareOperatorRPC::PrepareOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PrepareOperatorRPC::PrepareOperatorRPC() 
    : BlockingRPC("s")
{
    id = "unknown";
    applyOperatorRPC = NULL;
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::~PrepareOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PrepareOperatorRPC::~PrepareOperatorRPC() 
{
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::SetApplyOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PrepareOperatorRPC::SetApplyOperatorRPC(ApplyOperatorRPC *rpc)
{
    applyOperatorRPC = rpc;
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::operator
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PrepareOperatorRPC::operator()(const std::string &n)
{
    id = n;
    Select(0, (void*)&id);
    Execute();
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
PrepareOperatorRPC::SelectAll()
{
    Select(0, (void*)&id);
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
string
PrepareOperatorRPC::GetID()
{
    return id;
}

// ****************************************************************************
//  Method:  PrepareOperatorRPC::GetApplyOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
ApplyOperatorRPC*
PrepareOperatorRPC::GetApplyOperatorRPC()
{
    return applyOperatorRPC;
}


// ----------------------------------------------------------------------------
//                           class ApplyOperatorRPC
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Method:      : BlockingRPC("a
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
ApplyOperatorRPC::ApplyOperatorRPC() 
    : BlockingRPC("a")
{
    atts = NULL;
    prepareOperatorRPC.SetApplyOperatorRPC(this);
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::~ApplyOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
ApplyOperatorRPC::~ApplyOperatorRPC()
{
    if (atts)
        delete atts;
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::SetAtts
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
ApplyOperatorRPC::SetAtts(AttributeSubject *a)
{
    if (atts)
        delete atts;
    atts = a;
    if (atts)
        Select(0, (void*)atts);
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::SetXfer
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
ApplyOperatorRPC::SetXfer(Xfer *x)
{
    VisItRPC::SetXfer(x);
    xfer->Add(&prepareOperatorRPC);
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::SelectAll
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
void
ApplyOperatorRPC::SelectAll()
{
    // nothing
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::GetID
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
std::string
ApplyOperatorRPC::GetID()
{
    return prepareOperatorRPC.GetID();
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::GetAtts
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
AttributeSubject*
ApplyOperatorRPC::GetAtts()
{
    return atts;
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::GetPrepareOperatorRPC
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************
PrepareOperatorRPC&
ApplyOperatorRPC::GetPrepareOperatorRPC()
{
    return prepareOperatorRPC;
}

// ****************************************************************************
//  Method:  ApplyOperatorRPC::operator
//
//  Purpose:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 21 11:09:54 PST 2002
//    Added propogation of error from preparatory RPC.
//
// ****************************************************************************
void
ApplyOperatorRPC::operator()(const string &n, const AttributeSubject *a)
{
    prepareOperatorRPC(n);
    if (prepareOperatorRPC.GetStatus() == VisItRPC::error)
    {
        reply.SetStatus(VisItRPC::error);
        reply.SetMessage(prepareOperatorRPC.GetMessage());
        reply.SetExceptionType(prepareOperatorRPC.GetExceptionType());
        return;
    }

    if (a)
        Select(0, (void*)a);

    Execute();
}
