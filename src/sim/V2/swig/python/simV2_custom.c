/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#include <Python.h>
#include <VisItInterfaceTypes_V2.h>

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
static int
pylibsim_VisIt_VariableData_setDataEx(visit_handle obj, int nComps, int nTuples, 
    void *seqv, double *d)
{
    int t, c, isTuple = 0;
    double *dptr = NULL;
    PyObject *seq = (PyObject *)seqv; /* The pointer was really to a PyObject. */
    if(PyTuple_Check(seq))
        isTuple = 1;
    else if(!PyList_Check(seq)) 
        return VISIT_ERROR;

    dptr = d;
    for(t = 0; t < nTuples; ++t)
    {
        PyObject *item = NULL;
        if(isTuple)
            item = PyTuple_GetItem(seq, t);
        else
            item = PyList_GetItem(seq, t);

        if(item == NULL)
        {
            return VISIT_ERROR;
        }
        else if(PyTuple_Check(item))
        {
            for(c = 0; c < nComps; ++c)
            {
                if(c < PyTuple_Size(item))
                {
                    PyObject *item2 = PyTuple_GET_ITEM(item, c);
                    if(PyFloat_Check(item))
                        *dptr++ = PyFloat_AsDouble(item);
                    else if(PyInt_Check(item))
                        *dptr++ = (double)PyInt_AsLong(item);
                    else if(PyLong_Check(item))
                        *dptr++ = (double)PyLong_AsLong(item);
                    else
                    {
                        return VISIT_ERROR;
                    } 
                }
                else
                    *dptr++ = 0.;
            }
        }
        else if(PyList_Check(item))
        {
            for(c = 0; c < nComps; ++c)
            {
                if(c < PyList_Size(item))
                {
                    PyObject *item2 = PyList_GET_ITEM(item, c);
                    if(PyFloat_Check(item))
                        *dptr++ = PyFloat_AsDouble(item);
                    else if(PyInt_Check(item))
                        *dptr++ = (double)PyInt_AsLong(item);
                    else if(PyLong_Check(item))
                        *dptr++ = (double)PyLong_AsLong(item);
                    else
                    {
                        return VISIT_ERROR;
                    } 
                }
                else
                    *dptr++ = 0.;
            }
        }
        else if(PyFloat_Check(item))
        {
            double val = PyFloat_AsDouble(item);
            for(c = 0; c < nComps; ++c)
            {
                *dptr++ = val;
            }
        }
        else if(PyInt_Check(item))
        {
            double val = (double)PyInt_AsLong(item);
            for(c = 0; c < nComps; ++c)
            {
                *dptr++ = val;
            }
        }
        else if(PyLong_Check(item))
        {
            double val = (double)PyLong_AsLong(item);
            for(c = 0; c < nComps; ++c)
            {
                *dptr++ = val;
            }
        }
        else
        {
            return VISIT_ERROR;
        }
    }
#if 0
    printf("array = {");
    c = nTuples * nComps;
    for(t = 0; t < c; ++t)
        printf("%lg, ", d[t]);
    printf("}\n");
#endif

    return VISIT_OKAY;
}

int
pylibsim_VisIt_VariableData_setData(visit_handle obj, int owner, int nComps, 
    int nTuples, void *seqv)
{
    int ret = VISIT_ERROR;
    double *d = (double *)malloc(sizeof(double) * nComps * nTuples);
    if(d != NULL)
    {
        if(pylibsim_VisIt_VariableData_setDataEx(obj, nComps, nTuples, seqv, d) == VISIT_OKAY)
        {
            /* We will always let VisIt own the memory since we're creating a copy based
             * on the data in the Python objects.
             */
            ret = VisIt_VariableData_setDataD(obj, VISIT_OWNER_VISIT, nComps, nTuples, d);
        }
        else
            free(d);
    }
    return ret;
}

int
pylibsim_VisIt_VariableData_setDataAsI(visit_handle obj, int owner, int nComps, 
    int nTuples, void *seqv)
{
    int ret = VISIT_ERROR;
    double *d = (double *)malloc(sizeof(double) * nComps * nTuples);
    if(d != NULL)
    {
        if(pylibsim_VisIt_VariableData_setDataEx(obj, nComps, nTuples, seqv, d) == VISIT_OKAY)
        {
            int i, nvals, *ivals = NULL;
            nvals = nComps * nTuples;
            ivals = (int *)malloc(sizeof(int) * nvals);
            for(i = 0; i < nvals; ++i)
                ivals[i] = (int)d[i];

            /* We will always let VisIt own the memory since we're creating a copy based
             * on the data in the Python objects.
             */
            ret = VisIt_VariableData_setDataI(obj, VISIT_OWNER_VISIT, nComps, nTuples, ivals);
        }
        free(d);
    }

    return ret;
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

    DISCONNECT_CALLBACK1(VisItSetBroadcastIntFunction);
    DISCONNECT_CALLBACK1(VisItSetBroadcastStringFunction);
    DISCONNECT_CALLBACK1(VisItSetSlaveProcessCallback);
    DISCONNECT_CALLBACK2(VisItSetCommandCallback);

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
