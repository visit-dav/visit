// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef KEYFRAME_DATA_H
#define KEYFRAME_DATA_H
#include <QMap>
#include <QMetaType>

// ****************************************************************************
// Class: PlotRangeData
//
// Purpose:
//   This class is used to exchange data between KeyframeDataModel and 
//   QvisKeyframeDelegate. PlotRangeData represents the start,end range of
//   a plot in keyframing mode.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 11:56:07 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class PlotRangeData
{
public:
    PlotRangeData();

    int id;
    int numFrames;
    int currentIndex;

    int start;
    int end;
};
Q_DECLARE_METATYPE(PlotRangeData)

// ****************************************************************************
// Class: KeyframePoints
//
// Purpose:
//   This class is used to exchange data between KeyframeDataModel and 
//   QvisKeyframeDelegate. KeyframePoints contains a set of keyframe ids with
//   the frame numbers where the keyframes are defined.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 11:56:07 PST 2008
//
// Modifications:
//   
// ****************************************************************************

struct KeyframePoints
{
    KeyframePoints();

    int id;
    int numFrames;
    int currentIndex;

    QMap<int,int> idToIndex;
};
Q_DECLARE_METATYPE(KeyframePoints)

#endif
