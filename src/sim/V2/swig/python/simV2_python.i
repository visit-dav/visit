/*******************************************************************************
 * Python-specific typemaps and code for the SWIG-generated Python bindings
 * to VisIt's SimV2. Typemaps let us provide custom behavior for arguments
 * such as function pointers or float*.
 *
 * Programmer: Brad Whitlock
 * Date: Thu Mar 24 14:29:25 PDT 2011
 *
 * Modifications:
 *
 ******************************************************************************/

%typemap(in) void* {
    static PyObject *cb[2] = {NULL, NULL};
    if(cb[0] != NULL)
        Py_DECREF(cb[0]); 
    if(cb[1] != NULL)
        Py_DECREF(cb[1]); 

    cb[0] = (obj0 != Py_None) ? obj0 : NULL; /* kind of a hack (assumes void*cbdata will be 2nd arg. */
    cb[1] = ($input != Py_None) ? $input : NULL;

    if(cb[0] != NULL)
        Py_INCREF(cb[0]); 
    if(cb[1] != NULL)
        Py_INCREF(cb[1]); 
    $1 = (void*)cb;
}

/* For VisItSetBroadcastIntFunction*/
%typemap(in) int (*)(int *, int) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern int  pylibsim_setbroadcastintfunction(int*,int);
    extern void pylibsim_setbroadcastintfunction_data(void*);
    pylibsim_setbroadcastintfunction_data((void *)$input);
    $1 = pylibsim_setbroadcastintfunction;
}

/* For VisItSetBroadcastStringFunction*/
%typemap(in) int (*)(char *, int, int) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern int  pylibsim_setbroadcaststringfunction(char *, int, int);
    extern void pylibsim_setbroadcaststringfunction_data(void *);
    pylibsim_setbroadcaststringfunction_data((void *)$input);
    $1 = pylibsim_setbroadcaststringfunction;
}

/* For VisItSetSlaveProcessCallback*/
%typemap(in) void (*)(void) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern void pylibsim_setslaveprocesscallback(void);
    extern void pylibsim_setslaveprocesscallback_data(void*);
    pylibsim_setslaveprocesscallback_data((void *)$input);
    $1 = pylibsim_setslaveprocesscallback;
}

/* For VisItSetCommandCallback*/
%typemap(in) void (*)(const char *, const char *, void *) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern void pylibsim_void__pconstchar_pconstchar_pvoid(const char *, const char *, void *);
    $1 = pylibsim_void__pconstchar_pconstchar_pvoid;
}

/* For SetGetMetaData */
%typemap(in) visit_handle (*)(void *) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern visit_handle pylibsim_visit_handle__pvoid(void *);
    $1 = pylibsim_visit_handle__pvoid;
}

/* For SetActivateTimestep*/
%typemap(in) int (*)(void *) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern int pylibsim_int__pvoid(void *);
    $1 = pylibsim_int__pvoid;
}

/* For SetGetMesh, SetGetVariable,...*/
%typemap(in) visit_handle (*)(int, const char *, void *) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern visit_handle pylibsim_visit_handle__int_pconstchar_pvoid(int, const char *, void *);
    $1 = pylibsim_visit_handle__int_pconstchar_pvoid;
}

/**/
%typemap(in) visit_handle (*)(const char *, void *) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern visit_handle pylibsim_visit_handle__pconstchar_pvoid(const char *, void *);
    $1 = pylibsim_visit_handle__pconstchar_pvoid;
}

/* For writing routines. */
%typemap(in) int (*)(const char *, void *) {
    if($input != Py_None && PyCallable_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        return NULL;
    }
    extern int pylibsim_int__pconstchar_pvoid(const char *, void *);
    $1 = pylibsim_int__pconstchar_pvoid;
}

