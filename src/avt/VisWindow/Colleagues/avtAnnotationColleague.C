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
