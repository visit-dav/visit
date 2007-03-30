#ifndef OBSERVER_H
#define OBSERVER_H
#include <state_exports.h>
#include <SimpleObserver.h>

// *******************************************************************
// Class: Observer
//
// Purpose:
//   Same as SimpleObserver. The difference is that this class requires
//   a Subject pointer in order to be instantiated. This lets you get
//   all the Attach/Detach stuff for free.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 6 10:38:47 PDT 2000
//
// Modifications:
//   
// *******************************************************************

class STATE_API Observer : public SimpleObserver
{
public:
    Observer(Subject *s);
    virtual ~Observer();
    virtual void Update(Subject *TheChangedSubject) = 0;
    virtual void SubjectRemoved(Subject *);
protected:
    Subject *subject;
};

#endif
