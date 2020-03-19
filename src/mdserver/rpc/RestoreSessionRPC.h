// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _RESTORE_SESSION_RPC_H_
#define _RESTORE_SESSION_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: RestoreSessionRPC
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

class MDSERVER_RPC_API RestoreSessionRPC : public BlockingRPC
{
public:
    struct MDSERVER_RPC_API SessionFile : public AttributeSubject
    {
        std::string sessionFile;
    public:
        SessionFile();
        ~SessionFile();
        virtual void SelectAll();
        virtual const std::string TypeName() const;
    };

public:
    RestoreSessionRPC();
    virtual ~RestoreSessionRPC();

    // Invokation method
    void operator()(const std::string &_filename, std::string &_contents);

    // Property selection methods
    virtual void SelectAll();

    // Return name of object.
    virtual const std::string TypeName() const;

    // Restore session file to file system.
    void RestoreSessionFile();

private:
    std::string filename;

    SessionFile sf;
};

#endif

