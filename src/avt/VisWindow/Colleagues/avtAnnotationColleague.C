// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtAnnotationColleague.h>

// ****************************************************************************
// Method: avtAnnotationColleague::avtAnnotationColleague
//
// Purpose: 
//   Constructor for the avtAnnotationColleague class.
//
// Arguments:
//   m : A reference to the mediator.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 16:30:43 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 20 10:24:44 PDT 2007
//   Added name.
//
// ****************************************************************************

avtAnnotationColleague::avtAnnotationColleague(VisWindowColleagueProxy &m) :
    VisWinColleague(m), name("")
{
    active = false;
    visible = true;
    timeScale = 1.0;
    timeOffset = 0.0;
}

// ****************************************************************************
// Method: avtAnnotationColleague::~avtAnnotationColleague
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 16:31:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

avtAnnotationColleague::~avtAnnotationColleague()
{
}

// ****************************************************************************
// Method: avtAnnotationColleague::SetTimeScaleAndOffset
//
// Purpose: 
//   Sets the scale and offset that will be applied to the time.
//
// Arguments:
//   scale  : Multiplier for the time.
//   offset : Offset that will be added to the time.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar  2 14:13:57 PST 2009
//
// Modifications:
//   
//   Mark C. Miller, Thu Oct  5 14:15:30 PDT 2023
//   Moved here from avtTimeSliderColleague.C
// ****************************************************************************

void
avtAnnotationColleague::SetTimeScaleAndOffset(double scale, double offset)
{
    timeScale = scale; 
    timeOffset = offset;
}
