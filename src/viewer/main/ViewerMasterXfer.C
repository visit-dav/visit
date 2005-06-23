#include <ViewerMasterXfer.h>

// ****************************************************************************
// Method: ViewerMasterXfer::ViewerMasterXfer
//
// Purpose: 
//   Constructor for the ViewerMasterXfer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:11:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ViewerMasterXfer::ViewerMasterXfer() : Xfer()
{
    updateCallback = 0;
    updateCallbackData = 0;
}

// ****************************************************************************
// Method: ViewerMasterXfer::~ViewerMasterXfer
//
// Purpose: 
//   Destructor for the ViewerMasterXfer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:11:38 PST 2005
//
// Modifications:
//   
// ****************************************************************************

ViewerMasterXfer::~ViewerMasterXfer()
{
}

// ****************************************************************************
// Method: ViewerMasterXfer::Update
//
// Purpose: 
//   This method is called when the subjects attached to Xfer call their
//   Notify method.
//
// Arguments:
//   subj : A pointer to the subject that caused this method to get called.
//
// Note:       We call a special callback if one is defined. Otherwise we
//             call the base class's Update method.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:11:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMasterXfer::Update(Subject *subj)
{
    if(updateCallback != 0)
    {
        updateCallback(updateCallbackData, subj);
    }
    else
    {
        // Call the parent's Update method if we're not replacing it
        // with a callback function.
        Xfer::Update(subj);
    }
}

// ****************************************************************************
// Method: ViewerMasterXfer::SetUpdateCallback
//
// Purpose: 
//   Set up a special callback for the Update function.
//
// Arguments:
//   updateCB     : The callback function pointer.
//   updateCBData : The callback function data.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:13:24 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerMasterXfer::SetUpdateCallback(void (*updateCB)(void *, Subject *),
    void *updateCBData)
{
    updateCallback = updateCB;
    updateCallbackData = updateCBData;
}

