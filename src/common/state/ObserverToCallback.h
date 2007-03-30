#ifndef OBSERVER_TO_CALLBACK_H
#define OBSERVER_TO_CALLBACK_H
#include <state_exports.h>
#include <Observer.h>

//
// This is the type of callback function to use.
//
typedef void (ObserverCallback)(Subject *subj, void *data);

// ****************************************************************************
// Class: ObserverToCallback
//
// Purpose:
//   This is a special observer subclass that can be used to call a callback
//   function when its subject does a Notify().
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:13:46 PST 2001
//
// Modifications:
//   
// ****************************************************************************

class STATE_API ObserverToCallback : public Observer
{
public:
    ObserverToCallback(Subject *subj,
                       ObserverCallback *cb = 0,
                       void *cbd = 0);
    virtual ~ObserverToCallback();
    virtual void Update(Subject *subj);

    void SetCallback(ObserverCallback *cb);
    void SetCallbackData(void *cbd);
private:
    ObserverCallback *cbFunction;
    void             *cbData;
};

#endif
