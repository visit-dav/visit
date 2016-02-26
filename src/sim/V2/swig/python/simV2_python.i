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

%include "typemaps.i"

/******************************************************************************
 * used by: VisItGetMemory
 ******************************************************************************/
%apply double *OUTPUT { double *m_size, double *m_rss };

/******************************************************************************
 * some utility functions for use in our typemaps
 ******************************************************************************/
%fragment("util","header") {

/* assign with validation the given object */
int assignCallback(simV2_PyObject &dest, PyObject *src)
{
    if (src == Py_None)
    {
        /* release the current dest */
        dest.Reset();
    }
    else
    if (PyCallable_Check(src) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "not callable");
        dest.Reset();
        return -1;
    }
    else
    {
        dest.SetObject(src);
    }
    return 0;
}

}

/******************************************************************************
 the following three type maps follow similar pattern:
    1) take a reference to and store the user provided callback object
       in a global variable.
    2) Give visit a pointer to an "invoker" function that when called
       uses the Python C-API to call the user's python function.
    these are cleared either by user passing None or calling VisItFinalize
 ******************************************************************************/

/******************************************************************************
 * BroadcastIntFunction
 ******************************************************************************/
%typemap(in, fragment="util") (int (*bicb)(int *, int)) {
    simV2_PyObject temp;
    if (assignCallback(temp, $input))
    {
        pylibsim_setBroadcastIntCallback(NULL);
        return NULL;
    }
    pylibsim_setBroadcastIntCallback(temp);
    $1 = pylibsim_invokeBroadcastIntCallback;
}

/******************************************************************************
 * BroadcastStringFunction
 ******************************************************************************/
%typemap(in) (int (*bscb)(char *, int, int)) {
    simV2_PyObject temp;
    if (assignCallback(temp, $input))
    {
        pylibsim_setBroadcastStringCallback(NULL);
        return NULL;
    }
    pylibsim_setBroadcastStringCallback(temp);
    $1 = pylibsim_invokeBroadcastStringCallback;
}

/******************************************************************************
 * SlaveProcessCallback
 ******************************************************************************/
%typemap(in) (void (*spcb)(void)) {
    simV2_PyObject temp;
    if (assignCallback(temp, $input))
    {
        pylibsim_setSlaveProcessCallback(NULL);
        return NULL;
    }
    pylibsim_setSlaveProcessCallback(temp);
    $1 = pylibsim_invokeSlaveProcessCallback;
}

/******************************************************************************
 * typemap for callback's data

    this is a bit tricky. we're going to package the python callback and its
    data and pass into visit as a single object. we then insert an invoker
    function that visit will call with this package. the invoker will
    unpackage and invokes the python function passing the callback data
    to it.

    in libsim's set callback functions, the callback always directly
    precedes the callback data. we're taking advantage of that order
    by declaring a wrapper scope local variable which we can use to
    pass the user's callback into the typemap that handles the callback
    data. The latter typemap will then package them both into a single
    object as described above.

    our usage pattern is as follows
    1) typemap for a callback (various function pointers) creates a local
       validates and places the passed in object in it. gives visit a
       pointer to the invoker function which has the same signature.
    2) typemap for callback data (void *cbdataN) allocates a pair constructed
       with the callback object (now in the local variable created in 1)
       and the passed in object. The pair is saved in a global list
       and delete'd when the engine disconnects.

 ******************************************************************************/
/* for when callback is the first argument */
%typemap(in) (void *cbdata1) {
    simV2_CallbackData *package = newCallbackData(callback1, $input);
    $1 = static_cast<void*>(package);
}
/* same as above but for when callback is argument 2 */
%typemap(in) (void *cbdata2) {
    simV2_CallbackData *package = newCallbackData(callback2, $input);
    $1 = static_cast<void*>(package);
}

/******************************************************************************
 * used by: BroadcastIntFunction2
 ******************************************************************************/
