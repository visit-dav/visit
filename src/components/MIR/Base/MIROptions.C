#include "MIROptions.h"

// ****************************************************************************
//  Constructor:  MIROptions::MIROptions
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 13, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 18 16:36:55 PDT 2005
//    Added algorithm and isovolumeVF.
//
// ****************************************************************************
MIROptions::MIROptions()
{
    algorithm            = 1;
    subdivisionLevel     = MIROptions::Low;
    numIterations        = 0;
    smoothing            = true;
    leaveCleanZonesWhole = true;
    cleanZonesOnly       = false;
    isovolumeVF          = 0.5;
}
