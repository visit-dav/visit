/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include "simV2_custom.h"
#include <simV2_python_config.h>
#include <VisItControlInterface_V2.h>
#include <VisIt_VariableData.h>
#include <stdio.h>
#include <algorithm>
#include <functional>

// Also see the documentation in simV2_python.i
// which describes the pattern's we use for handling
// the python callbacks implemented here.

// helper
template<typename T> static T *del(T *obj){ delete obj; return NULL; }

// A container for callback objects that need to have
// their ref count decremented during a disconnect event
// or during shutdown.
class simV2_CallbackDataVector
{
public:
    simV2_CallbackDataVector() {}
    ~simV2_CallbackDataVector(){ Clear(); }
    // allocate a new callback data and add to the vector
    simV2_CallbackData *PushNewCallbackData(simV2_PyObject &callback, simV2_PyObject data)
    {
        simV2_CallbackData *cbd = new simV2_CallbackData(callback, data);
        Push(cbd);
        return cbd;
    }
    // add a callback
    void Push(simV2_CallbackData* data) { Data.push_back(data); }
    // delete all of the callback data.
    void Clear()
    {
        std::for_each(Data.begin(), Data.end(), std::ptr_fun(del<simV2_CallbackData>));
        Data.clear();
    }
private:
    std::deque<simV2_CallbackData*> Data;
private:
    void operator=(const simV2_CallbackDataVector&); // not implemented
    simV2_CallbackDataVector(const simV2_CallbackDataVector&); // not implemented
};

namespace {
// internal instance for all managed callback data
simV2_CallbackDataVector Callbacks;
simV2_CallbackDataVector CommCallbacks;

// internal API for releasing callback data
// this will be called during the disconnect event
void deleteCallbackData()
{
    Callbacks.Clear();
}

void deleteCommCallbackData()
{
    CommCallbacks.Clear();
}

};


// public API for creating new pair of callback and its data
// to be given to visit, when this data needs to be released
// in response to disconnect event
simV2_CallbackData *newCallbackData(simV2_PyObject &callback, simV2_PyObject data)
{
    return ::Callbacks.PushNewCallbackData(callback, data);
}
// public API for creating new pair of callback and its  data
// to be given to visit, when this data needs to be released
// during shutdown
simV2_CallbackData *newCommCallbackData(simV2_PyObject &callback, simV2_PyObject data)
{
    return ::CommCallbacks.PushNewCallbackData(callback, data);
}

/******************************************************************************
 * BroadcastInt callback invoker
 ******************************************************************************/
static simV2_PyObject broadcastIntCallback;

void pylibsim_setBroadcastIntCallback(PyObject *cb)
{ broadcastIntCallback.SetObject(cb); }

int pylibsim_invokeBroadcastIntCallback(int *arg0, int arg1)
{
    int retval = VISIT_ERROR;
    if (broadcastIntCallback)
    {
        int i = (arg1 == 0) ? *arg0 : 0;
        PyObject *tuple = PyTuple_New(2);
        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)i));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));

        PyObject *ret = PyObject_Call(broadcastIntCallback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                /* Return the value in arg0. */
                *arg0 = (int)PyInt_AsLong(ret);
                retval = VISIT_OKAY;
            }
            Py_DECREF(ret);
        }
    }
    return retval;
}

/******************************************************************************
 * BroadcastString callback invoker
 ******************************************************************************/
static simV2_PyObject broadcastStringCallback;

void pylibsim_setBroadcastStringCallback(PyObject *cb)
{ broadcastStringCallback.SetObject(cb); }

int
pylibsim_invokeBroadcastStringCallback(char *arg0, int arg1, int arg2)
{
    int retval = VISIT_ERROR;
    if (broadcastStringCallback)
    {
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg2 == 0 ? arg0 : ""));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));
        PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong((long)arg2));

        PyObject *ret = PyObject_Call(broadcastStringCallback, tuple, NULL);

        Py_DECREF(tuple);
        if (ret != NULL)
        {
            if(PyString_Check(ret))
            {
                /* Return the value in arg0. */
                strcpy(arg0, PyString_AsString(ret));
                retval = VISIT_OKAY;

            }
            Py_DECREF(ret);
        }
    }
    return retval;
}