%typemap(in, fragment="util") (int (*)(int *, int, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_i_F_pi_i_pv;
}

/******************************************************************************
 * used by: BroadcastStringFunction2
 ******************************************************************************/
%typemap(in, fragment="util") (int (*)(char *, int, int, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_i_F_pcc_i_i_pv;
}

/******************************************************************************
 * used by: SlaveProcess2
 ******************************************************************************/
%typemap(in, fragment="util") (void (*)(void*)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_v_F_pv;
}

/******************************************************************************
 * used by: CommandCalllback
 ******************************************************************************/
%typemap(in, fragment="util", fragment="util") (void (*)(const char *, const char *, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_v_F_pcc_pcc_pv;
}

/******************************************************************************
 * used by: GetMetaData
 ******************************************************************************/
%typemap(in, fragment="util") (visit_handle (*)(void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_h_F_pv;
}

/******************************************************************************
 * used by: ActivateTimestep
 ******************************************************************************/
%typemap(in, fragment="util") (int (*)(void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_i_F_pv;
}

/******************************************************************************
 * used by: GetMesh GetMaterial GetSpecies GetVariable GetMixedVariable callbacks
 ******************************************************************************/
%typemap(in, fragment="util") (visit_handle (*)(int, const char *, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_h_F_i_pcc_pv;
}

/******************************************************************************
 * used by: GetCurve GetDomainList GetDomainBoundaries GetDomainNesting
 ******************************************************************************/
%typemap(in, fragment="util") (visit_handle (*)(const char *, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_h_F_pcc_pv;
}

/******************************************************************************
 * used by : WriteBegin, WriteEnd
 ******************************************************************************/
%typemap(in, fragment="util") (int (*)(const char *, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_i_F_pcc_pv;
}

/******************************************************************************
 * used by WriteMesh
 ******************************************************************************/
%typemap(in, fragment="util") (int (*cb)(const char *, int, int, visit_handle, visit_handle, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_i_F_pcc_i_i_h_h_pv;
}

/******************************************************************************
 * used by: WriteVariable
 ******************************************************************************/
%typemap(in, fragment="util") (int (*cb)(const char *, const char *, int, visit_handle, visit_handle, void *)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_i_F_pcc_pcc_i_h_h_pv;
}

/******************************************************************************
 * used by: UI_clicked
 ******************************************************************************/
%typemap(in, fragment="util") (void (*cb)(void*)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_v_F_pv;
}

/******************************************************************************
 * used by: UI_stateChanged, UI_valueChanged
 ******************************************************************************/
%typemap(in, fragment="util") (void (*cb)(int,void*)) (simV2_PyObject callback) {
    if (assignCallback(callback, $input)) { return NULL; }
    $1 = pylibsim_invoke_v_F_i_pv;
}


/******************************************************************************
 ******************************************************************************/
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
    pylibsim_VisItDisconnect();
}
%}

/* inject a finalize function for cleanup of various
internal data that would otherwise be leaked */
%inline{
void VisItFinalize() { pylibsim_VisItFinalize(); }
}

/*
 * we need to convert from python data structures to c
 * arrays. intercept calls passing data arrays into
 * VisIt to do so.
 */
%typemap(in) double *dataarray { $1 = (double *)$input; }
%typemap(in) float *dataarray { $1 = (float *)$input; }
%typemap(in) int *dataarray { $1 = (int *)$input; }
%typemap(in) char *dataarray { $1 = (char *)$input; }

%rename(VisIt_VariableData_setDataC) pylibsim_VisIt_VariableData_setDataC;
%rename(VisIt_VariableData_setDataI) pylibsim_VisIt_VariableData_setDataI;
%rename(VisIt_VariableData_setDataF) pylibsim_VisIt_VariableData_setDataF;
%rename(VisIt_VariableData_setDataD) pylibsim_VisIt_VariableData_setDataD;

%inline %{

int pylibsim_VisIt_VariableData_setDataC(visit_handle obj, int owner, int nComps, int nTuples, char *dataarray)
{
    return pylibsim_VisIt_VariableData_setDataAsC(obj, owner, nComps, nTuples, (PyObject*)dataarray);
}

int pylibsim_VisIt_VariableData_setDataI(visit_handle obj, int owner, int nComps, int nTuples, int *dataarray)
{
    return pylibsim_VisIt_VariableData_setDataAsI(obj, owner, nComps, nTuples, (PyObject*)dataarray);
}

int pylibsim_VisIt_VariableData_setDataF(visit_handle obj, int owner, int nComps, int nTuples, float *dataarray)
{
    return pylibsim_VisIt_VariableData_setDataAsF(obj, owner, nComps, nTuples, (PyObject*)dataarray);
}

int pylibsim_VisIt_VariableData_setDataD(visit_handle obj, int owner, int nComps, int nTuples, double *dataarray)
{
    return pylibsim_VisIt_VariableData_setDataAsD(obj, owner, nComps, nTuples, (PyObject*)dataarray);
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
