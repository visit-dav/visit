#include <GetMetaDataRPC.h>
#include <GetMetaDataException.h>
#include <DebugStream.h>
#include <string>
using std::string;

// ****************************************************************************
// Method: GetMetaDataRPC::GetMetaDataRPC
//
// Purpose: 
//   Constructor for the GetMetaDataRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:23:52 PST 2003
//   I added timeState.
//
// ****************************************************************************

GetMetaDataRPC::GetMetaDataRPC() : BlockingRPC("si",&metaData)
{
    timeState = 0;
}

// ****************************************************************************
// Method: GetMetaDataRPC::~GetMetaDataRPC
//
// Purpose: 
//   Destructor for the GetMetaDataRPC class.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//
// ****************************************************************************

GetMetaDataRPC::~GetMetaDataRPC()
{
}

// ****************************************************************************
// Method: GetMetaDataRPC::TypeName
//
// Purpose: 
//   Returns the name of the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:28:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const std::string
GetMetaDataRPC::TypeName() const
{
    return "GetMetaDataRPC";
}

// ****************************************************************************
// Method: GetMetaDataRPC::operator()
//
// Purpose: 
//   This is the () operator for the GetMetaDataRPC class. This method
//   makes the objects of this class function objects. This method
//   executes the RPC to get the metadata from a file and returns
//   a pointer to the metadata.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//    Jeremy Meredith, Fri Nov 17 16:29:43 PST 2000
//    Made output go to log file instead of cout or cerr.
//
//    Brad Whitlock, Thu Feb 7 11:11:27 PDT 2002
//    Added the error message to the new exception.
//
//    Brad Whitlock, Tue May 13 15:24:16 PST 2003
//    I added timeState.
//
// ****************************************************************************

const avtDatabaseMetaData *
GetMetaDataRPC::operator()(const string &f, int ts)
{
    debug3 << "Executing GetMetaData RPC on file " << f.c_str()
           << ", timestate=" << ts << endl;

    SetFile(f);
    SetTimeState(ts);

    // Try to execute the RPC.
    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(GetMetaDataException, GetReply()->GetMessage());
    }

    return &metaData;
}

// ****************************************************************************
// Method: GetMetaDataRPC::SelectAll
//
// Purpose: 
//   This method selects all of the components in the GetMetaDataRPC
//   before the RPC is executed. This is so RPC function parameters
//   would be communicated.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:25:11 PST 2003
//   I added timeState.
//
// ****************************************************************************

void
GetMetaDataRPC::SelectAll()
{
    Select(0, (void*)&file);
    Select(1, (void*)&timeState);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetFile
//
// Purpose: 
//   This sets the file name from which to get the metadata.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// ****************************************************************************

void
GetMetaDataRPC::SetFile(const std::string &f)
{
    file = f;
    Select(0, (void*)&file);
}

// ****************************************************************************
// Method: GetMetaDataRPC::SetTimeState
//
// Purpose: 
//   This sets the timestate for which to get metadata.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:26:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
GetMetaDataRPC::SetTimeState(int ts)
{
    timeState = ts;
    Select(1, (void*)&timeState);
}

// ****************************************************************************
// Method: GetMetaDataRPC::GetFile
//
// Purpose: 
//   This gets the file name from which to get the metadata.
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   
// ****************************************************************************

string
GetMetaDataRPC::GetFile() const
{
    return file;
}

// ****************************************************************************
// Method: GetMetaDataRPC::GetTimeState
//
// Purpose: 
//   This gets the timestate for which to get metadata.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:26:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
GetMetaDataRPC::GetTimeState() const
{
    return timeState;
}
