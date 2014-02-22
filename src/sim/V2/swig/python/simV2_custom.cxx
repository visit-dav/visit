/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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
#include <VisItInterfaceTypes_V2.h>
#include <VisItControlInterface_V2.h>
#include <VisIt_VariableData.h>
//#define SIMV2_NUMPY_DEBUG

extern "C" {

#include <Python.h>
#include <simV2_python_config.h>

#define START_WRAPPER(T, V) \
    PyObject *func = NULL, *func_data = NULL, *ret = NULL; \
    T h = V; \
    func = ((PyObject **)cbdata)[0]; \
    func_data = ((PyObject **)cbdata)[1]; \
    if(func_data == NULL) \
        func_data = Py_None; \
    if(PyErr_Occurred() == NULL && func != NULL) \
    {

#define END_WRAPPER(T)\
        if(ret != NULL) \
        { \
            if(PyInt_Check(ret))\
                h = (T)PyInt_AsLong(ret);\
            Py_DECREF(ret); \
        } \
    } \
    return h;

#if 1
#define DEBUG_PRINT(A) ;
#else
#define DEBUG_PRINT(A) A
#endif

#include <stdio.h>

/******************************************************************************/

static PyObject *pylibsim_setbroadcastintfunction_object = NULL;

int
pylibsim_setbroadcastintfunction(int *arg0, int arg1)
{
    int retval = VISIT_ERROR;
    PyObject *ret = NULL; 
    if(pylibsim_setbroadcastintfunction_object != NULL) 
    {
        int i;
        PyObject *tuple = NULL;
        i = (arg1 == 0) ? *arg0 : 0;
        tuple = PyTuple_New(2);
        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)i));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));

DEBUG_PRINT( printf("pylibsim_setbroadcastintfunction: calling Python with (%d, %d)\n", i, arg1); )
        ret = PyObject_Call(pylibsim_setbroadcastintfunction_object, tuple, NULL);

        Py_DECREF(tuple);
        if(ret != NULL)
        {
            if(PyInt_Check(ret))
            {
                /* Return the value in arg0. */
                *arg0 = (int)PyInt_AsLong(ret);
DEBUG_PRINT( printf("pylibsim_setbroadcastintfunction: callback returned: %d\n", *arg0); )
                retval = VISIT_OKAY;
            }

            Py_DECREF(ret);
        }
    }

    return retval;
}

void
pylibsim_setbroadcastintfunction_data(void *cbdata)
{
    PyObject *obj = (PyObject *)cbdata;
    if(pylibsim_setbroadcastintfunction_object != NULL)
        Py_DECREF(pylibsim_setbroadcastintfunction_object);

    pylibsim_setbroadcastintfunction_object = obj;
    Py_INCREF(obj);
}

/******************************************************************************/
static PyObject *pylibsim_setbroadcaststringfunction_object = NULL;

int  
pylibsim_setbroadcaststringfunction(char *arg0, int arg1, int arg2)
{
    int retval = VISIT_ERROR;
    PyObject *ret = NULL; 
    if(pylibsim_setbroadcaststringfunction_object != NULL) 
    {
        PyObject *tuple = PyTuple_New(3);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg2 == 0 ? arg0 : ""));
        PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)arg1));
        PyTuple_SET_ITEM(tuple, 2, PyInt_FromLong((long)arg2));

DEBUG_PRINT( printf("pylibsim_setbroadcaststringfunction: calling Python with (%s, %d, %d)\n", arg0, arg1, arg2); )

        ret = PyObject_Call(pylibsim_setbroadcaststringfunction_object, tuple, NULL);

        Py_DECREF(tuple);
        if(ret != NULL)
        {
            if(PyString_Check(ret))
            {
                /* Return the value in arg0. */
                strcpy(arg0, PyString_AsString(ret));
                retval = VISIT_OKAY;

DEBUG_PRINT( printf("pylibsim_setbroadcaststringfunction: callback returned: %s\n", arg0); )
            }

            Py_DECREF(ret);
        }
    }

    return retval;
}

void 
pylibsim_setbroadcaststringfunction_data(void *cbdata)
{
    PyObject *obj = (PyObject *)cbdata;
    if(pylibsim_setbroadcaststringfunction_object != NULL)
        Py_DECREF(pylibsim_setbroadcaststringfunction_object);

    pylibsim_setbroadcaststringfunction_object = obj;
    Py_INCREF(obj);
}

/******************************************************************************/
static PyObject *pylibsim_setslaveprocesscallback_object = NULL;

