// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Python.h>
#include <Py2and3Support.h>
#include <PyContract.h>
#include <PyDataRequest.h>

#include <sstream>
#include <string>

// ****************************************************************************
//  Modifications:
//
// ****************************************************************************
struct PyContractObject
{
    PyObject_HEAD
    avtContract_p *contract;
};

//
// Internal prototypes
//

//
// avtContract methods
//

// ****************************************************************************
// Function: Contract_ShouldUseStreaming
//
// Purpose:
//   Wrap avtContract::ShouldUseStreaming.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_ShouldUseStreaming(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    if(contract->ShouldUseStreaming())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: Contract_NoStreaming
//
// Purpose:
//   Wrap avtContract::NoStreaming.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_NoStreaming(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    contract->NoStreaming();
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: Contract_ShouldUseLoadBalancing
//
// Purpose:
//   Wrap avtContract::ShouldUseLoadBalancing.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_ShouldUseLoadBalancing(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    if(contract->ShouldUseLoadBalancing())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


// ****************************************************************************
// Function: Contract_UseLoadBalancing
//
// Purpose:
//   Wrap avtContract::UseLoadBalancing.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//  Cyrus Harrison, Mon Jun 14 10:34:27 PDT 2010
//  Use 'O' instead of 'o'.
//
// ****************************************************************************
static PyObject *
Contract_UseLoadBalancing(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "O", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val = true;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    contract->UseLoadBalancing(val);
    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: Contract_SetHaveRectilinearMeshOptimizations
//
// Purpose:
//   Wrap avtContract::SetHaveRectilinearMeshOptimizations.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//  Cyrus Harrison, Mon Jun 14 10:34:27 PDT 2010
//  Use 'O' instead of 'o'.
//
// ****************************************************************************
static PyObject *
Contract_SetHaveRectilinearMeshOptimizations(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "O", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val = true;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    contract->SetHaveRectilinearMeshOptimizations(val);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: Contract_GetHaveRectilinearMeshOptimizations
//
// Purpose:
//   Wrap avtContract::GetHaveRectilinearMeshOptimizations.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_GetHaveRectilinearMeshOptimizations(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    if(contract->GetHaveRectilinearMeshOptimizations())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}



// ****************************************************************************
// Function: Contract_SetHaveCurvilinearMeshOptimizations
//
// Purpose:
//   Wrap avtContract::SetHaveCurvilinearMeshOptimizations.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//  Cyrus Harrison, Mon Jun 14 10:34:27 PDT 2010
//  Use 'O' instead of 'o'.
//
// ****************************************************************************
static PyObject *
Contract_SetHaveCurvilinearMeshOptimizations(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "O", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val = true;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    contract->SetHaveCurvilinearMeshOptimizations(val);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: Contract_GetHaveCurvilinearMeshOptimizations
//
// Purpose:
//   Wrap avtContract::GetHaveCurvilinearMeshOptimizations.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_GetHaveCurvilinearMeshOptimizations(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    if(contract->GetHaveCurvilinearMeshOptimizations())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: Contract_DoingOnDemandStreaming
//
// Purpose:
//   Wrap avtContract::DoingOnDemandStreaming.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_DoingOnDemandStreaming(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    if(contract->DoingOnDemandStreaming())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: Contract_SetOnDemandStreaming
//
// Purpose:
//   Wrap avtContract::SetOnDemandStreaming.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//  Cyrus Harrison, Mon Jun 14 10:34:27 PDT 2010
//  Use 'O' instead of 'o'.
//
// ****************************************************************************
static PyObject *
Contract_SetOnDemandStreaming(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "O", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val = true;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    contract->SetOnDemandStreaming(val);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: Contract_SetDataRequest
//
// Purpose:
//   Wrap avtContract::SetDataRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//  Cyrus Harrison, Mon Jun 14 10:34:27 PDT 2010
//  Use 'O' instead of 'o'.
//
// ****************************************************************************
static PyObject *
Contract_SetDataRequest(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // get python data request ...
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "O", &py_val))
        return NULL;

    if(!PyDataRequest_Check(py_val))
        return NULL;

    avtDataRequest_p request = PyDataRequest_FromPyObject(py_val);
    contract->SetDataRequest(request);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: Contract_GetDataRequest
//
// Purpose:
//   Wrap avtContract::GetDataRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
Contract_GetDataRequest(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    avtDataRequest_p req = contract->GetDataRequest();
    return PyDataRequest_Wrap(req);
}

// Forward declaration for methods table
static PyObject *Contract_dir(PyObject *self, PyObject *args);

//
// Method Table
//
static PyMethodDef PyContract_methods[] = {
    {"__dir__",                                 Contract_dir, METH_NOARGS},
    {"ShouldUseStreaming",                      Contract_ShouldUseStreaming, METH_VARARGS},
    {"NoStreaming",                             Contract_NoStreaming, METH_VARARGS},
    {"ShouldUseLoadBalancing",                  Contract_ShouldUseLoadBalancing, METH_VARARGS},
    {"UseLoadBalancing",                        Contract_UseLoadBalancing, METH_VARARGS},
    {"SetHaveRectilinearMeshOptimizations",     Contract_SetHaveRectilinearMeshOptimizations, METH_VARARGS},
    {"GetHaveRectilinearMeshOptimizations",     Contract_GetHaveRectilinearMeshOptimizations, METH_VARARGS},
    {"SetHaveCurvilinearMeshOptimizations",     Contract_SetHaveCurvilinearMeshOptimizations, METH_VARARGS},
    {"GetHaveCurvilinearMeshOptimizations",     Contract_GetHaveCurvilinearMeshOptimizations, METH_VARARGS},
    {"DoingOnDemandStreaming",                  Contract_DoingOnDemandStreaming, METH_VARARGS},
    {"SetOnDemandStreaming",                    Contract_SetOnDemandStreaming, METH_VARARGS},
    {"SetDataRequest",                          Contract_SetDataRequest, METH_VARARGS},
    {"GetDataRequest",                          Contract_GetDataRequest, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//
static PyObject *
Contract_dir(PyObject *self, PyObject *args)
{
    PyObject *dir_list = PyList_New(0);
    if (!dir_list)
    {
        PyErr_NoMemory();
        return NULL;
    }
       
    // Add methods from the methods table
    for (PyMethodDef const *method = PyContract_methods;
         method && method->ml_name;
         method++) {
        PyList_Append(dir_list, PyUnicode_FromString(method->ml_name));
    }

    return dir_list;
}   


// ****************************************************************************
// Function: Contract_dealloc
//
// Purpose:
//   Destructor for PyContract.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static void
PyContract_dealloc(PyObject *v)
{
    // Contract is  stored in a a ref ptr, so it will clean itself up.
    // but ?
}


// ****************************************************************************
// Function: Contract_getattro
//
// Purpose:
//   Attribute fetch for PySILContract.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

static PyObject *
PyContract_getattro(PyObject *self, PyObject *attr_name)
{
    const char *name = PyUnicode_AsUTF8(attr_name);
    if (!name) return NULL;
    return Py_FindMethod(PyContract_methods, self, (char*)name);
}

// ****************************************************************************
// Function: Contract_print
//
// Purpose:
//   Print method for PyContract.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
PyContract_str(PyObject *v)
{
    PyContractObject *obj = (PyContractObject *)v;
    avtContract_p contract= *(obj->contract);
    std::ostringstream oss;
    contract->Print(oss);
    std::string result = oss.str();
    return PyUnicode_FromString(result.c_str());
}

static char const *PyContract_purpose = "This class provides access to the avt pipeline contract";

// Re-define slot definitions for this custom object
#undef VISIT_PY_TYPE_OBJ_TP_SLOTS
#define VISIT_PY_TYPE_OBJ_TP_SLOTS(VSObjName)                                \
    Py##VSObjName##Type.tp_doc = Py##VSObjName##_purpose;                    \
    retval += ((void*) Py##VSObjName##Type.tp_doc != (void*)0);              \
    Py##VSObjName##Type.tp_dealloc = Py##VSObjName##_dealloc;                \
    retval += ((void*) Py##VSObjName##Type.tp_dealloc != (void*)0);          \
    Py##VSObjName##Type.tp_str = Py##VSObjName##_str;                        \
    retval += ((void*) Py##VSObjName##Type.tp_str != (void*)0);              \
    Py##VSObjName##Type.tp_getattro = Py##VSObjName##_getattro;              \
    retval += ((void*) Py##VSObjName##Type.tp_getattro != (void*)0);         \
    Py##VSObjName##Type.tp_methods = Py##VSObjName##_methods;                \
    retval += ((void*) Py##VSObjName##Type.tp_methods != (void*)0)
VISIT_PY_TYPE_OBJ(Contract);

///////////////////////////////////////////////////////////////////////////////
//
// Interface used by Python Filter environment.
//
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Function: PyContract_Wrap
//
// Purpose:
//   Create wrap avtContract_p into a python contract object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

PyObject *
PyContract_Wrap(avtContract_p contract)
{
    PyContractObject *res;
    res = PyObject_NEW(PyContractObject, &PyContractType);
    if(res  == NULL)
        return NULL;
    res->contract = new avtContract_p;
    // set the contract
    *(res->contract) = contract;
    return (PyObject *)res;
}

// ****************************************************************************
// Function: PyContract_Check
//
// Purpose:
//   Check if given python object is an instance of PyContract.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

bool
PyContract_Check(PyObject *obj)
{
    return (obj->ob_type == &PyContractType);
}



// ****************************************************************************
// Function: PyContract_FromPyObject
//
// Purpose:
//   Obtain the avtContract from a python contract object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

avtContract_p
PyContract_FromPyObject(PyObject *obj)
{
    PyContractObject *py_contract = (PyContractObject *)obj;
    return *(py_contract->contract);
}


