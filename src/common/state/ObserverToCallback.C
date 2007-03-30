#include <ObserverToCallback.h>
#include <Subject.h>

// ****************************************************************************
// Method: ObserverToCallback::ObserverToCallback
//
// Purpose: 
//   Constructor for the ObserverToCallback class.
//
// Arguments:
//   subj : The subject that the observer will observe.
//   cb   : The callback function to call when the subject updates.
//   cbd  : Data to pass to the callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:22:30 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ObserverToCallback::ObserverToCallback(Subject *subj,
    ObserverCallback *cb, void *cbd) : Observer(subj)
{
    cbFunction = cb;
    cbData = cbd;
}

// ****************************************************************************
// Method: ObserverToCallback::~ObserverToCallback
//
// Purpose: 
//   Destructor for the ObserverToCallback class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:23:50 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ObserverToCallback::~ObserverToCallback()
{
    cbFunction = 0;
    cbData = 0;
}

// ****************************************************************************
// Method: ObserverToCallback::Update
//
// Purpose: 
//   This method is called when the observer's subject is modified.
//
// Arguments:
//   subj : A pointer to the observer's subject.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:24:11 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ObserverToCallback::Update(Subject *subj)
{
    if(cbFunction)
    {
        (*cbFunction)(subj, cbData);
    }
}

// ****************************************************************************
// Method: ObserverToCallback::SetCallback
//
// Purpose: 
//   Sets the object's callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:25:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ObserverToCallback::SetCallback(ObserverCallback *cb)
{
    cbFunction = cb;
}

// ****************************************************************************
// Method: ObserverToCallback::SetCallbackData
//
// Purpose: 
//   Sets the object's callback function data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 17:25:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ObserverToCallback::SetCallbackData(void *cbd)
{
    cbData = cbd;
}
