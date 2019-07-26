// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <RestoreSessionRPC.h>
#include <DebugStream.h>


// *******************************************************************
// Constructor: RestoreSessionRPC::RestoreSessionRPC
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

RestoreSessionRPC::RestoreSessionRPC() : BlockingRPC("ss", &sf), sf()
{
}


// *******************************************************************
// Denstructor: RestoreSessionRPC::~RestoreSessionRPC
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

RestoreSessionRPC::~RestoreSessionRPC()
{
}


// *******************************************************************
// Method: RestoreSessionRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   filename : Filename to load session from.
//   sessionFile : Load session file into this var.
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
RestoreSessionRPC::operator()(const std::string &_filename, 
                              std::string &_sessionFile)
{
    if (DebugStream::Level3())
    {
        debug3  << "Executing RestoreSessionRPC RPC" 
                << "\n\t In Filename ='" << _filename.c_str() << "'"
                << endl;
    }

    filename    = _filename;

    Execute();

    _sessionFile = sf.sessionFile;
}


// *******************************************************************
// Method: RestoreSessionRPC::SelectAll
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
RestoreSessionRPC::SelectAll()
{
    Select(0, (void*)&filename);
    //Select(1, (void*)&sessionFile);
}


// ****************************************************************************
// Method: RestoreSessionRPC::TypeName
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
RestoreSessionRPC::TypeName() const
{
    return "RestoreSessionRPC";
}

// ****************************************************************************
// Method: RestoreSessionRPC::RestoreSessionFile
//
// Purpose: 
//   Restore session file to file system.
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

void
RestoreSessionRPC::RestoreSessionFile()
{
    if (DebugStream::Level3())
    {
        debug3  << "Executing RestoreSessionFile" << endl;
    }

    ifstream sFile(filename.c_str());
    if (sFile.is_open())
    {
        sf.sessionFile.assign((std::istreambuf_iterator<char>(sFile)), std::istreambuf_iterator<char>());
        sFile.close();

        SendReply(&sf);
    }
    else
    {
        // Send error message with filename.
        std::string errMessage("Failed to load remote session file: ");
        errMessage += filename;

        debug1  << errMessage << endl;
        SendError(errMessage);
    }
}

// *******************************************************************
// Method: RestoreSessionRPC::SessionFile::SessionFile
//
// Purpose:
//   Constructor for the RestoreSessionRPC::SessionFile class.
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// *******************************************************************

RestoreSessionRPC::SessionFile::SessionFile() : AttributeSubject("s"),
    sessionFile()
{
}

// *******************************************************************
// Method: RestoreSessionRPC::SessionFile::~SessionFile
//
// Purpose:
//   Destructor for the RestoreSessionRPC::SessionFile class.
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// *******************************************************************

RestoreSessionRPC::SessionFile::~SessionFile()
{
}

// *******************************************************************
// Method: RestoreSessionRPC::SessionFile::SelectAll
//
// Purpose:
//   Selects all the attributes in the object.
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// *******************************************************************

void
RestoreSessionRPC::SessionFile::SelectAll()
{
    Select(0, (void *)&sessionFile);
}

// ****************************************************************************
// Method: RestoreSessionRPC::SessionFile::TypeName
//
// Purpose:
//   Returns the RPC sessionFile.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// ****************************************************************************

const std::string
RestoreSessionRPC::SessionFile::TypeName() const
{
    return "RestoreSessionRPC::SessionFile";
}

