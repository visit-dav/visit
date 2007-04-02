#include <GetDBPluginInfoRPC.h>
#include <GetMetaDataException.h>
#include <DebugStream.h>
#include <string>
using std::string;

// ****************************************************************************
// Method: GetDBPluginInfoRPC::GetDBPluginInfoRPC
//
// Purpose: 
//   Constructor for the GetDBPluginInfoRPC class.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// Modifications:
//
// ****************************************************************************

GetDBPluginInfoRPC::GetDBPluginInfoRPC() : BlockingRPC("",&dbPluginInfo)
{
}

// ****************************************************************************
// Method: GetDBPluginInfoRPC::~GetDBPluginInfoRPC
//
// Purpose: 
//   Destructor for the GetDBPluginInfoRPC class.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// Modifications:
//
// ****************************************************************************

GetDBPluginInfoRPC::~GetDBPluginInfoRPC()
{
}

// ****************************************************************************
// Method: GetDBPluginInfoRPC::TypeName
//
// Purpose: 
//   Returns the RPC name,
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:33:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const string
GetDBPluginInfoRPC::TypeName() const
{
    return "GetDBPluginInfoRPC";
}

// ****************************************************************************
//  Method: GetDBPluginInfoRPC::operator()
//
//  Purpose: 
//    This is the () operator for the GetDBPluginInfoRPC class. This method
//    makes the objects of this class function objects. This method
//    executes the RPC to get the db options from a plugins and returns
//    a pointer to it.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2005
//
// ****************************************************************************

const DBPluginInfoAttributes *
GetDBPluginInfoRPC::operator()()
{
    // Try to execute the RPC.
    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(GetMetaDataException, GetReply()->Message());
    }

    return &dbPluginInfo;
}

// ****************************************************************************
// Method: GetDBPluginInfoRPC::SelectAll
//
// Purpose: 
//   This method selects all of the components in the GetDBPluginInfoRPC
//   before the RPC is executed. This is so RPC function parameters
//   would be communicated.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// ****************************************************************************

void
GetDBPluginInfoRPC::SelectAll()
{
}

