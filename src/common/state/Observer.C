#include <Observer.h>
#include <Subject.h>

// *******************************************************************
// Method: Observer::Observer
//
// Purpose: 
//   Constructor for the Observer class. It registers the observer
//   with the subject that is passed in.
//
// Arguments:
//     s : This is a pointer to a valid subject that we want to observe.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 6 10:43:37 PDT 2000
//
// Modifications:
//   
// *******************************************************************

Observer::Observer(Subject *s) : SimpleObserver()
{
    subject = s;
    subject->Attach(this);
}

// *******************************************************************
// Method: Observer::~Observer
//
// Purpose: 
//   Destructor for the Observer class. Detaches this observer from
//   the subject that it is watching.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 6 10:44:47 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 31 15:05:59 PST 2000
//   I changed the code so it does not try to detach from the subject.
//   The subject told the observer that it was destroyed.
//
// *******************************************************************

Observer::~Observer()
{
    if(subject != 0)
    {
        subject->Detach(this);
        subject = 0;
    }
}

// *******************************************************************
// Method: Observer::SubjectRemoved
//
// Purpose: 
//   Indicates that the subject that the Observer was watching is
//   not around anymore.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 15:20:45 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Observer::SubjectRemoved(Subject *)
{
    subject = 0;
}
