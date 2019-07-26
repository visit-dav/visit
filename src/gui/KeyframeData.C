// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <KeyframeData.h>

// ****************************************************************************
// Method: PlotRangeData::PlotRangeData
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 13:21:16 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PlotRangeData::PlotRangeData() : id(-1), numFrames(0), currentIndex(0), start(0), end(0)
{
}

// ****************************************************************************
// Method: KeyframePoints::KeyframePoints
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 13:21:16 PST 2008
//
// Modifications:
//   
// ****************************************************************************

KeyframePoints::KeyframePoints() : id(-1), numFrames(0), currentIndex(0), idToIndex()
{
}
