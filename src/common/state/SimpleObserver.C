#include <SimpleObserver.h>

// *******************************************************************
// Method: SimpleObserver::SimpleObserver
//
// Purpose: 
//   Constructor for the Observer class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 1 11:31:06 PDT 2000
//
// Modifications:
//   
// *******************************************************************

SimpleObserver::SimpleObserver()
{
    doUpdate = true;
}

// *******************************************************************
// Method: SimpleObserver::~SimpleObserver
//
// Purpose: 
//   Destructor for the Observer class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 1 11:31:39 PDT 2000
//
// Modifications:
//   
// *******************************************************************

SimpleObserver::~SimpleObserver()
{
    // nothing special here.
}

// *******************************************************************
// Method: SimpleObserver::SetUpdate
//
// Purpose: 
//   Sets a flag that indicates whether or not the Observer's Update
//   method should be called by the Observer's subject.
//
// Arguments:
//    update : Tells the subject we're observing if the observer's
//             Update method needs to be called during a Notify.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 1 11:32:17 PDT 2000
//
// Modifications:
//   
// *******************************************************************

void
SimpleObserver::SetUpdate(bool update)
{
    doUpdate = update;
}

// *******************************************************************
// Method: SimpleObserver::GetUpdate
//
// Purpose: 
//   Gets the value of the update flag.
//
// Arguments:
//
// Returns:    The value of the update flag.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 1 11:32:23 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
SimpleObserver::GetUpdate()
{
    return doUpdate;
}

// *******************************************************************
// Method: SimpleObserver::SubjectRemoved
//
// Purpose: 
//   Tells the observer that it should not try and detach from the
//   subject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 15:19:49 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
SimpleObserver::SubjectRemoved(Subject *)
{
    // nothing
}
