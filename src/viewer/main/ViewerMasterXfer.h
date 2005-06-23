#ifndef VIEWER_XFER_H
#define VIEWER_XFER_H
#include <Xfer.h>

// ****************************************************************************
// Class: ViewerMasterXfer
//
// Purpose:
//   This is a special subclass of Xfer that lets us do a special callback
//   during an Update, if we set up a special callback.
//
// Notes:      We set up a special callback once the viewer is set up so we
//             can send a state object to multiple clients.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:09:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class ViewerMasterXfer : public Xfer
{
public:
    ViewerMasterXfer();
    virtual ~ViewerMasterXfer();
    virtual void Update(Subject *);

    void SetUpdateCallback(void (*updateCB)(void *, Subject *),
                           void *updateCBData);

protected:
    void (*updateCallback)(void *, Subject *);
    void *updateCallbackData;
};

#endif

