#include <ExpandPathRPC.h>
#include <DebugStream.h>
#include <VisItException.h>

// ****************************************************************************
// Constructor: ExpandPathRPC::ExpandPathRPC
//
// Purpose: 
//   This is the constructor.
//
// Programmer: Brad Whitlock
// Creation:   August 29, 2000
//
// Modifications:
//
// ****************************************************************************

ExpandPathRPC::ExpandPathRPC() : BlockingRPC("a", &path)
{
}


// ****************************************************************************
// Denstructor: ExpandPathRPC::~ExpandPathRPC
//
// Purpose: 
//   This is the denstructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:42:29 PST 2002
//
// Modifications:
//
// ****************************************************************************

ExpandPathRPC::~ExpandPathRPC()
{
}


// ****************************************************************************
// Method: ExpandPathRPC::operator()
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
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:42:29 PST 2002
//
// Modifications:
//
// ****************************************************************************

std::string
ExpandPathRPC::operator()(const std::string &s)
{
    debug3 << "Executing ExpandPath RPC" 
           << "\n\t directory='" << s.c_str() << "'"
           << endl;

    SetPath(s);
    Execute();
    return path.name;
}


// ****************************************************************************
// Method: ExpandPathRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:42:29 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
ExpandPathRPC::SelectAll()
{
    Select(0, (void*)&path);
}

// ****************************************************************************
// Method: ExpandPathRPC::SetPath
//
// Purpose: 
//   Sets the path that we're going to expand.
//
// Arguments:
//   p : The path
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 15:37:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ExpandPathRPC::SetPath(const std::string &p)
{
    path.name = p;
    path.SelectAll();
    SelectAll();
}

// *******************************************************************
// Method: ExpandPathRPC::PathName::PathName
//
// Purpose: 
//   Constructor for the ExpandPathRPC::PathName class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:44:30 PST 2002
//
// Modifications:
//   
// *******************************************************************

ExpandPathRPC::PathName::PathName() : AttributeSubject("s"),
    name()
{
}

// *******************************************************************
// Method: ExpandPathRPC::PathName::~PathName
//
// Purpose: 
//   Destructor for the ExpandPathRPC::PathName class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:44:30 PST 2002
//
// Modifications:
//   
// *******************************************************************

ExpandPathRPC::PathName::~PathName()
{
}

// *******************************************************************
// Method: ExpandPathRPC::PathName::SelectAll
//
// Purpose: 
//   Selects all the attributes in the object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:44:30 PST 2002
//
// Modifications:
//   
// *******************************************************************

void
ExpandPathRPC::PathName::SelectAll()
{
    Select(0, (void *)&name);
}
