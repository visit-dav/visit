#include "MIROptions.h"

// ****************************************************************************
//  Constructor:  MIROptions::MIROptions
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2002
//
// ****************************************************************************
MIROptions::MIROptions()
{
    subdivisionLevel     = MIROptions::Low;
    numIterations        = 0;
    smoothing            = true;
    leaveCleanZonesWhole = true;
    cleanZonesOnly       = false;
}
