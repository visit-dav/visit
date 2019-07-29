// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetEFileOpenOptionsRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetEFileOpenOptionsRPC::SetEFileOpenOptionsRPC
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

SetEFileOpenOptionsRPC::SetEFileOpenOptionsRPC()
    : BlockingRPC("a",NULL)
{
}


// *******************************************************************
// Denstructor: SetEFileOpenOptionsRPC::~SetEFileOpenOptionsRPC
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

SetEFileOpenOptionsRPC::~SetEFileOpenOptionsRPC()
{
}


// *******************************************************************
// Method: SetEFileOpenOptionsRPC::operator()
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
SetEFileOpenOptionsRPC::operator()(const FileOpenOptions &opts)
{
    debug3 << "Executing SetEFileOpenOptionsRPC\n";

    SetFileOpenOptions(opts);
    Execute();
}


// *******************************************************************
// Method: SetEFileOpenOptionsRPC::SelectAll
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
SetEFileOpenOptionsRPC::SelectAll()
{
    Select(0, (void*)&fileOpenOptions);
}


// *******************************************************************
// Method: SetEFileOpenOptionsRPC::SetDirectory
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
SetEFileOpenOptionsRPC::SetFileOpenOptions(const FileOpenOptions &opts)
{
    fileOpenOptions = opts;
}

// *******************************************************************
// Method: SetEFileOpenOptionsRPC::GetDirectory
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
SetEFileOpenOptionsRPC::GetFileOpenOptions() const
{
    return fileOpenOptions;
}

// ****************************************************************************
// Method: SetEFileOpenOptionsRPC::TypeName
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
SetEFileOpenOptionsRPC::TypeName() const
{
    return "SetEFileOpenOptionsRPC";
}
