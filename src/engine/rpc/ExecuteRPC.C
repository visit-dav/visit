#include <ExecuteRPC.h>
#include <string.h>
#include <DebugStream.h>

// ****************************************************************************
//  Constructor: ExecuteRPC::ExecuteRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//     Mark C. Miller, 08Apr03, added respond with null data boolean
// ****************************************************************************

ExecuteRPC::ExecuteRPC() : NonBlockingRPC("b")
{
}

// ****************************************************************************
//  Destructor: ExecuteRPC::~ExecuteRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
// ****************************************************************************

ExecuteRPC::~ExecuteRPC()
{
}

// ****************************************************************************
//  Method: ExecuteRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:41:39 PST 2000
//    Made output go to log file instead of cout or cerr.
//
//    Mark C. Miller, 08Apr03, added respond with null data object boolean
//
// ****************************************************************************

void
ExecuteRPC::operator()(bool respond)
{
    debug3 << "Executing ExecuteRPC" << endl;

    SetRespondWithNull(respond);

    Execute();
}


// ****************************************************************************
//  Method: ExecuteRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   September 7, 2000
//
//  Modifications:
//     Mark C. Miller, 08Apr03, Added respondWithNullDataObject boolean
//
// ****************************************************************************

void
ExecuteRPC::SelectAll()
{
    Select(0, (void*)&respondWithNullDataObject);
}

// ****************************************************************************
//  Method: ExecuteRPC::SetRespondWithNull
//
//  Purpose: 
//    Sets flag to indicate if execute should respond with null data or not 
//
//  Programmer: Mark C. Miller 
//  Creation:   08Apr03 
// ****************************************************************************

void
ExecuteRPC::SetRespondWithNull(bool respond)
{
   respondWithNullDataObject = respond;
   Select(0, (void*)&respondWithNullDataObject);
}

// ****************************************************************************
//  Method: ExecuteRPC::GetRespondWithNull
//
//  Purpose: 
//    Gets flag indicating if execute should respond with null data or not 
//
//  Programmer: Mark C. Miller 
//  Creation:   08Apr03 
// ****************************************************************************
bool
ExecuteRPC::GetRespondWithNull() const
{
   return respondWithNullDataObject;
}
