/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <Python.h>
#include <PyContract.h>
#include <PyDataRequest.h>


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
//
// ****************************************************************************
static PyObject *
Contract_UseLoadBalancing(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
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
//
// ****************************************************************************
static PyObject *
Contract_SetHaveRectilinearMeshOptimizations(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
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
//
// ****************************************************************************
static PyObject *
Contract_SetHaveCurvilinearMeshOptimizations(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
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
//
// ****************************************************************************
static PyObject *
Contract_SetOnDemandStreaming(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
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
//
// ****************************************************************************
static PyObject *
Contract_SetDataRequest(PyObject *self, PyObject *args)
{
    PyContractObject *obj = (PyContractObject *)self;
    avtContract_p contract= *(obj->contract);

    // get python data request ...
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
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


//
// Method Table
//


static struct PyMethodDef Contract_methods[] = {
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
Contract_dealloc(PyObject *v)
{
    // Contract is  stored in a a ref ptr, so it will clean itself up.
    // but ?
}


// ****************************************************************************
// Function: Contract_getattr
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
Contract_getattr(PyObject *self, char *name)
{
    return Py_FindMethod(Contract_methods, self, name);
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
static int
Contract_print(PyObject *v, FILE *fp, int flags)
{
    PyContractObject *obj = (PyContractObject *)v;
    avtContract_p contract= *(obj->contract);
    contract->Print(cout);
    return 0;
}

#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *Contract_Doc = "This class provides access to the avt pipeline contract.";
#else
static char *Contract_Doc = "This class provides access to the avt pipeline contract";
#endif

//
// The type description structure
//
static PyTypeObject PyContractType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "Contract",                          // tp_name
    sizeof(PyContractObject),           // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)Contract_dealloc,        // tp_dealloc
    (printfunc)Contract_print,          // tp_print
    (getattrfunc)Contract_getattr,       // tp_getattr
    (setattrfunc)0,                      // tp_setattr
    (cmpfunc)0,                          // tp_compare
    (reprfunc)0,                         // tp_repr
    //
    // Type Categories
    //
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    //
    // More methods
    //
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_CHECKTYPES,               // tp_flags
    Contract_Doc,                        // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0                                    // tp_weaklistoffset
};


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


