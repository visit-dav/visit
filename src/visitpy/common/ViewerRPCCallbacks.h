// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWERRPC_CALLBACKS_H
#define VIEWERRPC_CALLBACKS_H
#include <Python.h>
#include <ViewerRPC.h>
#include "Py2and3Support.h"

// ****************************************************************************
// Class: ViewerRPCCallbacks
//
// Purpose:
//   Keeps track of user-defined callbacks for specific ViewerRPC values.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  4 09:21:44 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Feb  6 10:26:09 PST 2008
//   Added callback data.
//
// ****************************************************************************

class ViewerRPCCallbacks
{
public:
    ViewerRPCCallbacks();
    ~ViewerRPCCallbacks();

    void GetCallbackNames(stringVector &names);
    bool RegisterCallback(const std::string &, PyObject *, PyObject *);
    PyObject *GetCallback(ViewerRPC::ViewerRPCType);
    PyObject *GetCallbackData(ViewerRPC::ViewerRPCType);
private:
    PyObject *pycb[ViewerRPC::MaxRPC];
    PyObject *pycb_data[ViewerRPC::MaxRPC];
};

#endif
