#include <Subject.h>
#include <SimpleObserver.h>

// *******************************************************************
// Method: Subject::Subject
//
// Purpose: 
//   Constructor for the Subject class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 16:00:53 PST 2000
//
// Modifications:
//   
// *******************************************************************

Subject::Subject()
{
}

// *******************************************************************
// Method: Subject::~Subject
//
// Purpose: 
//   Destructor for the Subject class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 16:00:53 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 31 15:17:03 PST 2000
//   Added code to tell remaining observers not to try and detach
//   from the subject.
//
// *******************************************************************

Subject::~Subject()
{
    // If there are still observers around it means that this
    // object is being destroyed first. Tell the remaining observers
    // that they should not try to detach.
    std::vector<SimpleObserver *>::iterator pos;
    for(pos = observers.begin(); pos != observers.end(); ++pos)
    {
        (*pos)->SubjectRemoved(this);
    }
}

// *******************************************************************
// Method: Subject::Attach
//
// Purpose: 
//   Adds an Observer to the list of Observers that are watching the
//   Subject. When the subject changes, the new Observer will also
//   be called.
//
// Arguments:
//   o : A pointer to the new Observer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 16:02:26 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Subject::Attach(SimpleObserver *o)
{
    observers.push_back(o);
}

// *******************************************************************
// Method: Subject::Detach
//
// Purpose: 
//   Removes an Observer from the list of Observers that is maintained
//   by the subject. The detached observer will no longer be notified
//   when the subject changes.
//
// Arguments:
//   o : A pointer to the Observer that will be removed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 16:03:29 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Subject::Detach(SimpleObserver *o)
{
    std::vector<SimpleObserver *>::iterator pos;

    // Erase all references to observer o.
    for(pos = observers.begin(); pos != observers.end(); )
    {
       if(*pos == o)
           pos = observers.erase(pos);
       else
           ++pos;
    }
}

// *******************************************************************
// Method: Subject::Notify
//
// Purpose: 
//   Notifies all Observers that are watching the subject that the
//   subject has changed.
//
// Note:       
//   If an Observer's update state is false, that Observer is not
//   notified of the update because, presumeably, it was the Observer
//   that caused the subject to change.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 16:04:43 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Subject::Notify()
{
    std::vector<SimpleObserver *>::iterator pos;

    for(pos = observers.begin(); pos != observers.end(); ++pos)
    {
        // Update the observer if it wants to be updated. If it didn't
        // want to be updated, set its update to true so it will be 
        // updated next time.
        if((*pos)->GetUpdate())
           (*pos)->Update(this);
        else
           (*pos)->SetUpdate(true);
    }
}
