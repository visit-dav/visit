// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Python.h>
#include <Py2and3Support.h>
#include <PyDataRequest.h>
#include <PySILRestrictionBase.h>

// ****************************************************************************
//  Modifications:
//
// ****************************************************************************
struct PySILRequestObject
{
    PyObject_HEAD
    avtSILSpecification *request;
};

//
// avtSILSpecification  methods
//

// ****************************************************************************
// Function: SILRequest_UseRestriction
//
// Purpose:
//   Wraps avtSILSpecification::useRestriction.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_UseRestriction(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    if(request->useRestriction)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: SILRequest_GetSIL
//
// Purpose:
//   Wraps avtSILSpecification::silr.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_GetSIL(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    return PySILRestriction_Wrap(request->silr);

}

// ****************************************************************************
// Function: SILRequest_GetDataChunk
//
// Purpose:
//   Wraps avtSILSpecification::useRestriction.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_GetDataChunk(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    return PyInt_FromLong((long)request->dataChunk);
}


// ****************************************************************************
// Function: SILRequest_GetDomainList
//
// Purpose:
//   Wraps avtSILSpecification::GetDomainList.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_GetDomainList(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    std::vector<int> domains;
    request->GetDomainList(domains);

    int ndoms = domains.size();
    PyObject *res = PyTuple_New(ndoms);

    for(int i = 0; i < ndoms; ++i)
    {
        PyObject *py_var = PyInt_FromLong((long)domains[i]);
        if(py_var == NULL)
            continue;
        PyTuple_SET_ITEM(res, i, py_var);
    }

    return res;
}

// ****************************************************************************
// Function: SILRequest_UsesAllData
//
// Purpose:
//   Wraps avtSILSpecification::UsesAllData.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_UsesAllData(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    return PySILRestriction_Wrap(request->silr);

    if(request->UsesAllData())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


// ****************************************************************************
// Function: SILRequest_UsesAllDomains
//
// Purpose:
//   Wraps avtSILSpecification::UsesAllDomains.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_UsesAllDomains(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    if(request->UsesAllDomains())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: SILRequest_EmptySpecification
//
// Purpose:
//   Wraps avtSILSpecification::EmptySpecification.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_EmptySpecification(PyObject *self, PyObject *args)
{
    PySILRequestObject *obj = (PySILRequestObject *)self;
    avtSILSpecification *request= obj->request;

    if(request->EmptySpecification())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


//
// Method Table
//

static struct PyMethodDef SILRequest_methods[] = {
    {"UseRestriction",     SILRequest_UseRestriction, METH_VARARGS},
    {"GetSIL",             SILRequest_GetSIL, METH_VARARGS},
    {"GetDataChunk",       SILRequest_GetDataChunk, METH_VARARGS},
    {"GetDomainList",      SILRequest_GetDomainList, METH_VARARGS},
    {"GetDomainList",      SILRequest_GetDomainList, METH_VARARGS},
    {"UsesAllData",        SILRequest_UsesAllData, METH_VARARGS},
    {"UsesAllDomains",     SILRequest_UsesAllDomains, METH_VARARGS},
    {"EmptySpecification", SILRequest_EmptySpecification, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

// ****************************************************************************
// Function: SILRequest_dealloc
//
// Purpose:
//   Destructor for PySILRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static void
SILRequest_dealloc(PyObject *v)
{
    // do nothing
    // avtSILSpecification lives as non heap member of avtDataRequest.
    // This is a little different than most other objects in the avt pipeline
    // which use ref pointers.
}


// ****************************************************************************
// Function: SILRequest_getattr
//
// Purpose:
//   Attribute fetch for PySILRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
SILRequest_getattr(PyObject *self, char *name)
{
    return Py_FindMethod(SILRequest_methods, self, name);
}



#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *SILRequest_Doc = "This class provides access to the avt pipeline SIL Request.";
#else
static char *SILRequest_Doc = "This class provides access to the avt pipeline SIL Request.";
#endif

//
// Note: avtSILSpecification provides a comparison which could be exposed.
//
//

//
// Python Type Struct Def Macro from Py2and3Support.h
//
//         VISIT_PY_TYPE_OBJ( VPY_TYPE,
//                            VPY_NAME,
//                            VPY_OBJECT,
//                            VPY_DEALLOC,
//                            VPY_PRINT,
//                            VPY_GETATTR,
//                            VPY_SETATTR,
//                            VPY_STR,
//                            VPY_PURPOSE,
//                            VPY_RICHCOMP,
//                            VPY_AS_NUMBER)

//
// The type description structure
//
VISIT_PY_TYPE_OBJ(PySILRequestType,      \
                  "SILRequest",          \
                  PySILRequestObject,    \
                  SILRequest_dealloc,    \
                  0,                     \
                  SILRequest_getattr,    \
                  0,                     \
                  0,                     \
                  SILRequest_Doc,        \
                  0,                     \
                  0); /* as_number*/

///////////////////////////////////////////////////////////////////////////////
//
// Interface used by Python Filter environment.
//
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Function: PySILRequest_Wrap
//
// Purpose:
//   Wrap avtSILSpecification into a python SILRequest object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

PyObject *
PySILRequest_Wrap(avtSILSpecification *request)
{
    PySILRequestObject *res;
    res = PyObject_NEW(PySILRequestObject, &PySILRequestType);
    if(res  == NULL)
        return NULL;
    res->request = request;
    return (PyObject *)res;
}

// ****************************************************************************
// Function: PySILRequest_Check
//
// Purpose:
//   Check if given python object is an instance of PySILRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

bool
PySILRequest_Check(PyObject *obj)
{
    return (obj->ob_type == &PySILRequestType);
}




