#ifndef SIMPLEOBSERVER_H
#define SIMPLEOBSERVER_H
#include <state_exports.h>

class Subject;

// *******************************************************************
// Class: SimpleObserver
//
// Purpose:
//   Defines an interface for the observer in the observer design
//   pattern. See Design Patterns p.293
//
// Notes:      This is an abstract base class. Note that the subject
//   that actually changed is passed to the Update method. This is so
//   we can support having observers that watch more than one subject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 1 11:28:42 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 31 15:18:18 PST 2000
//   Added SubjectRemoved.
//
// *******************************************************************

class STATE_API SimpleObserver
{
public:
    SimpleObserver();
    virtual ~SimpleObserver();
    virtual void Update(Subject *TheChangedSubject) = 0;
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    void SetUpdate(bool);
    bool GetUpdate();
private:
    bool doUpdate;
};

#endif