void 
pylibsim_setslaveprocesscallback(void)
{
    PyObject *ret = NULL; 
    if(pylibsim_setslaveprocesscallback_object != NULL) 
    {
        PyObject *tuple = PyTuple_New(0);

        ret = PyObject_Call(pylibsim_setslaveprocesscallback_object, tuple, NULL);

        Py_DECREF(tuple);
        if(ret != NULL)
            Py_DECREF(ret);
    }
}

void 
pylibsim_setslaveprocesscallback_data(void *cbdata)
{
    PyObject *obj = (PyObject *)cbdata;
    if(pylibsim_setslaveprocesscallback_object != NULL)
        Py_DECREF(pylibsim_setslaveprocesscallback_object);

    pylibsim_setslaveprocesscallback_object = obj;
    Py_INCREF(obj);
}

/******************************************************************************/

void 
pylibsim_void__pconstchar_pconstchar_pvoid(const char *arg0, const char *arg1, void *cbdata)
{
    PyObject *func = NULL, *func_data = NULL, *ret = NULL; 
    func = ((PyObject **)cbdata)[0]; 
    func_data = ((PyObject **)cbdata)[1];
    if(func_data == NULL)
        func_data = Py_None;
    if(PyErr_Occurred() == NULL && func != NULL) 
    {
        int rc = Py_REFCNT(func_data);
        PyObject *tuple = PyTuple_New(3);
        Py_INCREF(func_data);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString((arg0 != NULL) ? arg0 : ""));
        PyTuple_SET_ITEM(tuple, 1, PyString_FromString((arg1 != NULL) ? arg1 : ""));
        PyTuple_SET_ITEM(tuple, 2, func_data);

        ret = PyObject_Call(func, tuple, NULL);

        Py_DECREF(tuple);
        while(Py_REFCNT(func_data) < rc)
            Py_INCREF(func_data);
        if(ret != NULL)
            Py_DECREF(ret);
    }
}

