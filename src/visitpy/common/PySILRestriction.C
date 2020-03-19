// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PySILRestriction.h>
#include <PySILRestrictionBase.h>
#include <ViewerProxy.h>
#include <VisItException.h>
#include <avtSILRestrictionTraverser.h>

#include <visitmodulehelpers.h>

// ****************************************************************************
//  File: PySILRestriction.C
//
//  Modifications:
//  Cyrus Harrison, Wed Feb 10 10:13:30 PST 2010
//   NOTE: I needed wrap SILRestrictions without any viewer proxy
//         dependancies for contract access in Python Filters. To accomplish
//         this I moved most of the functionality from PySILRestriction.{h,C}
//         into PySILRestrictionBase.{h,C} and left the visitmodule specific
//         pieces in PySILRestriction.{h,C}.
//
// ****************************************************************************

// ****************************************************************************
// Function: NewSILRestrictionObject
//
// Purpose:
//   Helper for constructing a new PySILRestriction object.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//   Cyrus Harrison, Wed Feb 10 10:00:33 PST 2010
//   Changed to use PySILRestriction_Wrap.
//
// ****************************************************************************

static PyObject *
NewSILRestrictionObject()
{
    avtSILRestriction_p silr = GetViewerProxy()->GetPlotSILRestriction();
    return (PyObject *)PySILRestriction_Wrap(silr);
}

///////////////////////////////////////////////////////////////////////////////
//
// Interface that is exposed to the VisIt module.
//
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Function: SILRestriction_new
//
// Purpose:
//   Constructor for PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//
//
// ****************************************************************************

PyObject *
SILRestriction_new(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return (PyObject *)NewSILRestrictionObject();
}

//
// Plugin method table. These methods are added to the visitmodule's methods.
//

static PyMethodDef SILRestrictionMethods[] = {
    {"SILRestriction", SILRestriction_new, METH_VARARGS},
    {NULL,      NULL}        /* Sentinel */
};


// ****************************************************************************
// Function: PySILRestriction_GetMethodTable
//
// Purpose:
//   Provides method table for VisIt module.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//
//
// ****************************************************************************

PyMethodDef *
PySILRestriction_GetMethodTable(int *nMethods)
{
    *nMethods = 1;
    return SILRestrictionMethods;
}

// ****************************************************************************
// Function: PySILRestriction_GetMethodTable
//
// Purpose:
//   Creates a new python wrapped sil restriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//
//
// ****************************************************************************

PyObject *
PySILRestriction_NewPyObject()
{
    return NewSILRestrictionObject();
}

