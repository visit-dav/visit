#include <KeepAliveRPC.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: KeepAliveRPC::KeepAliveRPC
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:05:46 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

KeepAliveRPC::KeepAliveRPC() : BlockingRPC("i", NULL)
{
    value = 0;
}

// ****************************************************************************
// Method: KeepAliveRPC::~KeepAliveRPC
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:05:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

KeepAliveRPC::~KeepAliveRPC()
{
}

// ****************************************************************************
// Method: KeepAliveRPC::operator()
//
// Purpose: 
//   Invocation method for the RPC.
//
// Arguments:
//   v : The value to pass along.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:06:15 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
KeepAliveRPC::operator()()
{
    SetValue(value + 1);
    debug3 << "Executing keep alive RPC"
           << "\n\t value='" << value << "'"
           << endl;
    Execute();
}

// ****************************************************************************
// Method: KeepAliveRPC::SelectAll
//
// Purpose: 
//   Selects the value that we received.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:07:30 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
KeepAliveRPC::SelectAll()
{
    Select(0, (void*)&value);
}

// ****************************************************************************
// Method: KeepAliveRPC::SetValue
//
// Purpose: 
//   Sets the RPC's value.
//
// Arguments:
//   v : The new value for the RPC's value field.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:08:19 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
KeepAliveRPC::SetValue(int v)
{
    value = v;
    SelectAll();
}

// ****************************************************************************
// Method: KeepAliveRPC::GetValue
//
// Purpose: 
//   Returns the value for the RPC's value field.
//
// Returns:    The RPC's value field.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:09:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

int
KeepAliveRPC::GetValue() const
{
    return value;
}
