// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Python.h>
#include <Py2and3Support.h>
#include <PyDataSelection.h>
#include <vector>

// ****************************************************************************
//  Modifications:
//
// ****************************************************************************
struct PyDataSelectionObject
{
    PyObject_HEAD
    // this is  a hack - I couldn't alloc a ref pointer to
    // an avtDataSelection, I suspect b/c it is an abstract class.
    // vector of ref_ptrs works fine (as used in other parts of visit)
    std::vector<avtDataSelection_p> selections;
};


//
// avtDataSelection methods
//


// ****************************************************************************
// Function: DataSelection_GetType
//
// Purpose:
//   Wrap avtDataSelection::GetType.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataSelection_GetType(PyObject *self, PyObject *args)
{
    PyDataSelectionObject *obj = (PyDataSelectionObject *)self;
    avtDataSelection_p selection = *(obj->selections[0]);

    return PyString_FromString(selection->GetType());
}


// Forward declearation for method table
static PyObject *DataSelection_dir(PyObject *self, PyObject *args);

//
// Method Table
//
static struct PyMethodDef PyDataSelection_methods[] = {
    {"__dir__",      DataSelection_dir, METH_NOARGS},
    {"GetType",      DataSelection_GetType, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//

static PyObject *
DataSelection_dir(PyObject *self, PyObject *args)
{
    PyObject *dir_list = PyList_New(0);
    if (!dir_list)
    {
        PyErr_NoMemory();
        return NULL;
    }
       
    // Add methods from the methods table
    for (PyMethodDef const *method = PyDataSelection_methods;
         method && method->ml_name;
         method++) {
        PyList_Append(dir_list, PyUnicode_FromString(method->ml_name));
    }

    return dir_list;
}   

// ****************************************************************************
// Function: PyDataSelection_getattro
//
// Purpose:
//   Attribute fetch for PyDataSelection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
PyDataSelection_getattro(PyObject *self, PyObject *attr_name)
{
    const char *name = PyUnicode_AsUTF8(attr_name);
    if (!name) return NULL;
    return Py_FindMethod(PyDataSelection_methods, self, (char*)name);
}

static char const *PyDataSelection_purpose = "This class provides access to the avt pipeline data selection base class.";

// Re-define tp slots for this custom object
#undef VISIT_PY_TYPE_OBJ_TP_SLOTS
#define VISIT_PY_TYPE_OBJ_TP_SLOTS(VSObjName)                                \
    Py##VSObjName##Type.tp_doc = Py##VSObjName##_purpose;                    \
    retval += ((void*) Py##VSObjName##Type.tp_doc != (void*)0);              \
    Py##VSObjName##Type.tp_getattro = Py##VSObjName##_getattro;              \
    retval += ((void*) Py##VSObjName##Type.tp_getattro != (void*)0);         \
    Py##VSObjName##Type.tp_methods = Py##VSObjName##_methods;                \
    retval += ((void*) Py##VSObjName##Type.tp_methods != (void*)0)
VISIT_PY_TYPE_OBJ(DataSelection);

///////////////////////////////////////////////////////////////////////////////
//
// Interface used by Python Filter environment.
//
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Function: PyDataSelection_Wrap
//
// Purpose:
//   Wrap avtDataSelection_p into a python DataSelection object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

PyObject *
PyDataSelection_Wrap(avtDataSelection_p sel)
{
    PyDataSelectionObject *res;
    return Py_None;
    res = PyObject_NEW(PyDataSelectionObject, &PyDataSelectionType);
    if(res  == NULL)
        return NULL;
    res->selections.push_back(sel);
    PyType_Ready(&PyDataSelectionType);
    return (PyObject *)res;
}

// ****************************************************************************
// Function: PyDataSelection_Check
//
// Purpose:
//   Check if given python object is an instance of PyDataSelection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

bool
PyDataSelection_Check(PyObject *obj)
{
    return (obj->ob_type == &PyDataSelectionType);
}


// ****************************************************************************
// Function: PyContract_WrapContract
//
// Purpose:
//   Obtain the avtDataSelection from a python data selection object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

avtDataSelection_p
PyDataSelection_FromPyObject(PyObject *obj)
{
    PyDataSelectionObject *py_sel = (PyDataSelectionObject *)obj;
    return *(py_sel->selections[0]);
}



