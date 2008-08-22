#include <ViewerStateBuffered.h>
#include <ViewerClientConnection.h>
#include <ViewerState.h>

// ****************************************************************************
// Method: ViewerStateBuffered::ViewerStateBuffered
//
// Purpose: 
//   Constructor for the ViewerStateBuffered class.
//
// Arguments:
//   s : The global ViewerState whose composition we'll copy.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 14:29:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerStateBuffered::ViewerStateBuffered(ViewerState *s) : ViewerBase(), SimpleObserver()
{
    // Create a copy of ViewerState and make this object observe it.
    viewerState = new ViewerState(*s);
    for(int i = 0; i < viewerState->GetNumStateObjects(); ++i)
    {
        viewerState->GetStateObject(i)->SetGuido(i);
        viewerState->GetStateObject(i)->Attach(this);
    }

    // Let's also observe the real viewer state so that we can copy it into this 
    // object's local viewer state.
    for(int i = 0; i < s->GetNumStateObjects(); ++i)
        s->GetStateObject(i)->Attach(this);
}

// ****************************************************************************
// Method: ViewerStateBuffered::~ViewerStateBuffered
//
// Purpose: 
//   Destructor for the ViewerStateBuffered class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 14:30:02 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerStateBuffered::~ViewerStateBuffered()
{
    delete viewerState;
}

// ****************************************************************************
// Method: ViewerStateBuffered::Update
//
// Purpose: 
//   This method is called when Notify is called on objects in the local
//   viewer state or objects in the global viewer state Notify observers.
//
// Arguments:
//   subj : The subject that caused the Notify.
//
// Returns:    
//
// Note:       If the Notify came from global viewer state then we copy that
//             object into our local viewer state to maintain consistency. If
//             the update came from the local viewer state then we emit a 
//             signal so the object will be serialized to the central xfer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 14:31:06 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
ViewerStateBuffered::Update(Subject *subj)
{
    AttributeSubject *obj = (AttributeSubject *)subj;

    // If the subject that updated is from the viewer state then we need to mirror
    // that state in our local viewer state so do a copy. The global and local
    // viewer states have identical composition so this is safe.
    for(int i = 0; i < GetViewerState()->GetNumStateObjects(); ++i)
    {
        if(obj == GetViewerState()->GetStateObject(i))
        {
            viewerState->GetStateObject(i)->CopyAttributes(obj);
            return;
        }
    }
    
    // The subject that updated was from our local viewer state so emit the
    // InputFromClient signal so the subject will be serialized to the
    // central buffered Xfer input.
    emit InputFromClient(0, obj);
}
