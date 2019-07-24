// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SaveSessionRPC.h>
#include <DebugStream.h>


// *******************************************************************
// Constructor: SaveSessionRPC::SaveSessionRPC
//
// Purpose: 
//   This is the constructor.
//
// Arguments:
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

SaveSessionRPC::SaveSessionRPC() : BlockingRPC("ss",NULL)
{
}


// *******************************************************************
// Denstructor: SaveSessionRPC::~SaveSessionRPC
//
// Purpose: 
//   This is the destructor.
//
// Arguments:
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

SaveSessionRPC::~SaveSessionRPC()
{
}


// *******************************************************************
// Method: SaveSessionRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   filename : Filename to save session too.
//   sessionFile : Session file to save.
//
// Note:       
//   If the RPC returned an error, throw an exception.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

void
SaveSessionRPC::operator()(const std::string &_filename, 
                           const std::string &_sessionFile)
{
    if (DebugStream::Level3())
    {
        debug3  << "Executing SaveSessionRPC RPC" 
                << "\n\t In Filename ='" << _filename.c_str() << "'"
                << endl;
    }

    filename    = _filename;
    sessionFile = _sessionFile;

    Execute();
}


// *******************************************************************
// Method: SaveSessionRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

void
SaveSessionRPC::SelectAll()
{
    Select(0, (void*)&filename);
    Select(1, (void*)&sessionFile);
}


// ****************************************************************************
// Method: SaveSessionRPC::TypeName
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
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

const std::string
SaveSessionRPC::TypeName() const
{
    return "SaveSessionRPC";
}

// ****************************************************************************
// Method: SaveSessionRPC::SaveSessionFile
//
// Purpose: 
//   Save session file to file system.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

bool
SaveSessionRPC::SaveSessionFile()
{
    if (DebugStream::Level3())
    {
        debug3  << "Executing SaveSessionFile" << endl;
    }

    ofstream sFile(filename.c_str(), (ios::out | ios::trunc));
    if (sFile.is_open())
    {
        sFile << sessionFile;
        sFile.close();
    }
    else
    {
        // Send error message with filename.
        std::string errMessage("Failed to save remote session file: ");
        errMessage += filename;

        debug1  << errMessage << endl;
        SendError(errMessage);
        return( false );
    }

    return( true );
}

