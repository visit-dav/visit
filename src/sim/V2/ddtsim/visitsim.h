// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISITSIM_H
#define VISITSIM_H

class VisItSim
{
public:
    VisItSim(int rank, int size);

    int runmode()       { return mRunmode; }
    int rank()          { return mRank; }
    int size()          { return mSize; }
    int cycle()         { return mCycle; }
    double time()       { return mTime; }

    bool autoupdate()   { return mAutoupdate; }
    bool stepping()     { return mStepping; }

    void setRunmode(int runmode) { mRunmode = runmode; }
    void setAutoupdate(bool b)   { mAutoupdate = b; }
    void setStepping(bool b)     { mStepping = b; }

    void incCycleAndTime()     { ++mCycle; mTime = mCycle; }
private:
    int mRank;
    int mSize;

    int mCycle;
    double mTime;

    bool mAutoupdate;
    bool mStepping;

    int mRunmode;
};

#endif // VISITSIM_H
