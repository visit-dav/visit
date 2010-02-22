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

#include <PyDataSelection.h>
#include <Python.h>
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
//   Wrap avtDataRequest::GetType.
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


//
// Method Table
//

static struct PyMethodDef DataSelection_methods[] = {
    {"GetType",      DataSelection_GetType, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//


// ****************************************************************************
// Function: DataSelection_dealloc
//
// Purpose:
//   Destructor for PyDataDataSelection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static void
DataSelection_dealloc(PyObject *v)
{
    // do nothing
}


// ****************************************************************************
// Function: DataSelection_getattr
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
DataSelection_getattr(PyObject *self, char *name)
{
    return Py_FindMethod(DataSelection_methods, self, name);
}



#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *DataSelection_Doc = "This class provides access to the avt pipeline data selection base class.";
#else
static char *DataSelection_Doc = "This class provides access to the avt pipeline data selection base class.";
#endif

//
// The type description structure
//

//
// Note: avtSILSpecification provides a comparison which could be exposed.
//
//

static PyTypeObject PyDataSelectionType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "DataSelection",                     // tp_name
    sizeof(PyDataSelectionObject),       // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)DataSelection_dealloc,   // tp_dealloc
    (printfunc)0,                        // tp_print
    (getattrfunc)DataSelection_getattr,  // tp_getattr
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
    DataSelection_Doc,                   // tp_doc
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



