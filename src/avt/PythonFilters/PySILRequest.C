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


// Forward declearation for method table
static PyObject *SILRequest_dir(PyObject *self, PyObject *args);

//
// Method Table
//

static struct PyMethodDef PySILRequest_methods[] = {
    {"__dir__",            SILRequest_dir, METH_NOARGS},
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

static PyObject *
SILRequest_dir(PyObject *self, PyObject *args)
{
    PyObject *dir_list = PyList_New(0);
    if (!dir_list)
    {
        PyErr_NoMemory();
        return NULL;
    }
       
    // Add methods from the methods table
    for (PyMethodDef const *method = PySILRequest_methods;
         method && method->ml_name;
         method++) {
        PyList_Append(dir_list, PyUnicode_FromString(method->ml_name));
    }

    return dir_list;
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
PySILRequest_getattro(PyObject *self, PyObject *attr_name)
{
    const char *name = PyUnicode_AsUTF8(attr_name);
    if (!name) return NULL;
    return Py_FindMethod(PySILRequest_methods, self, (char*)name);
}

static char const *PySILRequest_purpose = "This class provides access to the avt pipeline SIL Request.";

//
// Note: avtSILSpecification provides a comparison which could be exposed.
//
//

// Re-define tp slots for this custom object
#undef VISIT_PY_TYPE_OBJ_TP_SLOTS
#define VISIT_PY_TYPE_OBJ_TP_SLOTS(VSObjName)                                \
    Py##VSObjName##Type.tp_doc = Py##VSObjName##_purpose;                    \
    retval += ((void*) Py##VSObjName##Type.tp_doc != (void*)0);              \
    Py##VSObjName##Type.tp_getattro = Py##VSObjName##_getattro;              \
    retval += ((void*) Py##VSObjName##Type.tp_getattro != (void*)0);         \
    Py##VSObjName##Type.tp_methods = Py##VSObjName##_methods;                \
    retval += ((void*) Py##VSObjName##Type.tp_methods != (void*)0)
VISIT_PY_TYPE_OBJ(SILRequest);

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
    PyType_Ready(&PySILRequestType);
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




