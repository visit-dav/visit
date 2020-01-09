// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LINEOUT_INFO_H
#define LINEOUT_INFO_H

#include <Line.h>

// ****************************************************************************
// LineOutInfo
//
// Purpose:
//   A struct to hold basic lineout information. (Passed between viewer &
//   vis window during callback.)
//
// Programmer: Kathleen Bonnell
// Creation:   January 14, 2002
//
// ****************************************************************************

typedef struct LineOutInfo
{
    Line            atts;
    void           *callbackData;
} LINE_OUT_INFO;


#endif
