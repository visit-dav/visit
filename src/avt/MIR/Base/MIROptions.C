// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 11:22:39 EST 2009
//    Added MIR iteration capability.
//
// ****************************************************************************
MIROptions::MIROptions()
{
    algorithm            = 1;
    subdivisionLevel     = MIROptions::Low;
    numIterations        = 0;
    iterationDamping     = 0.4;
    smoothing            = true;
    leaveCleanZonesWhole = true;
    cleanZonesOnly       = false;
    isovolumeVF          = 0.5;
    annealingTime        = 10;
}
