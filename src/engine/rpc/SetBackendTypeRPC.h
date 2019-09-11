// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_BACKEND_TYPE_RPC_H
#define SET_BACKEND_TYPE_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: SetBackendTypeRPC
//
// Purpose:
//   This RPC sets the default backend type.
//
// Notes:
//
// Programmer: Cameron Christensen
// Creation:   June 10, 2014
//
// Modifications:
//
// ****************************************************************************
class ENGINE_RPC_API SetBackendTypeRPC : public BlockingRPC
{
public:
    SetBackendTypeRPC();
    virtual ~SetBackendTypeRPC();

    // Invocation method
    void operator()(const int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetBackendType(const int);

    // Property getting methods
    int GetBackendType() const;

    // Return name of object.
    virtual const std::string TypeName() const;
private:
    int backendType;
};

#endif