/******************************************************************************
 * SlaveProcess callback invoker
 ******************************************************************************/
static simV2_PyObject slaveProcessCallback;

void pylibsim_setSlaveProcessCallback(PyObject *cb)
{ slaveProcessCallback.SetObject(cb); }

void pylibsim_invokeSlaveProcessCallback(void)
{
    if (slaveProcessCallback)
    {
        PyObject *tuple = PyTuple_New(0);

        PyObject *ret = PyObject_Call(slaveProcessCallback, tuple, NULL);

        Py_DECREF(tuple);

        if (ret)
        {
            Py_DECREF(ret);
        }
    }
}

/******************************************************************************
 * used by: BroadcastString2
 ******************************************************************************/
int
pylibsim_invoke_i_F_pcc_i_i_pv(char *arg0, int arg1, int arg2, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    int retval = VISIT_ERROR;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(4);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg2 == 0 ? arg0 : ""));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));
        PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong((long)arg2));
        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 3, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);
        if (ret != NULL)
        {
            if(PyString_Check(ret))
            {
                /* Return the value in arg0. */
                strcpy(arg0, PyString_AsString(ret));
                retval = VISIT_OKAY;

            }
            Py_DECREF(ret);
        }
    }

    return retval;
}

/******************************************************************************
 * used by: BroadcastInt2
 ******************************************************************************/
int pylibsim_invoke_i_F_pi_i_pv(int *arg0, int arg1, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    // invoke the user provided callback
    int retval = VISIT_ERROR;

    if (callback)
    {
        int i = (arg1 == 0) ? *arg0 : 0;
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)i));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));
        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 2, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                /* Return the value in arg0. */
                *arg0 = (int)PyInt_AsLong(ret);
                retval = VISIT_OKAY;
            }
            Py_DECREF(ret);
        }
    }
    return retval;
}

/******************************************************************************
 * used by: SlaveProcess2
 * used by: UI_clicked
 ******************************************************************************/
void pylibsim_invoke_v_F_pv(void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(1);

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 2, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if (ret)
        {
            Py_DECREF(ret);
        }
    }
}

/******************************************************************************
 * used by: CommandCallback
 ******************************************************************************/
void
pylibsim_invoke_v_F_pcc_pcc_pv(const char *arg0, const char *arg1, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString((arg0 != NULL) ? arg0 : ""));
        PyTuple_SET_ITEM(tuple, 1, PyString_FromString((arg1 != NULL) ? arg1 : ""));

        Py_INCREF(data); // set item steals a reference
        PyTuple_SET_ITEM(tuple, 2, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if (ret != NULL)
        {
            Py_DECREF(ret);
        }
    }
}

/******************************************************************************
 * used by: GetMetadata
 ******************************************************************************/
visit_handle pylibsim_invoke_h_F_pv(void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    visit_handle h = VISIT_INVALID_HANDLE;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(1);

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 0, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if (ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                h = static_cast<visit_handle>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }

    return h;
}

/******************************************************************************
 * used by: ActivateTimestep
 ******************************************************************************/
int pylibsim_invoke_i_F_pv(void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    int h = VISIT_INVALID_HANDLE;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(1);

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 0, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if (ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                h = static_cast<int>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }

    return h;
}

/******************************************************************************
 * used by: GetMesh GetMaterial GetSpecies GetVariable GetMixedVariable
 ******************************************************************************/
visit_handle pylibsim_invoke_h_F_i_pcc_pv(int arg0, const char *arg1, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    visit_handle h = VISIT_INVALID_HANDLE;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(3);

        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)arg0));
        PyTuple_SET_ITEM(tuple, 1, PyString_FromString(arg1));

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 2, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                h = static_cast<visit_handle>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }
    return h;
}


/******************************************************************************
 * used by: GetCurve GetDomainList GetDomainBoundaries GetDomainNesting
 ******************************************************************************/
visit_handle pylibsim_invoke_h_F_pcc_pv(const char *arg0, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    visit_handle h = VISIT_INVALID_HANDLE;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(2);

        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg0));

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 1, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                h = static_cast<visit_handle>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }

    return h;
}

/******************************************************************************
 * used by : WriteBegin, WriteEnd
 ******************************************************************************/
