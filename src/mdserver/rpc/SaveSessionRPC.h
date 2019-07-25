// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _SAVESESSION_RPC_H_
#define _SAVESESSION_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: SaveSessionRPC
//
// Purpose:
//   This RPC save session file on a remote file system.
//
// Notes:      
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// ****************************************************************************

class MDSERVER_RPC_API SaveSessionRPC : public BlockingRPC
{
public:
    SaveSessionRPC();
    virtual ~SaveSessionRPC();

    // Invokation method
    void operator()(const std::string &_filename, const std::string &_contents);

    // Property selection methods
    virtual void SelectAll();

    // Return name of object.
    virtual const std::string TypeName() const;

    // Save session file to file system.
    bool SaveSessionFile();

private:
    std::string filename;
    std::string sessionFile;
};

#endif

