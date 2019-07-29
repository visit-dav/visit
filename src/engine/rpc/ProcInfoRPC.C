// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ProcInfoRPC.h" 


// ****************************************************************************
//  Method: ProcInfoRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2004
//
// ****************************************************************************

ProcInfoRPC::ProcInfoRPC() : NonBlockingRPC("a", &returnAtts)
{
  // nothing here;
}