int pylibsim_invoke_i_F_pcc_pv(const char *arg0, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    int ierr = VISIT_ERROR;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(2);

        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg0));

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 1, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                ierr = static_cast<int>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }

    return ierr;
}

/******************************************************************************
 * used by WriteMesh
 ******************************************************************************/
int pylibsim_invoke_i_F_pcc_i_i_h_h_pv(
    const char *arg0, int arg1, int arg2, visit_handle arg3, visit_handle arg4,
    void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    int ierr = VISIT_ERROR;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(6);

        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg0));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));
        PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong((long)arg2));
        PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong((long)arg3));
        PyTuple_SET_ITEM(tuple, 4, PyInt_FromLong((long)arg4));

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 5, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                ierr = static_cast<int>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }

    return ierr;
}

/******************************************************************************
 * used by: WriteVariable
 ******************************************************************************/
int pylibsim_invoke_i_F_pcc_pcc_i_h_h_pv(
    const char *arg0, const char *arg1, int arg2, visit_handle arg3, visit_handle arg4,
    void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    int ierr = VISIT_ERROR;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(6);

        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg0));
        PyTuple_SET_ITEM(tuple, 1, PyString_FromString(arg1));
        PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong((long)arg2));
        PyTuple_SET_ITEM(tuple, 3, PyInt_FromLong((long)arg3));
        PyTuple_SET_ITEM(tuple, 4, PyInt_FromLong((long)arg4));

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 5, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                ierr = static_cast<int>(PyInt_AsLong(ret));
            }
            Py_DECREF(ret);
        }
    }

    return ierr;
}


/******************************************************************************
 * used by: UI_stateChanged, UI_valueChanged
 ******************************************************************************/
void pylibsim_invoke_v_F_i_pv(int arg0, void *cbdata)
{
    /* a callback and its data */
    simV2_CallbackData *cbpair = static_cast<simV2_CallbackData*>(cbdata);
    simV2_PyObject &callback = cbpair->first;
    simV2_PyObject &data = cbpair->second;

    if (callback)
    {
        PyObject *tuple = PyTuple_New(1);

        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)arg0));

        Py_INCREF(data); // SET_ITEM steals a ref
        PyTuple_SET_ITEM(tuple, 1, data);

        PyObject *ret = PyObject_Call(callback, tuple, NULL);

        Py_DECREF(tuple);

        if(ret != NULL)
        {
            Py_DECREF(ret);
        }
    }
}

