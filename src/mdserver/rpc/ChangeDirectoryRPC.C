#include <ChangeDirectoryRPC.h>
#include <ChangeDirectoryException.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: ChangeDirectoryRPC::ChangeDirectoryRPC
//
// Purpose: 
//   This is the constructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

ChangeDirectoryRPC::ChangeDirectoryRPC() : BlockingRPC("s",NULL)
{
}


// *******************************************************************
// Denstructor: ChangeDirectoryRPC::~ChangeDirectoryRPC
//
// Purpose: 
//   This is the denstructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

ChangeDirectoryRPC::~ChangeDirectoryRPC()
{
}


// *******************************************************************
// Method: ChangeDirectoryRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   s : The directory we want to change to.
//
// Note:       
//   If the RPC returned an error, throw an exception.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 18:19:50 PST 2000
//
// Modifications:
//    Brad Whitlock, Tue Aug 29 18:21:04 PST 2000
//    I added code to throw an exception if the reply had an error.
//
//    Jeremy Meredith, Fri Nov 17 16:30:50 PST 2000
//    Made output go to log file instead of cout or cerr.
//
// *******************************************************************

void
ChangeDirectoryRPC::operator()(const string &s)
{
    debug3 << "Executing ChangeDirectory RPC" 
           << "\n\t directory='" << s.c_str() << "'"
           << endl;

    SetDirectory(s);
    Execute();

    // If there was an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(ChangeDirectoryException, s);
    }
}


// *******************************************************************
// Method: ChangeDirectoryRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

void
ChangeDirectoryRPC::SelectAll()
{
    Select(0, (void*)&directory);
}


// *******************************************************************
// Method: ChangeDirectoryRPC::SetDirectory
//
// Purpose: 
//   Set the directory argument of the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

void
ChangeDirectoryRPC::SetDirectory(const string &s)
{
    directory = s;
}

// *******************************************************************
// Method: ChangeDirectoryRPC::GetDirectory
//
// Purpose: 
//   Get the directory argument for the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   Brad Whitlock, Wed Apr 17 10:47:19 PDT 2002
//   Made the function return a reference.
//
// *******************************************************************

const string &
ChangeDirectoryRPC::GetDirectory() const
{
    return directory;
}
