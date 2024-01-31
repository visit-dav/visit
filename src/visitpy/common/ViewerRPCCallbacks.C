// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerRPCCallbacks.h>

// ****************************************************************************
// Method: ViewerRPCCallbacks::ViewerRPCCallbacks
//
// Purpose: 
//   Constructor for the ViewerRPCCallbacks class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:46:12 PST 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerRPCCallbacks::ViewerRPCCallbacks()
{
    for(int r = 0; r < (int)ViewerRPC::MaxRPC; ++r)
    {
        pycb[r] = 0;
        pycb_data[r] = 0;
    }
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::~ViewerRPCCallbacks
//
// Purpose: 
//   Destructor for the ViewerRPCCallbacks class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:46:12 PST 2008
//
// Modifications:
//   Brad Whitlock, Thu Aug 19 15:52:59 PDT 2010
//   Be careful about deleting a class instance from here.
//   
// ****************************************************************************

ViewerRPCCallbacks::~ViewerRPCCallbacks()
{
    for(int r = 0; r < (int)ViewerRPC::MaxRPC; ++r)
    {
        if(pycb[r] != 0)
            Py_DECREF(pycb[r]);
        if(pycb_data[r] != 0)
        {
#ifdef Py_REFCNT
            // Hack! I found that decrementing the refcount of a class 
            //       instance from here causes a crash if it's the last
            //       reference and it will cause the object to be deleted.
            //       Decrementing the refcount on other object types is fine,
            //       even if it causes the object to get deleted. Other objects
            //       work okay so our reference counting seems good.
            bool lastInstance = Py_REFCNT(pycb_data[r]) == 1 &&
                                PyType_Check(pycb_data[r]);
            if(!lastInstance)
#endif
                Py_DECREF(pycb_data[r]);
        }
    }
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::GetCallbackNames
//
// Purpose: 
//   Get the callback names.
//
// Arguments:
//   names : The return vector for the callback names.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:46:40 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
ViewerRPCCallbacks::GetCallbackNames(stringVector &names)
{
    for(int r = 0; r < (int)ViewerRPC::MaxRPC; ++r)
        names.push_back(ViewerRPC::ViewerRPCType_ToString((ViewerRPC::ViewerRPCType)r));
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::RegisterCallback
//
// Purpose: 
//   Registers a callback for a specific viewerrpc (by name).
//
// Arguments:
//   rpcName : The name of the ViewerRPC for which we're installing a callback.
//   cb      : The user-provided callback.
//   cbdata  : callback data.
//
// Returns:    True on success; False on failure.
//
// Note:       Call with NULL cb, cbdata to unregister the callback.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:47:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerRPCCallbacks::RegisterCallback(const std::string &rpcName, PyObject *cb, 
    PyObject *cbdata)
{
    bool retval = false;
    ViewerRPC::ViewerRPCType r;
    if(ViewerRPC::ViewerRPCType_FromString(rpcName, r))
    {
        if(pycb[r] != 0)
            Py_DECREF(pycb[r]);
        if(pycb_data[r] != 0)
            Py_DECREF(pycb_data[r]);
        pycb[r] = cb;
        pycb_data[r] = cbdata;
        if(pycb[r] != 0)
            Py_INCREF(pycb[r]);
        if(pycb_data[r] != 0)
            Py_INCREF(pycb_data[r]);
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::GetCallback
//
// Purpose: 
//   Returns the user-defined callback for the specified rpc.
//
// Arguments:
//   r : The rpc whose callback we want to access.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:48:55 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
ViewerRPCCallbacks::GetCallback(ViewerRPC::ViewerRPCType r)
{
    return pycb[r];
}

// ****************************************************************************
// Method: ViewerRPCCallbacks::GetCallbackData
//
// Purpose: 
//   Returns the user-defined callback data for the specified rpc.
//
// Arguments:
//   r : The rpc whose callback data we want to access.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb  6 10:26:44 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
ViewerRPCCallbacks::GetCallbackData(ViewerRPC::ViewerRPCType r)
{
    return pycb_data[r];
}