/******************************************************************************/
visit_handle 
pylibsim_visit_handle__pvoid(void *cbdata)
{
    START_WRAPPER(visit_handle, VISIT_INVALID_HANDLE)
    {
        PyObject *tuple = PyTuple_New(1);
DEBUG_PRINT( printf("pylibsim_visit_handle__pvoid: 0: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )
        Py_INCREF(func_data);
DEBUG_PRINT( printf("pylibsim_visit_handle__pvoid: 1: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )
        PyTuple_SET_ITEM(tuple, 0, func_data);

        ret = PyObject_Call(func, tuple, NULL);
DEBUG_PRINT( printf("pylibsim_visit_handle__pvoid: 2: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )

        Py_DECREF(tuple);
DEBUG_PRINT( printf("pylibsim_visit_handle__pvoid: 3: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )
    }
    END_WRAPPER(visit_handle)
}

int 
pylibsim_int__pvoid(void *cbdata)
{
    START_WRAPPER(int, VISIT_INVALID_HANDLE)
    {
        PyObject *tuple = PyTuple_New(1);
        Py_INCREF(func_data);
        PyTuple_SET_ITEM(tuple, 0, func_data);

        ret = PyObject_Call(func, tuple, NULL);

        Py_DECREF(tuple);
    }
    END_WRAPPER(int)
}

/******************************************************************************/
visit_handle 
pylibsim_visit_handle__int_pconstchar_pvoid(int arg0, const char *arg1, void *cbdata)
{
    START_WRAPPER(visit_handle, VISIT_INVALID_HANDLE)
    {
        PyObject *tuple = PyTuple_New(3);
DEBUG_PRINT( printf("pylibsim_visit_handle__int_pconstchar_pvoid: 0: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )
        Py_INCREF(func_data);
DEBUG_PRINT( printf("pylibsim_visit_handle__int_pconstchar_pvoid: 1: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )
        PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)arg0));
        PyTuple_SET_ITEM(tuple, 1, PyString_FromString(arg1));
        PyTuple_SET_ITEM(tuple, 2, func_data);

        ret = PyObject_Call(func, tuple, NULL);
DEBUG_PRINT( printf("pylibsim_visit_handle__int_pconstchar_pvoid: 2: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )

        Py_DECREF(tuple);
DEBUG_PRINT( printf("pylibsim_visit_handle__int_pconstchar_pvoid: 3: func_data refct=%d\n", (int)Py_REFCNT(func_data)); )
    }
    END_WRAPPER(visit_handle)
}

/******************************************************************************/
visit_handle 
pylibsim_visit_handle__pconstchar_pvoid(const char *arg0, void *cbdata)
{
    START_WRAPPER(visit_handle, VISIT_INVALID_HANDLE)
    {
        PyObject *tuple = PyTuple_New(2);
        Py_INCREF(func_data);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg0));
        PyTuple_SET_ITEM(tuple, 1, func_data);

        ret = PyObject_Call(func, tuple, NULL);

        Py_DECREF(tuple);
    }
    END_WRAPPER(visit_handle)
}

int
pylibsim_int__pconstchar_pvoid(const char *arg0, void *cbdata)
{
    START_WRAPPER(int, VISIT_ERROR)
    {
        PyObject *tuple = PyTuple_New(2);
        Py_INCREF(func_data);
        PyTuple_SET_ITEM(tuple, 0, PyString_FromString(arg0));
        PyTuple_SET_ITEM(tuple, 1, func_data);

        ret = PyObject_Call(func, tuple, NULL);

        Py_DECREF(tuple);
    }
    END_WRAPPER(int)
}

/******************************************************************************/
static const char *getPyObjectType(PyObject *obj)
{
    const char *objType = "unknown";
    const char *tmp = NULL;
    PyObject *rep = NULL;
    if (obj && (rep = PyObject_Repr(obj)) && (tmp = PyString_AsString(rep)))
    {
        objType = tmp;
    }
    return objType;
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

/******************************************************************************/
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
            if (getSequenceSize(o, seqSize))
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


/******************************************************************************/
template <typename T>
int copyObject(PyObject *obj, T *&dptr)
{
    if (PyFloat_Check(obj))
    {
        *dptr = static_cast<T>(PyFloat_AsDouble(obj));
    }
    else
    if (PyInt_Check(obj))
    {
        *dptr = static_cast<T>(PyInt_AsLong(obj));
    }
    else
    if (PyLong_Check(obj))
    {
        *dptr = static_cast<T>(PyLong_AsLong(obj));
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "copyObject : Unsupported type.");
        return -1;
    }
    ++dptr;
    return 0;
}

/******************************************************************************/
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
            // deletion while in use by VisIt
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

extern "C"
{

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
void
pylibsim_VisItDisconnect(void)
{
    /* Call into the wrapped functions with arguments that tell them to
     * release their reference counted callback data. We call into the 
     * functions using the Python interpreter because we can't directly
     * call the _wrap functions due to their placement in the sources.
     */
    char cmd[500];
#define DISCONNECT_CALLBACK_EX(FUNC, ARGS) \
    sprintf(cmd,\
        "try:\n"\
        "    libsimV2.%s(%s)\n"\
        "except:\n"\
        "    try:\n"\
        "        %s(%s)\n"\
        "    except:\n"\
        "        pass\n",\
        #FUNC, ARGS, #FUNC, ARGS);\
    PyRun_SimpleString(cmd);

#define DISCONNECT_CALLBACK1(FUNC) DISCONNECT_CALLBACK_EX(FUNC, "None")
#define DISCONNECT_CALLBACK2(FUNC) DISCONNECT_CALLBACK_EX(FUNC, "None, None")

    /*
     * Note, we do not free the int and string broadcast functions since they are
     * registered for the life of libsim. If we freed then the sim would have to
     * register them when the sim reconnected. Sims typically only register the
     * broadcast functions on startup so let's not require the user to reregister
     * them.
     */

    /* Free these callbacks because they are associated with the engine handle,
     * which is freed later. Reconnecting will reset these.
     */
    DISCONNECT_CALLBACK1(VisItSetSlaveProcessCallback);
    DISCONNECT_CALLBACK2(VisItSetCommandCallback);

    /*
     * Free these data callbacks. Reconnecting will reset these.
     */
    DISCONNECT_CALLBACK2(VisItSetGetMetaData);
    DISCONNECT_CALLBACK2(VisItSetGetMesh);
    DISCONNECT_CALLBACK2(VisItSetGetMaterial);
    DISCONNECT_CALLBACK2(VisItSetGetSpecies);
    DISCONNECT_CALLBACK2(VisItSetGetVariable);
    DISCONNECT_CALLBACK2(VisItSetGetMixedVariable);
    DISCONNECT_CALLBACK2(VisItSetGetCurve);
    DISCONNECT_CALLBACK2(VisItSetGetDomainList);
    DISCONNECT_CALLBACK2(VisItSetGetDomainBoundaries);
    DISCONNECT_CALLBACK2(VisItSetGetDomainNesting);

    DISCONNECT_CALLBACK2(VisItSetWriteBegin);
    DISCONNECT_CALLBACK2(VisItSetWriteEnd);
    DISCONNECT_CALLBACK2(VisItSetWriteMesh);
    DISCONNECT_CALLBACK2(VisItSetWriteVariable);

    VisItDisconnect();
}

}