namespace pylibsim {

#if defined(SIMV2_USE_NUMPY)
template<typename T> class CppToNumpy {};
template<> class CppToNumpy<float> { public: enum { Type = NPY_FLOAT }; };
template<> class CppToNumpy<double> { public: enum { Type = NPY_DOUBLE }; };
template<> class CppToNumpy<int> { public: enum { Type = NPY_INT }; };
template<> class CppToNumpy<char> { public: enum { Type = NPY_BYTE }; };

template<int C> class NumpyToCpp {};
template<> class NumpyToCpp<NPY_FLOAT> { public: typedef float Type; };
template<> class NumpyToCpp<NPY_DOUBLE> { public: typedef double Type; };
template<> class NumpyToCpp<NPY_INT> { public: typedef int Type; };
template<> class NumpyToCpp<NPY_BYTE> { public: typedef char Type; };

/******************************************************************************/
template <typename T>
int getArrayPointer(
      int nComps,
      int nTuples,
      PyObject *seq,
      T *&data)
{
    // make some sanity checks that will prevent impropper
    // use of the pointer.
    if (!PyArray_Check(seq))
    {
        // not a numpy array
        return -1;
    }

    PyArrayObject *ndarray = reinterpret_cast<PyArrayObject*>(seq);

    if (PyArray_TYPE(ndarray) != CppToNumpy<T>::Type)
    {
        // internal datatype doesn't match X in setDataX call
        return -2;
    }

    if (PyArray_SIZE(ndarray) != (nComps*nTuples))
    {
        // size is not as reported by the user
        return -3;
    }

    if (!(PyArray_IS_C_CONTIGUOUS(ndarray) || PyArray_IS_F_CONTIGUOUS(ndarray)))
    {
        // the array is not contiguous
        return -4;
    }
#if defined(SIMV2_NUMPY_DEBUG)
    fprintf(stderr, "successfully got a point to the numpy array\n");
#endif
    // success, safe to use the pointer
    data = static_cast<T*>(PyArray_DATA(ndarray));
    return 0;
}

/******************************************************************************/
template<typename T, int C>
T numpyIterDeref(PyArrayObject *it)
{
    return
    static_cast<T>(*(static_cast<typename NumpyToCpp<C>::Type *>(PyArray_ITER_DATA(it))));
}

/******************************************************************************/
template<typename T, int C>
void numpyIterCopy(PyArrayObject *it, T *dptr)
{
    while (PyArray_ITER_NOTDONE(it))
    {
        *dptr = numpyIterDeref<T, CppToNumpy<T>::Type>(it);
        PyArray_ITER_NEXT(it);
        ++dptr;
    }
}

/******************************************************************************/
template <typename T>
int copyArray(PyArrayObject *ndarray, T *dptr)
{
    int ierr = 0;
    PyArrayObject *it = reinterpret_cast<PyArrayObject*>(
            PyArray_IterNew(reinterpret_cast<PyObject*>(ndarray)));
    PyArray_ITER_RESET(it);
    switch (PyArray_TYPE(ndarray))
    {
    case NPY_FLOAT:
        numpyIterCopy<T, NPY_FLOAT>(it, dptr);
        break;

    case NPY_DOUBLE:
        numpyIterCopy<T, NPY_DOUBLE>(it, dptr);
        break;

    case NPY_INT:
        numpyIterCopy<T, NPY_INT>(it, dptr);
        break;

    case NPY_BYTE:
        numpyIterCopy<T, NPY_BYTE>(it, dptr);
        break;

    default:
        ierr = -1;
        PyErr_SetString(PyExc_TypeError, "copyArray : Unsupported type");
    }
    Py_DECREF(it);
    return ierr;
}
#endif // end of numpy support

//******************************************************************************
// Modifications:
//   Kathleen Biagas, Fri Jun  6 11:08:09 PDT 2014
//   Add support for string object.
//
//******************************************************************************
static int getSequenceSize(PyObject *obj, Py_ssize_t *seqSize)
{
    PyObject *seq = PySequence_Fast(obj, "getSequenceSize : Not a sequence");
    if (!seq)
    {
        return -1;
    }
    Py_ssize_t n = PySequence_Size(seq);
    for (Py_ssize_t i=0; i<n; ++i)
    {
        PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
        if (PySequence_Check(o))
        {
            if (PyString_Check(o))
            {
                *seqSize += PyString_Size(o);
            }
            else if (getSequenceSize(o, seqSize))
            {
                return -2;
            }
        }
        else
        if (PyFloat_Check(o) || PyInt_Check(o) || PyLong_Check(o))
        {
            ++(*seqSize);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "getSequenceSize : Unsupported type");
            return -3;
        }
    }
    return 0;
}


//******************************************************************************
// Modifications:
//   Kathleen Biagas, Fri Jun  6 11:08:09 PDT 2014
//   Add support for string object.
//
//******************************************************************************
template <typename T>
int copyObject(PyObject *obj, T *&dptr)
{
    if (PyFloat_Check(obj))
    {
        *dptr = static_cast<T>(PyFloat_AsDouble(obj));
        ++dptr;
    }
    else
    if (PyInt_Check(obj))
    {
        *dptr = static_cast<T>(PyInt_AsLong(obj));
        ++dptr;
    }
    else
    if (PyLong_Check(obj))
    {
        *dptr = static_cast<T>(PyLong_AsLong(obj));
        ++dptr;
    }
    else
    if (PyString_Check(obj))
    {
        Py_ssize_t n = PyString_Size(obj);
        char *asChar = PyString_AsString(obj);
        for (Py_ssize_t i = 0; i < n; ++i)
        {
            *dptr = asChar[i];
            ++dptr;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "copyObject : Unsupported type.");
        return -1;
    }
    return 0;
}

