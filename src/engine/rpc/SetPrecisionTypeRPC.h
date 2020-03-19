// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_PRECISION_TYPE_RPC_H
#define SET_PRECISION_TYPE_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: SetPrecisionTypeRPC
//
// Purpose:
//   This RPC sets the default precision type.
//
// Notes:
//
// Programmer: Kathleen Biagas
// Creation:   August 1, 2013
//
// Modifications:
//
// ****************************************************************************

class ENGINE_RPC_API SetPrecisionTypeRPC : public BlockingRPC
{
public:
    SetPrecisionTypeRPC();
    virtual ~SetPrecisionTypeRPC();

    // Invocation method
    void operator()(const int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPrecisionType(const int);

    // Property getting methods
    int GetPrecisionType() const;

    // Return name of object.
    virtual const std::string TypeName() const;
private:
    int precisionType;
};


#endif
