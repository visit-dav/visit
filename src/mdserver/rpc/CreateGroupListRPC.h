// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CREATEGROUPLIST_RPC_H
#define CREATEGROUPLIST_RPC_H
#include <mdsrpc_exports.h>

#include <VisItRPC.h>

#include <string>
#include <vector>

// ****************************************************************************
// Class: CreateGroupListRPC
//
// Purpose:
//   This class encodes an RPC that tells the MDserver to create a
//   grouplist with the given filename
//
// Notes:      I'm not sure I know what I'm doing.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:45:41 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Dec  7 11:12:32 PST 2007
//   Added TypeName override.
//
// ****************************************************************************

class MDSERVER_RPC_API CreateGroupListRPC : public NonBlockingRPC
{
public:
    CreateGroupListRPC();

    // Invokation method
    void operator()(const std::string, std::vector<std::string>);

    // Property selection methods
    virtual void SelectAll();

    virtual const std::string TypeName() const;

    // Access methods
    std::string filename;
    std::vector<std::string> groupList;
};


#endif
