#ifndef SUBJECT_H
#define SUBJECT_H
#include <state_exports.h>
#include <vector>

// Forward declaration
class SimpleObserver;

// *******************************************************************
// Class: Subject
//
// Purpose:
//   Subject base class for Subject-Observer model. The subject is the
//   object that is watched by the observers. When the subject changes,
//   all of its obsevers are notified.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 15:59:45 PST 2000
//
// Modifications:
//   
// *******************************************************************

class STATE_API Subject
{
public:
    virtual ~Subject();

    virtual void Attach(SimpleObserver *);
    virtual void Detach(SimpleObserver *);
    virtual void Notify();
protected:
    Subject();
private:
    std::vector<SimpleObserver *> observers;
};

#endif
