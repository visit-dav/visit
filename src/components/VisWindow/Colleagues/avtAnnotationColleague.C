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
//   
// ****************************************************************************

avtAnnotationColleague::avtAnnotationColleague(VisWindowColleagueProxy &m) :
    VisWinColleague(m)
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
