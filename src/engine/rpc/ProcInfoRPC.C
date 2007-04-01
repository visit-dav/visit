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
