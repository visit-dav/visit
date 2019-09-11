// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _CHANGEDIRECTORY_RPC_H_
#define _CHANGEDIRECTORY_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: ChangeDirectoryRPC
//
// Purpose:
//   This RPC sets the current directory on a remote file system.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 15:34:51 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Apr 17 10:46:45 PDT 2002
//   Made GetDirectory return a reference.
//
//   Brad Whitlock, Fri Dec  7 11:04:56 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class MDSERVER_RPC_API ChangeDirectoryRPC : public BlockingRPC
{
public:
    ChangeDirectoryRPC();
    virtual ~ChangeDirectoryRPC();

    // Invokation method
    void operator()(const std::string&);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetDirectory(const std::string&);

    // Property getting methods
    const std::string &GetDirectory() const;

    // Return name of object.
    virtual const std::string TypeName() const;
private:
    std::string directory;
};


#endif
