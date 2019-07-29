// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetMFileOpenOptionsRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetMFileOpenOptionsRPC::SetMFileOpenOptionsRPC
//
// Purpose: 
//   This is the constructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

SetMFileOpenOptionsRPC::SetMFileOpenOptionsRPC()
    : BlockingRPC("a",NULL)
{
}


// *******************************************************************
// Denstructor: SetMFileOpenOptionsRPC::~SetMFileOpenOptionsRPC
//
// Purpose: 
//   This is the denstructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

SetMFileOpenOptionsRPC::~SetMFileOpenOptionsRPC()
{
}


// *******************************************************************
// Method: SetMFileOpenOptionsRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   opts : These are the new options.
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

void
SetMFileOpenOptionsRPC::operator()(const FileOpenOptions &opts)
{
    debug3 << "Executing SetMFileOpenOptionsRPC\n";

    SetFileOpenOptions(opts);
    Execute();
}


// *******************************************************************
// Method: SetMFileOpenOptionsRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

void
SetMFileOpenOptionsRPC::SelectAll()
{
    Select(0, (void*)&fileOpenOptions);
}


// *******************************************************************
// Method: SetMFileOpenOptionsRPC::SetDirectory
//
// Purpose: 
//   Set the directory argument of the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

void
SetMFileOpenOptionsRPC::SetFileOpenOptions(const FileOpenOptions &opts)
{
    fileOpenOptions = opts;
}

// *******************************************************************
// Method: SetMFileOpenOptionsRPC::GetDirectory
//
// Purpose: 
//   Get the file options for the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

const FileOpenOptions &
SetMFileOpenOptionsRPC::GetFileOpenOptions() const
{
    return fileOpenOptions;
}

// ****************************************************************************
// Method: SetMFileOpenOptionsRPC::TypeName
//
// Purpose: 
//   Returns the name of the RPC.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//   
// ****************************************************************************

const std::string
SetMFileOpenOptionsRPC::TypeName() const
{
    return "SetMFileOpenOptionsRPC";
}