#define ARRAY_ARGUMENT(T, T2, LEN, CONV) \
%typemap(in) T [LEN] (T2 temp[LEN]) {\
  int i;\
  if (!PySequence_Check($input)) {\
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");\
    return NULL;\
  }\
  if (PySequence_Length($input) != LEN) {\
    PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected " #LEN " elements");\
    return NULL;\
  }\
  for (i = 0; i < LEN; i++) {\
    PyObject *o = PySequence_GetItem($input,i);\
    if (PyNumber_Check(o)) {\
      temp[i] = (T2) CONV(o);\
    } else {\
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");\
      return NULL;\
    }\
  }\
  $1 = temp;\
}

ARRAY_ARGUMENT(int, int, 2, PyInt_AsLong)
ARRAY_ARGUMENT(int, int, 3, PyInt_AsLong)
ARRAY_ARGUMENT(int, int, 6, PyInt_AsLong)
ARRAY_ARGUMENT(double, double, 3, PyFloat_AsDouble)
ARRAY_ARGUMENT(double, double, 6, PyFloat_AsDouble)
ARRAY_ARGUMENT(const int, int, 6, PyInt_AsLong)

/*
 * We can't return handles by reference in Python so wrap the alloc functions
 * for all object types so we can just call the alloc function to return the
 * handle instead of an error code.
 */
#define WRAP_ALLOC(FUNC) \
%rename(FUNC) pylibsim_##FUNC; \
%inline %{\
visit_handle pylibsim_##FUNC(void)\
{\
    visit_handle h = VISIT_INVALID_HANDLE;\
    FUNC(&h);\
    return h;\
}\
%}
WRAP_ALLOC(VisIt_CommandMetaData_alloc)
WRAP_ALLOC(VisIt_CSGMesh_alloc)
WRAP_ALLOC(VisIt_CurveData_alloc)
WRAP_ALLOC(VisIt_CurveMetaData_alloc)
WRAP_ALLOC(VisIt_CurvilinearMesh_alloc)
WRAP_ALLOC(VisIt_DomainBoundaries_alloc)
WRAP_ALLOC(VisIt_DomainList_alloc)
WRAP_ALLOC(VisIt_DomainNesting_alloc)
WRAP_ALLOC(VisIt_ExpressionMetaData_alloc)
WRAP_ALLOC(VisIt_MaterialData_alloc)
WRAP_ALLOC(VisIt_MaterialMetaData_alloc)
WRAP_ALLOC(VisIt_MeshMetaData_alloc)
WRAP_ALLOC(VisIt_NameList_alloc)
WRAP_ALLOC(VisIt_PointMesh_alloc)
WRAP_ALLOC(VisIt_RectilinearMesh_alloc)
WRAP_ALLOC(VisIt_SimulationMetaData_alloc)
WRAP_ALLOC(VisIt_SpeciesData_alloc)
WRAP_ALLOC(VisIt_SpeciesMetaData_alloc)
WRAP_ALLOC(VisIt_UnstructuredMesh_alloc)
WRAP_ALLOC(VisIt_VariableData_alloc)
WRAP_ALLOC(VisIt_VariableMetaData_alloc)

/* Our callback routines keep a reference to python callbacks and data so
 * we need to decrement the reference count when we disconnect.
 */
%rename(VisItDisconnect) pylibsim_invoke_VisItDisconnect;
%inline %{
void pylibsim_invoke_VisItDisconnect(void)
{
    extern void pylibsim_VisItDisconnect(void);
    pylibsim_VisItDisconnect();
}
%}

%typemap(in) double * {
    if(PyList_Check($input) != 1 && PyTuple_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not list or tuple");
        return NULL;
    }
    $1 = (double *)$input; /* Stash the PyObject pointer. We'll use it as such later.*/
}

%typemap(in) float * {
    if(PyList_Check($input) != 1 && PyTuple_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not list or tuple");
        return NULL;
    }
    $1 = (float *)$input; /* Stash the PyObject pointer. We'll use it as such later.*/
}

/* These apply only to VariableData functions.*/
%typemap(in) int *dataarray{
    if(PyList_Check($input) != 1 && PyTuple_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not list or tuple");
        return NULL;
    }
    $1 = (int *)$input;
}

%typemap(in) char * dataarray {
    if(PyList_Check($input) != 1 && PyTuple_Check($input) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not list or tuple");
        return NULL;
    }
    $1 = (char *)$input;
}

%rename(VisIt_VariableData_setDataC) pylibsim_VisIt_VariableData_setDataC;
%rename(VisIt_VariableData_setDataI) pylibsim_VisIt_VariableData_setDataI;
%rename(VisIt_VariableData_setDataF) pylibsim_VisIt_VariableData_setDataF;
%rename(VisIt_VariableData_setDataD) pylibsim_VisIt_VariableData_setDataD;

%inline %{
int pylibsim_VisIt_VariableData_setDataC(visit_handle obj, int owner, int nComps, int nTuples, char *dataarray)
{
    extern int pylibsim_VisIt_VariableData_setData(visit_handle obj, int owner, int nComps, int nTuples, void *dataarray);
    return pylibsim_VisIt_VariableData_setData(obj, owner, nComps, nTuples, (void*)dataarray);
}

int pylibsim_VisIt_VariableData_setDataI(visit_handle obj, int owner, int nComps, int nTuples, int *dataarray)
{
    extern int pylibsim_VisIt_VariableData_setDataAsI(visit_handle obj, int owner, int nComps, int nTuples, void *dataarray);
    return pylibsim_VisIt_VariableData_setDataAsI(obj, owner, nComps, nTuples, (void*)dataarray);
}

int pylibsim_VisIt_VariableData_setDataF(visit_handle obj, int owner, int nComps, int nTuples, float *dataarray)
{
    extern int pylibsim_VisIt_VariableData_setData(visit_handle obj, int owner, int nComps, int nTuples, void *dataarray);
    return pylibsim_VisIt_VariableData_setData(obj, owner, nComps, nTuples, (void*)dataarray);
}

int pylibsim_VisIt_VariableData_setDataD(visit_handle obj, int owner, int nComps, int nTuples, double *dataarray)
{
    extern int pylibsim_VisIt_VariableData_setData(visit_handle obj, int owner, int nComps, int nTuples, void *dataarray);
    return pylibsim_VisIt_VariableData_setData(obj, owner, nComps, nTuples, (void*)dataarray);
}
%}

/* This renaming injects some error handling code that forces VisItProcessEngineCommand
 * to cause the interpreter to quit if one of the callback functions had an error.
 */
%rename(VisItProcessEngineCommand) pylibsim_VisItProcessEngineCommand;
%inline %{
/* This is a dummy function that never gets called. */
int pylibsim_VisItProcessEngineCommand(void)
{
    return VisItProcessEngineCommand();
}
/* Redefine pylibsim_VisItProcessEngineCommand so we inject error checking code for callbacks. */
#define pylibsim_VisItProcessEngineCommand() VisItProcessEngineCommand(); if(PyErr_Occurred()){ return NULL;}
%}

/* Define alternate VisItReadConsole so it returns a string. */
%rename(VisItReadConsole) pylibsim_VisItReadConsole;
%inline %{
char *pylibsim_VisItReadConsole(void)
{
    static char buf[1000];
    if(VisItReadConsole(1000, buf) == VISIT_ERROR)
        buf[0] = '\0';
    return buf;
}
%}

/* Define alternate VisItGetSockets so it returns a tuple. */
%rename(VisItGetSockets) pylibsim_VisItGetSockets;
%inline %{
PyObject *pylibsim_VisItGetSockets(void)
{
    PyObject *tuple = NULL;
    int lSock=-1, cSock=-1;
    VisItGetSockets(&lSock, &cSock);
    tuple = PyTuple_New(2);
    PyTuple_SET_ITEM(tuple, 0, PyInt_FromLong((long)lSock));
    PyTuple_SET_ITEM(tuple, 1, PyInt_FromLong((long)cSock));
    return tuple;
}
%}

/* Material changes. */
%rename(VisIt_MaterialData_addMaterial) pylibsim_VisIt_MaterialData_addMaterial;
%inline %{
int pylibsim_VisIt_MaterialData_addMaterial(visit_handle obj, const char *matName)
{
    int matno = 0;
    VisIt_MaterialData_addMaterial(obj, matName, &matno);
    return matno;
}
%}

%typemap(in) const int *matnos (int matnos[100]) {
  int i;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  if (PySequence_Length($input) < 1) {
    PyErr_SetString(PyExc_ValueError,"Sequence must have at least 1 element");
    return NULL;
  }
  if (PySequence_Length($input) >= 100) {
    PyErr_SetString(PyExc_ValueError,"Sequence must have at most 100 elements");
    return NULL;
  }
  for (i = 0; i < PySequence_Length($input); i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o)) {
      matnos[i] = (int) PyInt_AsLong(o);
      if(matnos[i] < 0)
      {
         PyErr_SetString(PyExc_ValueError,"Material numbers cannot be negative");
         return NULL;
      }
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
      return NULL;
    }
  }
  $1 = matnos;
}

%typemap(in) const float *mixvf (float mixvf[100]) {
  int i;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  if (PySequence_Length($input) < 1) {
    PyErr_SetString(PyExc_ValueError,"Sequence must have at least 1 element");
    return NULL;
  }
  if (PySequence_Length($input) >= 100) {
    PyErr_SetString(PyExc_ValueError,"Sequence must have at most 100 elements");
    return NULL;
  }
  for (i = 0; i < PySequence_Length($input); i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o)) {
      mixvf[i] = (float) PyFloat_AsDouble(o);
      if(mixvf[i] < 0. || mixvf[i] > 1.)
      {
         PyErr_SetString(PyExc_ValueError,"Volume fractions must be in [0.,1.]");
         return NULL;
      }
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
      return NULL;
    }
  }
  $1 = mixvf;
}

/* use the matnos rule to make VisIt_DomainNesting_set_nestingForPatch work. */
%apply const int *matnos { const int *nesting };