//******************************************************************************
// Modifications:
//   Kathleen Biagas, Fri Jun  6 11:08:09 PDT 2014
//   Add support for string object.
//
//******************************************************************************
template <typename T>
int copySequence(PyObject *obj, T *&dptr)
{
    PyObject *seq = PySequence_Fast(obj, "copySequence : Not a sequence");
    if (!seq)
    {
        return -1;
    }
    Py_ssize_t n = PySequence_Size(seq);
    for (Py_ssize_t i=0; i<n; ++i)
    {
        PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
        if (PySequence_Check(o))
        {
            if (PyString_Check(o))
            {
                if (copyObject(o, dptr))
                {
                    return -3;
                }
            }
            else
            if (copySequence(o, dptr))
            {
                return -2;
            }
        }
        else
        if (copyObject(o, dptr))
        {
            return -3;
        }
    }
    return 0;
}

/******************************************************************************/
template <typename T>
int copy(
      int nComps,
      int nTuples,
      PyObject *seq,
      T *&data)
{
    long long dataSize = nComps*nTuples;
    data = (T *)malloc(sizeof(T)*dataSize);
    if (data == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "copy : Malloc failed.");
        return -1;
    }
    T *pData = data;

#if defined(SIMV2_USE_NUMPY)
    if (PyArray_Check(seq))
    {
        PyArrayObject *ndarray = reinterpret_cast<PyArrayObject*>(seq);
        if (PyArray_SIZE(ndarray) != dataSize)
        {
            free(data);
            PyErr_SetString(PyExc_RuntimeError, "copy : Array size not nComps*nTuples");
            return -2;
        }

        if (copyArray(ndarray, pData))
        {
            free(data);
            PyErr_SetString(PyExc_RuntimeError, "copy : Failed to the copy array");
            return -3;
        }
#if defined(SIMV2_NUMPY_DEBUG)
        fprintf(stderr, "Successfully coppied numpy array\n");
#endif
    }
    else
#endif
    if (PySequence_Check(seq))
    {
        Py_ssize_t flatSize = 0;
        if (getSequenceSize(seq, &flatSize))
        {
            free(data);
            PyErr_SetString(PyExc_RuntimeError, "copy : Failed to get the sequence size");
            return -4;
        }

        if (flatSize != dataSize)
        {
            free(data);
            PyErr_SetString(PyExc_RuntimeError, "copy : Sequence size not nComps*nTuples");
            return -5;
        }

        if (copySequence(seq, pData))
        {
            free(data);
            PyErr_SetString(PyExc_RuntimeError, "copy : Failed to the copy sequence");
            return -6;
        }
#if defined(SIMV2_NUMPY_DEBUG)
        fprintf(stderr, "Successfully coppied python sequence\n");
#endif
    }
    else
    {
        free(data);
        PyErr_SetString(PyExc_TypeError, "copy : Not a sequence or array");
        return -7;
    }

    // success
    return 0;
}

/******************************************************************************/
template <typename T>
int getData(
          int &owner, // in/out
          int nComps,
          int nTuples,
          PyObject *&seq,
          T *&data) // out
{
    data = NULL;
#if defined(SIMV2_USE_NUMPY)
    if (owner == VISIT_OWNER_COPY)
    {
        // always honor OWNER_COPY, it's the simulation's
        // way of informing us that the data will disapear
        // and we must copy it
        if (copy<T>(nComps, nTuples, seq, data))
        {
            return -1;
        }
        // VisIt manages the copy with malloc/free
        owner = VISIT_OWNER_VISIT;
    }
    else
    {
        // attempt zero-copy, if we can't manage it safely then
        // fallback and make a copy of the data. In order to be
        // able to use the pointer all of the following must be
        // true:
        //     1) the object must be a numpy ndarry array
        //     2) the ndarray's type type must match X in the setDataX call
        //     3) the ndarray's size must match nTups*nComps
        //     4) the ndarray must have a contiguous internal represntation
        // if any of those are false a copy is made below.
        if (!getArrayPointer<T>(nComps, nTuples, seq, data))
        {
            // take a reference to the object preventing
            // deletion while in use by VisIt. Note that
            // holding a reference does not ensure that
            // the simulation won't modify the data while
            // we still need it.
#if defined(SIMV2_NUMPY_DEBUG)
            fprintf(stderr, "Py_INCREF(%p)\n", seq);
#endif
            owner = VISIT_OWNER_VISIT_EX;
            Py_INCREF(seq);
        }
        else
        if (!copy<T>(nComps, nTuples, seq, data))
        {
            // VisIt manages the copy
            owner = VISIT_OWNER_VISIT;
        }
        else
        {
            return -2;
        }
    }
#else
    // without numpy things are very simple: always make
    // a copy of the data
    if (copy<T>(nComps, nTuples, seq, data))
    {
        return -3;
    }
    // VisIt manages the copy
    owner = VISIT_OWNER_VISIT;
#endif
    // success
    return 0;
}

