#include <GetDirectoryRPC.h>
#include <DebugStream.h>
#include <string>

using std::string;

// *******************************************************************
// Method: GetDirectoryRPC::GetDirectoryRPC
//
// Purpose: 
//   Constructor for the GetDirectoryRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 10:52:47 PDT 2000
//   I changed the format string to be empty.
//
// *******************************************************************

GetDirectoryRPC::GetDirectoryRPC() : BlockingRPC("", &cwd), cwd()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::~GetDirectoryRPC
//
// Purpose: 
//   Destructor for the GetDirectoryRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//
// *******************************************************************

GetDirectoryRPC::~GetDirectoryRPC()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::operator()
//
// Purpose: 
//   Executes the RPC and returns the current directory.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//
//    Jeremy Meredith, Fri Nov 17 16:30:46 PST 2000
//    Made output go to log file instead of cout or cerr.
//
// *******************************************************************

string
GetDirectoryRPC::operator()()
{
    debug3 << "Executing GetDirectoryRPC RPC\n";

    Execute();
    return cwd.name;
}

// *******************************************************************
// Method: GetDirectoryRPC::SelectAll
//
// Purpose: 
//   Selects all the attributes that comprise the RPC's parameter list.
//
// Notes:
//   This RPC has no parameters so the no attributes are selected.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:52:20 PDT 2000
//
// Modifications:
//
// *******************************************************************

void
GetDirectoryRPC::SelectAll()
{
    // no data sent
}

// *******************************************************************
// Method: GetDirectoryRPC::DirectoryName::DirectoryName
//
// Purpose: 
//   Constructor for the GetDirectoryRPC::DirectoryName class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 11:47:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

GetDirectoryRPC::DirectoryName::DirectoryName() : AttributeSubject("s"),
    name()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::DirectoryName::~DirectoryName
//
// Purpose: 
//   Destructor for the GetDirectoryRPC::DirectoryName class.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 11:47:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

GetDirectoryRPC::DirectoryName::~DirectoryName()
{
}

// *******************************************************************
// Method: GetDirectoryRPC::DirectoryName::SelectAll
//
// Purpose: 
//   Selects all the attributes in the object.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 11:47:38 PDT 2000
//
// Modifications:
//   
// *******************************************************************

void
GetDirectoryRPC::DirectoryName::SelectAll()
{
    Select(0, (void *)&name);
}
