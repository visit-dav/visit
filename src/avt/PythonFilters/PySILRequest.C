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

#include <PyDataRequest.h>
#include <Python.h>
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
// The type description structure
//

//
// Note: avtSILSpecification provides a comparison which could be exposed.
//
//

static PyTypeObject PySILRequestType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "SILRequest",                        // tp_name
    sizeof(PySILRequestObject),          // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)SILRequest_dealloc,      // tp_dealloc
    (printfunc)0,                        // tp_print
    (getattrfunc)SILRequest_getattr,     // tp_getattr
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
    SILRequest_Doc,                      // tp_doc
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