/******************************************************************************/
void initialize()
{
#if defined(SIMV2_USE_NUMPY)
    static bool initialized = false;
    if (!initialized)
    {
        import_array();
        initialized = true;
    }
#endif
}

/******************************************************************************/
void pyarray_destructor(void *object)
{
#if defined(SIMV2_NUMPY_DEBUG)
  fprintf(stderr,"Py_DECREF(%p)\n", object);
#endif
  PyObject *pyobj = static_cast<PyObject*>(object);
  // release our reference to the object
  //Py_CLEAR(pyobj);
  Py_DECREF(pyobj);
}

/******************************************************************************/
int setData(visit_handle obj,
      int owner,
      int dataType,
      int nComps,
      int nTuples,
      PyObject *seq,
      void *data)
{
    if (owner == VISIT_OWNER_VISIT_EX)
    {
        return VisIt_VariableData_setDataEx(obj, owner, dataType, nComps,
                                 nTuples, data, pyarray_destructor, seq);
    }
    return VisIt_VariableData_setData(
        obj, owner, dataType, nComps, nTuples, data);
}

};

/******************************************************************************/
int pylibsim_VisIt_VariableData_setDataAsD(
          visit_handle obj,
          int owner,
          int nComps,
          int nTuples,
          PyObject *seq)
{
    pylibsim::initialize();
    double *data = NULL;
    if (pylibsim::getData<double>(owner, nComps, nTuples, seq, data))
    {
        return VISIT_ERROR;
    }
    return pylibsim::setData(
        obj, owner, VISIT_DATATYPE_DOUBLE, nComps, nTuples, seq, data);
}

/******************************************************************************/
int pylibsim_VisIt_VariableData_setDataAsF(
          visit_handle obj,
          int owner,
          int nComps,
          int nTuples,
          PyObject *seq)
{
    pylibsim::initialize();
    float *data = NULL;
    if (pylibsim::getData<float>(owner, nComps, nTuples, seq, data))
    {
        return VISIT_ERROR;
    }
    return pylibsim::setData(
        obj, owner, VISIT_DATATYPE_FLOAT, nComps, nTuples, seq, data);
}

/******************************************************************************/
int pylibsim_VisIt_VariableData_setDataAsI(
          visit_handle obj,
          int owner,
          int nComps,
          int nTuples,
          PyObject *seq)
{
    pylibsim::initialize();
    int *data = NULL;
    if (pylibsim::getData<int>(owner, nComps, nTuples, seq, data))
    {
        return VISIT_ERROR;
    }
    return pylibsim::setData(
        obj, owner, VISIT_DATATYPE_INT, nComps, nTuples, seq, data);
}

/******************************************************************************/
int pylibsim_VisIt_VariableData_setDataAsC(
          visit_handle obj,
          int owner,
          int nComps,
          int nTuples,
          PyObject *seq)
{
    pylibsim::initialize();
    char *data = NULL;
    if (pylibsim::getData<char>(owner, nComps, nTuples, seq, data))
    {
        return VISIT_ERROR;
    }
    return pylibsim::setData(
        obj, owner, VISIT_DATATYPE_CHAR, nComps, nTuples, seq, data);
}

/******************************************************************************/
void pylibsim_VisItDisconnect(void)
{
    pylibsim_setSlaveProcessCallback(NULL);
    deleteCallbackData();
    VisItDisconnect();
}

/******************************************************************************/
void pylibsim_VisItFinalize(void)
{
    pylibsim_setBroadcastIntCallback(NULL);
    pylibsim_setBroadcastStringCallback(NULL);
    pylibsim_setSlaveProcessCallback(NULL);
    deleteCallbackData();
    deleteCommCallbackData();
}
