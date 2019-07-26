// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "EnginePropertiesRPC.h" 


// ****************************************************************************
//  Method: EnginePropertiesRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 10 11:13:18 PDT 2011
//
// ****************************************************************************

EnginePropertiesRPC::EnginePropertiesRPC() : NonBlockingRPC("a", &returnAtts)
{
}
