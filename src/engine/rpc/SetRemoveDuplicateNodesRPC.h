// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_REMOVE_DUPLICATE_NODES_RPC_H
#define SET_REMOVE_DUPLICATE_NODES_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: SetRemoveDuplicateNodesRPC
//
// Purpose:
//   This RPC sets the flag for removing duplicate nodes.
//
// Notes:
//
// Programmer: Kathleen Biagas
// Creation:   December 22, 2014
//
// Modifications:
//
// ****************************************************************************

class ENGINE_RPC_API SetRemoveDuplicateNodesRPC : public BlockingRPC
{
public:
    SetRemoveDuplicateNodesRPC();
    virtual ~SetRemoveDuplicateNodesRPC();

    // Invocation method
    void operator()(bool);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetRemoveDuplicateNodes(bool);

    // Property getting methods
    bool GetRemoveDuplicateNodes() const;

    // Return name of object.
    virtual const std::string TypeName() const;
private:
    bool removeDuplicateNodes;
};


#endif
