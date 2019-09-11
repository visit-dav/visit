// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "visitsim.h"

#include <VisItInterfaceTypes_V2.h>

VisItSim::VisItSim(int rank, int size) : mRank(rank), mSize(size), mCycle(0), mTime(0),
    mAutoupdate(true), mStepping(false), mRunmode(VISIT_SIMMODE_STOPPED)
{
}

