#include <Python.h>
#include <stdio.h>
#include <visit-config.h> // for VERSION
#if !defined(_WIN32)
#include <strings.h>
#endif
#include <snprintf.h>
#include <map>

#define THREADS

//
// Include threads
//
#ifdef THREADS
#if defined(_WIN32)
#include <windows.h>
#ifdef GetMessage
#undef GetMessage
#endif
#else
#include <pthread.h>
#endif
#endif

#include <Connection.h>
#include <Observer.h>
#include <ObserverToCallback.h>
#include <ViewerProxy.h>
#include <VisItException.h>
#include <LostConnectionException.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <QueryList.h>
#include <Init.h>
#include <DebugStream.h>

//
// State object include files.
//
#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <ColorTableAttributes.h>
#include <DatabaseCorrelationList.h>
#include <DatabaseCorrelation.h>
#include <EngineList.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <InteractorAttributes.h>
#include <KeyframeAttributes.h>
#include <MessageAttributes.h>
#include <PickAttributes.h>
#include <Plot.h>
#include <PlotList.h>
#include <PluginManagerAttributes.h>
#include <QueryAttributes.h>
#include <PrinterAttributes.h>
#include <RenderingAttributes.h>
#include <StatusAttributes.h>
#include <SyncAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <ViewAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>
#include <ExpressionList.h>
#include <Expression.h>

//
// Extension include files.
//
#include <PyAnnotationAttributes.h>
#include <PyGlobalAttributes.h>
#include <PyGlobalLineoutAttributes.h>
#include <PyHostProfile.h>
#include <PyInteractorAttributes.h>
#include <PyKeyframeAttributes.h>
#include <PyMaterialAttributes.h>
#include <PyPickAttributes.h>
#include <PyPrinterAttributes.h>
#include <PyRenderingAttributes.h>
#include <PySaveWindowAttributes.h>
#include <PySILRestriction.h>
#include <PyText2DObject.h>
#include <PyQueryOverTimeAttributes.h>
#include <PyTimeSliderObject.h>
#include <PyViewAttributes.h>
#include <PyViewCurveAttributes.h>
#include <PyView2DAttributes.h>
#include <PyView3DAttributes.h>
#include <PyWindowInformation.h>

#include <avtSILRestrictionTraverser.h>

//
// Defines.
//
#define STATIC

#define ADD_EXTENSION(GM) methods = GM(&nMethods); \
    for(i = 0; i < nMethods; ++i) \
    { \
        debug1 << "Added method: " << methods[i].ml_name << endl; \
        AddMethod(methods[i].ml_name, methods[i].ml_meth); \
    }

#define ENSURE_VIEWER_EXISTS() if(noViewer) {\
    PyErr_SetString(VisItError, "VisIt's viewer is not running!"); \
    return NULL; }

//
// Make the initvisit function callable from C.
//
extern "C"
{
    void initvisit();
    void cli_initvisit(int, bool, int, char **);
    void cli_runscript(const char *);
}

//
// Visible Prototypes.
//
void VisItErrorFunc(const char *errString);
const ViewerProxy *VisItViewer();

//
// Prototypes
//
static void terminatevisit();
#if defined(_WIN32)
static DWORD WINAPI visit_eventloop(LPVOID);
#else
static void *visit_eventloop(void *);
#endif
static void CloseModule();
static void CreateListenerThread();
static void LaunchViewer();
static int  Synchronize();
static void SetLogging(bool val);
static void DelayedLoadPlugins();
static void PlotPluginAddInterface();
static void OperatorPluginAddInterface();
static void InitializeExtensions();

//
// Type definitions
//

struct AttributesObject
{
    PyObject_HEAD
    AttributeSubject *data;
};

//
// This is an observer that watches the viewer's message attributes and
// prints an error message when a message comes in.
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 11:47:40 PDT 2002
//   I made this class have methods to return information about the last
//   error that came in.
//
class VisItMessageObserver : public Observer
{
public:
    VisItMessageObserver(Subject *s) : Observer(s), lastError("")
    {
        errorFlag = 0;
    };

    virtual ~VisItMessageObserver() { };

    void ClearError()
    {
        errorFlag = 0;
    };

    int ErrorFlag() const { return errorFlag; };
    const std::string &GetLastError() const { return lastError; };

    virtual void Update(Subject *s)
    {
        MessageAttributes *m = (MessageAttributes *)s;

        if(m->GetSeverity() == MessageAttributes::Error)
        {
            // Store information about the error.
            errorFlag = 1;
            lastError = m->GetText();

            fprintf(stderr, "VisIt: Error - %s\n", m->GetText().c_str());
//            // This can't really be done this way since this code is only
//            // ever called by the 2nd thread. It cannot use Python.
//            VisItErrorFunc(m->GetText().c_str());
        }
        else if(m->GetSeverity() == MessageAttributes::Warning)
            fprintf(stderr, "VisIt: Warning - %s\n", m->GetText().c_str());
        else
            fprintf(stderr, "VisIt: Message - %s\n", m->GetText().c_str());
    }
private:
    int         errorFlag;
    std::string lastError;
};

//
// This is an observer that watches the viewer's status attributes and
// prints out status messages that come in.
//
class VisItStatusObserver : public Observer
{
public:
    VisItStatusObserver(Subject *s) : Observer(s)
    {
        verbose = false;
    };

    virtual ~VisItStatusObserver() { };

    void SetVerbose(bool val) { verbose = val; };

    virtual void Update(Subject *s)
    {
        StatusAttributes *statusAtts = (StatusAttributes *)s;

        // Update the status bar.
        if(verbose && !statusAtts->GetClearStatus())
        {
            // If the message field is selected, use it. Otherwise, make up
            // a message based on the other fields.
            if(statusAtts->GetMessageType() == 1)
                fprintf(stderr, "%s\n", statusAtts->GetMessage().c_str());
            else if (statusAtts->GetMessageType() == 2)
            {
                int total;
                if (statusAtts->GetMaxStage() > 0)
                {
                    float pct0  = float(statusAtts->GetPercent()) / 100.;
                    float pct1  = float(statusAtts->GetCurrentStage()-1) / float(statusAtts->GetMaxStage());
                    float pct2  = float(statusAtts->GetCurrentStage())   / float(statusAtts->GetMaxStage());
                    total = int(100. * ((pct0 * pct2) + ((1.-pct0) * pct1)));
                }
                else
                    total = 0;

                fprintf(stderr, "%d%% done: %s (%d%% of stage %d/%d)\n",
                    total,
                    statusAtts->GetCurrentStageName().c_str(),
                    statusAtts->GetPercent(),
                    statusAtts->GetCurrentStage(),
                    statusAtts->GetMaxStage());
            }
        }        
    }
private:
    bool verbose;
};

//
// VisIt module state flags and objects.
//
static PyObject             *visitModule = 0;
static bool                  moduleInitialized = false;
static bool                  keepGoing = true;
static ViewerProxy          *viewer = 0;
static FILE                 *logFile = 0;
static bool                  logging = true;
#ifdef THREADS
static bool                  moduleUseThreads = true;
#else
static bool                  moduleUseThreads = false;
#endif
static bool                  noViewer = true;
static int                   moduleDebugLevel = 0;
static VisItMessageObserver *messageObserver = 0;
static VisItStatusObserver  *statusObserver = 0;
static bool                  moduleVerbose = false;
static ObserverToCallback   *pluginLoader = 0;
static bool                  localNameSpace = false;
static int                   syncCount = 0;
static PyObject             *VisItError;

static int                   cli_argc = 0;
static char                **cli_argv = 0;

static std::vector<AnnotationObject *>   localObjectList;
static std::map<AnnotationObject *, int> localObjectReferenceCount;

#ifdef THREADS
#if defined(_WIN32)
#define POLLING_SYNCHRONIZE
static  CRITICAL_SECTION     mutex;
#define THREAD_INIT()
#define MUTEX_CREATE()       InitializeCriticalSection(&mutex)
#define MUTEX_DESTROY() 
#define MUTEX_LOCK()         EnterCriticalSection(&mutex)
#define MUTEX_UNLOCK()       LeaveCriticalSection(&mutex)
#else
// pthreads thread-related stuff.
static pthread_attr_t        thread_atts;
static pthread_mutex_t       mutex, sync_mutex;
static pthread_cond_t        received_sync_from_viewer;
static bool waitingForViewer = false;
static ObserverToCallback   *synchronizeCallback = 0;
#define THREAD_INIT()        pthread_attr_init(&thread_atts)
#define MUTEX_CREATE()       pthread_mutex_init(&mutex, NULL)
#define MUTEX_DESTROY()      pthread_mutex_destroy(&mutex)
#define MUTEX_LOCK()         pthread_mutex_lock(&mutex)
#define MUTEX_UNLOCK()       pthread_mutex_unlock(&mutex)

#define SYNC_MUTEX_LOCK()    pthread_mutex_lock(&sync_mutex);
#define SYNC_MUTEX_UNLOCK()  pthread_mutex_unlock(&sync_mutex);

#define SYNC_COND_WAIT()     if(keepGoing) \
                             {\
                                 waitingForViewer = true; \
                                 pthread_cond_wait(&received_sync_from_viewer, \
                                 &sync_mutex); \
                                 waitingForViewer = false; \
                             } \
                             SYNC_MUTEX_UNLOCK();
#define SYNC_WAKE_MAIN_THREAD() if(waitingForViewer) \
                                    WakeMainThread(0, 0);
#define SYNC_CREATE()        pthread_mutex_init(&sync_mutex, NULL); \
                             pthread_cond_init(&received_sync_from_viewer, NULL);

#define SYNC_DESTROY()       pthread_mutex_destroy(&sync_mutex); \
                             pthread_cond_destroy(&received_sync_from_viewer);

// Called by the listener thread when SyncAttributes is read from the viewer.
static void WakeMainThread(Subject *, void *)
{
    SYNC_MUTEX_LOCK();
    if(viewer->GetSyncAttributes()->GetSyncTag() == syncCount || !keepGoing)
        pthread_cond_signal(&received_sync_from_viewer);
    SYNC_MUTEX_UNLOCK();
}

#endif
#else
#define POLLING_SYNCHRONIZE
#define MUTEX_CREATE()
#define MUTEX_DESTROY()
#define MUTEX_LOCK()
#define MUTEX_UNLOCK()
#endif

//
// VisIt module functions that are written in Python.
//
static const char visit_EvalCubicSpline[] = 
"def EvalCubicSpline(t, allX, allY):\n"
"    n = len(allY)\n"
"    if((allX[0] > t) or (allX[n-1] < t)):\n"
"        raise 't must be in the range between the first and last X'\n"
"    for i in range(1, n):\n"
"        if(allX[i] >= t):\n"
"            break\n"
"    i1 = max(i-2, 0)\n"
"    i2 = max(i-1, 0)\n"
"    i3 = i\n"
"    i4 = min(i+1, n-1)\n"
"    X = (allX[i1], allX[i2], allX[i3], allX[i4])\n"
"    Y = (allY[i1], allY[i2], allY[i3], allY[i4])\n"
"    dx = (X[2] - X[1])\n"
"    invdx = 1. / dx\n"
"    dy1   = (Y[2] + (Y[0] * -1.)) * (1. / (X[2] - X[0]))\n"
"    dy2   = (Y[2] + (Y[1] * -1.)) * invdx\n"
"    dy3   = (Y[3] + (Y[1] * -1.)) * (1. / (X[3] - X[1]))\n"
"    ddy2  = (dy2 + (dy1 * -1)) * invdx\n"
"    ddy3  = (dy3 + (dy2 * -1)) * invdx\n"
"    dddy3 = (ddy3 + (ddy2 * -1)) * invdx\n"
"    u = (t - X[1])\n"
"    return (Y[1] + dy1*u + ddy2*u*u + dddy3*u*u*(u-dx))\n";

static const char visit_EvalLinear[] = 
"def EvalLinear(t, c0, c1):\n"
"    return ((c0*(1. - float(t))) + (c1*float(t)))\n";

static const char visit_EvalQuadratic[] = 
"def EvalQuadratic(t, c0, c1, c2):\n"
"    T = float(t)\n"
"    T2 = T * T\n"
"    OMT = 1. - T\n"
"    OMT2 = OMT * OMT\n"
"    return ((c0*OMT2) + (c1*(2.*OMT*T)) + (c2*T2))\n";

static const char visit_EvalCubic[] = 
"def EvalCubic(t, c0, c1, c2, c3):\n"
"    T = float(t)\n"
"    T2 = T * T\n"
"    T3 = T * T2\n"
"    OMT = 1. - T\n"
"    OMT2 = OMT * OMT\n"
"    OMT3 = OMT2 * OMT\n"
"    return ((c0*OMT3) + (c1*(3.*OMT2*T)) + (c2*(3.*OMT*T2)) + (c3*T3))\n";

// ****************************************************************************
// Function: IntReturnValue
//
// Purpose: 
//   Returns a Python long, which indicates the success value for the VisIt
//   methods. If errorFlag < 0 then the viewer died. Return 0 so the 
//   interpreter stops.
//
// Arguments:
//   errorFlag : The error Flag. 0 means success. 1 means failure. -1 means
//               that the viewer died.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 12:31:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

PyObject *
IntReturnValue(int errorFlag)
{
    PyObject *retval = NULL;

    if(errorFlag >= 0)
        retval = PyLong_FromLong(long(errorFlag == 0));

    return retval;
}

// ****************************************************************************
// Function: DeprecatedMessage
//
// Purpose: 
//   Prints a message for a deprecated function.
//
// Arguments:
//   deprecatedFunction : The function that was deprecated.
//   var                : The version in which the function was deprecated.
//   newFunction        : The new function that should be called.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 08:58:03 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
DeprecatedMessage(const char *deprecatedFunction, const char *ver,
    const char *newFunction)
{
    fprintf(stderr, 
            "***\n"
            "*** %s was deprecated in version %s.\n"
            "*** Calling %s will still work for now but you should\n"
            "*** update your code so it uses the %s function.\n"
            "***\n",
            deprecatedFunction, ver, deprecatedFunction, newFunction);
}

// ****************************************************************************
// Method: GetStringVectorFromPyObject
//
// Purpose: 
//   Populates a string vector from values in a PyObject.
//
// Arguments:
//   obj : The PyObject that we're checking for strings.
//   vec : The string vector that we're populating.
//
// Returns:    True if successful; false otherwise.
//
// Note:       Adapted from ParseTupleForVars but it can parse more than
//             tuples.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:51:39 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
GetStringVectorFromPyObject(PyObject *obj, stringVector &vec)
{
    bool retval = true;

    if(obj == 0)
    {
        retval = false;
    }
    else if(PyTuple_Check(obj))
    {
        // Extract arguments from the tuple.
        for(int i = 0; i < PyTuple_Size(obj); ++i)
        {
            PyObject *item = PyTuple_GET_ITEM(obj, i);
            if(PyString_Check(item))
                vec.push_back(PyString_AS_STRING(item));
            else
            {
                VisItErrorFunc("The tuple must contain all strings.");
                retval = false;
                break;
            }
        }
    }
    else if(PyList_Check(obj))
    {
        // Extract arguments from the list.
        for(int i = 0; i < PyList_Size(obj); ++i)
        {
            PyObject *item = PyList_GET_ITEM(obj, i);
            if(PyString_Check(item))
                vec.push_back(PyString_AS_STRING(item));
            else
            {
                VisItErrorFunc("The list must contain all strings.");
                retval = false;
                break;
            }
        }
    }
    else if(PyString_Check(obj))
    {
        vec.push_back(PyString_AS_STRING(obj));
    }
    else
    {
        retval = false;
        VisItErrorFunc("The object could not be converted to a "
                       "vector of strings.");
    }

    return retval;
}

// ****************************************************************************
// Method: GetDoubleArrayFromPyObject
//
// Purpose: 
//   Fills a double array with values from a tuple or list.
//
// Arguments:
//   obj    : The PyObject from which we're getting the values.
//   array  : The destination array.
//   maxLen : The length of the destination array.
//
// Returns:    True if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 10:14:33 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
GetDoubleArrayFromPyObject(PyObject *obj, double *array, int maxLen)
{
    bool retval = true;

    if(obj == 0)
    {
        retval = false;
    }
    else if(PyTuple_Check(obj))
    {
        int size = PyTuple_Size(obj);
        for(int i = 0; i < size && i < maxLen; ++i)
        {
            PyObject *item = PyTuple_GET_ITEM(obj, i);
            if(PyFloat_Check(item))
                array[i] = PyFloat_AS_DOUBLE(item);
            else if(PyInt_Check(item))
                array[i] = double(PyInt_AS_LONG(item));
            else if(PyLong_Check(item))
                array[i] = double(PyLong_AsDouble(item));
        }
    }
    else if(PyList_Check(obj))
    {
        int size = PyList_Size(obj);
        for(int i = 0; i < size && i < maxLen; ++i)
        {
            PyObject *item = PyList_GET_ITEM(obj, i);
            if(PyFloat_Check(item))
                array[i] = PyFloat_AS_DOUBLE(item);
            else if(PyInt_Check(item))
                array[i] = double(PyInt_AS_LONG(item));
            else if(PyLong_Check(item))
                array[i] = double(PyLong_AsDouble(item));
        }
    }
    else if(PyFloat_Check(obj))
    {
        array[0] = PyFloat_AS_DOUBLE(obj);
    }
    else if(PyInt_Check(obj))
    {
        array[0] = double(PyInt_AS_LONG(obj));
    }
    else if(PyLong_Check(obj))
    {
        array[0] = double(PyLong_AsDouble(obj));
    }
    else
    {
        retval = false;
    }

    return retval;
}

//
// Python callbacks for VisIt
//

// ****************************************************************************
// Function: visit_AddArgument
//
// Purpose: 
//   This is a Python callback that can be used to add arguments that are
//   passed to the viewer before it is created.
//
// Note:       Only has an effect before the viewer is created.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 4 16:55:58 PST 2002
//   Removed code to flag certain arguments.
//
// ****************************************************************************

STATIC PyObject *
visit_AddArgument(PyObject *self, PyObject *args)
{
    if(noViewer)
    {
        char *arg;
        if(!PyArg_ParseTuple(args, "s", &arg))
            return NULL;

        viewer->AddArgument(arg);
    }
    else
    {
        VisItErrorFunc("VisIt's viewer is already launched!");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_Version
//
// Purpose: 
//   This is a Python callback that returns the Version of the VisIt module.
//
// Note:       Can be called before the viewer is created.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_Version(PyObject *self, PyObject *args)
{
    return PyString_FromString(VERSION);
}

// ****************************************************************************
// Function: visit_Launch
//
// Purpose: 
//   This is a Python callback that launches the viewer.
//
// Note:       Only has an effect before the viewer is created.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon May 6 14:01:17 PST 2002
//   I moved the function calls that add the plugin interfaces to after the
//   call to Synchronize. This ensures that all state objects have good
//   default values from the viewer before we initialize local default values.
//
// ****************************************************************************

STATIC PyObject *
visit_Launch(PyObject *self, PyObject *args)
{
    //
    // Check to see if the viewer is already launched.
    //
    if(!noViewer)
    {
        VisItErrorFunc("VisIt's viewer is already launched!");
        return NULL;
    }

    //
    // Launch the viewer.
    //
    LaunchViewer();

    //
    // If the noViewer flag is false then the viewer could not launch.
    //
    if(noViewer)
    {
        VisItErrorFunc("VisIt's viewer could not launch!");
        return NULL;
    }

    //
    // Wait for the viewer to tell us to load the plugins.
    //    
    DelayedLoadPlugins();

    // Create the thread that listens for input from the viewer. Then
    // synchronize to flush out any initialization that came from the viewer
    // before we allow more commands to execute.
    CreateListenerThread();
    int errorFlag = Synchronize();

    //
    // Initialize the extensions.
    //
    InitializeExtensions();

    //
    // Iterate over the plugins and add their methods to the VisIt module's
    // methods.
    //
    PlotPluginAddInterface();
    OperatorPluginAddInterface();

    return IntReturnValue(errorFlag);
}

// ****************************************************************************
// Function: visit_LaunchNowin
//
// Purpose: 
//   This is a Python callback that launches the viewer without a window.
//
// Note:       Only has an effect before the viewer is created.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_LaunchNowin(PyObject *self, PyObject *args)
{
    if(noViewer)
        viewer->AddArgument("-nowin");
    return visit_Launch(self, args);
}

// ****************************************************************************
// Function: visit_SetDebugLevel
//
// Purpose: 
//   This is a Python callback that can be used to set the debug level that
//   the viewer uses.
//
// Note:       Only has an effect before the viewer is created.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetDebugLevel(PyObject *self, PyObject *args)
{
    int dLevel;
    if(!PyArg_ParseTuple(args, "i", &dLevel))
        return NULL;

    if(dLevel < 1 || dLevel > 5)
    {
        VisItErrorFunc("Valid debug levels are 1..5.");
        return NULL;
    }

    if(noViewer)
    {
        // Save the debug level.
        moduleDebugLevel = dLevel;

        viewer->AddArgument("-debug");
        char tmp[10];
        SNPRINTF(tmp, 10, "%d", moduleDebugLevel);
        viewer->AddArgument(tmp); 
    }
    else
    {
        VisItErrorFunc("Setting the debug level after the "
                     "viewer is launched has no effect.");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_GetDebugLevel
//
// Purpose: 
//   This is a Python callback that returns the debug level that the viewer
//   is using.
//
// Note:       Can be used before the viewer is launched.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetDebugLevel(PyObject *self, PyObject *args)
{
    return PyInt_FromLong(long(moduleDebugLevel));
}

// ****************************************************************************
// Function: visit_GetDebugLevel
//
// Purpose: 
//   Returns the last error that VisIt sent back to the cli.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 26 12:15:57 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetLastError(PyObject *self, PyObject *args)
{
    const char *str = "";
    if(messageObserver)
        str = messageObserver->GetLastError().c_str();

    return PyString_FromString(str);
}

// ****************************************************************************
// Function: visit_LocalNameSpace
//
// Purpose: 
//   Determines how plugins are added to the Python namespace. If this
//   function is called, all the plugin methods are added to the top-level
//   as if "from visit import *" was specified.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 9 19:00:31 PST 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_LocalNameSpace(PyObject *, PyObject *)
{
    localNameSpace = true;
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_GetLocalHostName
//
// Purpose: 
//   Returns the local host name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 10:09:52 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetLocalHostName(PyObject *self, PyObject *args)
{
    return PyString_FromString(viewer->GetLocalHostName().c_str());
}

// ****************************************************************************
// Function: visit_GetLocalUserName
//
// Purpose: 
//   Returns the user's login name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 10:09:52 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetLocalUserName(PyObject *self, PyObject *args)
{
    return PyString_FromString(viewer->GetLocalUserName().c_str());
}

// ****************************************************************************
// Function: visit_Close
//
// Purpose: 
//   This is a Python callback that closes the viewer.
//
// Note:       Only has an effect if the viewer is created.
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Tue Sep 18 14:04:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_Close(PyObject *self, PyObject *args)
{
    if(!noViewer)
    {
        CloseModule();

        MUTEX_LOCK();
        if(logging)
            fprintf(logFile, "Close()\n");
        MUTEX_UNLOCK();
    }
    else
    {
        VisItErrorFunc("No viewer exists to be closed.");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: LongFileName
//
// Purpose: 
//   Converts a Windows short filename into a long filename.
//
// Arguments:
//   shortName : The short Windows name of the file.
//
// Returns:    The long windows name of the file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 5 14:10:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_LongFileName(PyObject *self, PyObject *args)
{
    char *fileName = 0;
    if(!PyArg_ParseTuple(args, "s", &fileName))
        return NULL;

#if defined(_WIN32)
    char *buf = new char[1000];
    PyObject *retval = 0;
    if(GetLongPathName(fileName, buf, 1000) != 0)
        retval = PyString_FromString(buf);
    else
        retval = PyString_FromString(fileName);
    delete [] buf;
    return retval;
#else
    return PyString_FromString(fileName);
#endif
}

//
// ViewerProxy methods.
//

// ****************************************************************************
// Function: visit_AddWindow
//
// Purpose:
//   Tells the viewer to add a window.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Dec 30 13:10:54 PST 2002
//   I made it be synchronized.
//
// ****************************************************************************

STATIC PyObject *
visit_AddWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->AddWindow();
        if(logging)
            fprintf(logFile, "AddWindow()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ShowAllWindows
//
// Purpose:
//   Tells the viewer to show its windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 12:52:36 PDT 2003
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_ShowAllWindows(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ShowAllWindows();
        if(logging)
            fprintf(logFile, "ShowAllWindows()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CloneWindow
//
// Purpose:
//   Tells the viewer to clone the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 16 10:18:46 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Dec 30 13:10:42 PST 2002
//   I made it be synchronized.
//
// ****************************************************************************

STATIC PyObject *
visit_CloneWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->CloneWindow();
        if(logging)
            fprintf(logFile, "CloneWindow()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetDatabaseNStates
//
// Purpose: 
//   Gets the number of time states in the open database.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 14 17:02:02 PST 2001
//
// Modifications:
//   Eric Brugger, Mon Dec 16 10:20:27 PST 2002
//   Renamed to match the seperation of state and frame.
//
//   Brad Whitlock, Mon Dec 30 13:10:14 PST 2002
//   I made more of the animation methods synchronized.
//
//   Brad Whitlock, Tue Mar 2 08:49:45 PDT 2004
//   I rewrote the method to use the active source and its database
//   correlation to get the number of states for the database.
//
//   Brad Whitlock, Tue Apr 13 12:07:05 PDT 2004
//   I fixed a bug with my rewrite so single time state databases return 1
//   instead of 0 when we have an open single time state database.
//
// ****************************************************************************

STATIC PyObject *
visit_GetDatabaseNStates(PyObject *self, PyObject *args)
{
    WindowInformation *wi = viewer->GetWindowInformation();
    DatabaseCorrelationList *correlations = viewer->GetDatabaseCorrelationList();

    // Get the number of states for the active source.
    const std::string &source = wi->GetActiveSource();
    DatabaseCorrelation *c = correlations->FindCorrelation(source);
    int nStates = (source == "notset" || source == "") ? 0 : 1;
    if(c != 0)
        nStates = c->GetNumStates();

    PyObject *retval = PyLong_FromLong(long(nStates));
    return retval;
}

// ****************************************************************************
// Function: visit_TimeSliderNextState
//
// Purpose: 
//   Tells the viewer to advance the active time slider to the next state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 08:51:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_TimeSliderNextState(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->TimeSliderNextState();
        if(logging)
            fprintf(logFile, "TimeSliderNextState()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

STATIC PyObject *
visit_AnimationNextFrame(PyObject *self, PyObject *args)
{
    DeprecatedMessage("AnimationNextFrame", "1.3", "TimeSliderNextState");
    return visit_TimeSliderNextState(self, args);
}

// ****************************************************************************
// Function: visit_TimeSliderPreviousState
//
// Purpose: 
//   Tells the viewer to set the active time slider to the previous state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 08:51:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_TimeSliderPreviousState(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->TimeSliderPreviousState();
        if(logging)
            fprintf(logFile, "TimeSliderPreviousState()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

STATIC PyObject *
visit_AnimationPreviousFrame(PyObject *self, PyObject *args)
{
    DeprecatedMessage("AnimationPreviousFrame", "1.3", "TimeSliderPreviousState");
    return visit_TimeSliderPreviousState(self, args);
}

// ****************************************************************************
// Function: visit_SetTimeSliderState
//
// Purpose: 
//   Tells the viewer to set the time state for the active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:01:59 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 7 13:38:40 PST 2004
//   I changed the code to get the number of states to account for
//   keyframe mode.
//
//   Brad Whitlock, Tue Apr 13 16:11:01 PST 2004
//   I made it okay to call this function when there is no active time slider.
//
// ****************************************************************************

STATIC PyObject *
visit_SetTimeSliderState(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    //
    // Make sure that there is a time slider.
    //
    WindowInformation *wi = viewer->GetWindowInformation();
    if(wi->GetActiveTimeSlider() < 0)
    {
        fprintf(stderr, "SetTimeSliderState was called when there was no "
                        "time slider.\n");
        return IntReturnValue(Synchronize());
    }

    //
    // Get the number of states for the active time slider.
    //
    int nStates = 1;
    const std::string &ts = wi->GetTimeSliders()[wi->GetActiveTimeSlider()];
    if(viewer->GetKeyframeAttributes()->GetEnabled())
        nStates = viewer->GetKeyframeAttributes()->GetNFrames();
    else
    {
        DatabaseCorrelationList *correlations = viewer->GetDatabaseCorrelationList();
        DatabaseCorrelation *c = correlations->FindCorrelation(ts);
        if(c != 0)
            nStates = c->GetNumStates();
    }

    int state;
    if(!PyArg_ParseTuple(args, "i", &state))
        return NULL;

    if(state < 0 || state >= nStates)
    {
        fprintf(stderr, "The active time slider, %s, has states in this range:"
            " [0,%d]. You cannot use %d for the new time slider state because "
            "that value is not in the range for the time slider\n", 
            ts.c_str(), nStates-1, state);
        return NULL;
    }

    MUTEX_LOCK();
        viewer->SetTimeSliderState(state);
        if(logging)
            fprintf(logFile, "SetTimeSliderState(%d)\n", state);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

STATIC PyObject *
visit_AnimationSetFrame(PyObject *self, PyObject *args)
{
    DeprecatedMessage("AnimationSetFrame", "1.3", "SetTimeSliderState");
    return visit_SetTimeSliderState(self, args);
}

// ****************************************************************************
// Function: visit_SetActiveTimeSlider
//
// Purpose: 
//   Sets the active time slider in the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:13:29 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Mar 31 11:21:10 PDT 2004
//   I removed the invalid time slider check so it can be handled more
//   appropriately in the viewer.
//
// ****************************************************************************

STATIC PyObject *
visit_SetActiveTimeSlider(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    //
    // Get the name of the time slider that we want to use.
    //
    char *tsName = NULL;
    if(!PyArg_ParseTuple(args, "s", &tsName))
        return NULL;

    MUTEX_LOCK();
        viewer->SetActiveTimeSlider(tsName);
        if(logging)
            fprintf(logFile, "SetActiveTimeSlider(\"%s\")\n", tsName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_GetActiveTimeSlider
//
// Purpose: 
//   Returns the active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 19 11:05:01 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetActiveTimeSlider(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const WindowInformation *wi = viewer->GetWindowInformation();
    std::string activeTS("");
    if(wi->GetActiveTimeSlider() >= 0)
         activeTS = wi->GetTimeSliders()[wi->GetActiveTimeSlider()];
    return PyString_FromString(activeTS.c_str());
}

// ****************************************************************************
// Method: visit_GetTimeSliders
//
// Purpose: 
//   Returns a dictionary containing the time sliders with their states.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 19 11:15:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetTimeSliders(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const WindowInformation *wi = viewer->GetWindowInformation();
    const stringVector &timeSliders = wi->GetTimeSliders();
    const intVector &timeSliderStates = wi->GetTimeSliderCurrentStates();

    // Create a dictionary object and put the time sliders in it.
    PyObject *dict = PyDict_New();
    for(int i = 0; i < timeSliders.size(); ++i)
    {
        PyObject *tsName = PyString_FromString(timeSliders[i].c_str());
        PyObject *tsValue = PyInt_FromLong(timeSliderStates[i]);
        PyDict_SetItem(dict, tsName, tsValue);
    }

    return dict;
}

// ****************************************************************************
// Function: visit_TimeSliderGetNStates
//
// Purpose: 
//   Returns the number of states for the active time slider.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:16:26 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 7 14:24:26 PST 2004
//   I added support for getting the number of states when we're in keyframing
//   mode.
//
// ****************************************************************************

STATIC PyObject *
visit_TimeSliderGetNStates(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    //
    // Get the number of states for the active time slider.
    //
    WindowInformation *wi = viewer->GetWindowInformation();
    const std::string &ts = wi->GetTimeSliders()[wi->GetActiveTimeSlider()];
    int nStates = 1;
    if(viewer->GetKeyframeAttributes()->GetEnabled() && ts == "Keyframe animation")
        nStates = viewer->GetKeyframeAttributes()->GetNFrames();
    else
    {
        DatabaseCorrelationList *correlations = viewer->GetDatabaseCorrelationList();
        DatabaseCorrelation *c = correlations->FindCorrelation(ts);
        if(c != 0)
            nStates = c->GetNumStates();
    }

    // Return the success value.
    return PyLong_FromLong(long(nStates));
}

STATIC PyObject *
visit_AnimationGetNFrames(PyObject *self, PyObject *args)
{
    DeprecatedMessage("AnimationGetNFrames", "1.3", "TimeSliderGetNStates");
    return visit_TimeSliderGetNStates(self, args);
}

// ****************************************************************************
// Function: visit_AnimationSetNFrames
//
// Purpose: 
//   Sets the number of frames in the animation.
//
// Notes:
//
// Programmer: Eric Brugger
// Creation:   Mon Dec 16 09:58:27 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Dec 30 13:11:48 PST 2002
//   I made it be synchronized.
//
// ****************************************************************************

STATIC PyObject *
visit_AnimationSetNFrames(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int nFrames;
    if(!PyArg_ParseTuple(args, "i", &nFrames))
        return NULL;
    if(nFrames < 0)
        return NULL;

    MUTEX_LOCK();
        viewer->AnimationSetNFrames(nFrames);
        if(logging)
            fprintf(logFile, "AnimationSetNFrames(%d)\n", nFrames);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetWindowLayout
//
// Purpose:
//   Tells the viewer to change the window layout
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:07:34 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_SetWindowLayout(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int winLayout;
    if (!PyArg_ParseTuple(args, "i", &winLayout))
       return NULL;

    MUTEX_LOCK();
        viewer->SetWindowLayout(winLayout);
        if(logging)
            fprintf(logFile, "SetWindowLayout(%d)\n", winLayout);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetActiveWindow
//
// Purpose:
//   Tells the viewer to set a new active window
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:08:35 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_SetActiveWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int activewin;
    if (!PyArg_ParseTuple(args, "i", &activewin))
       return NULL;

    MUTEX_LOCK();
        viewer->SetActiveWindow(activewin);
        if(logging)
            fprintf(logFile, "SetActiveWindow(%d)\n", activewin);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_IconifyAllWindows
//
// Purpose:
//   Tells the viewer to iconify all the windows.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_IconifyAllWindows(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->IconifyAllWindows();
        if(logging)
            fprintf(logFile, "IconifyAllWindows()\n");
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_DeIconifyAllWindows
//
// Purpose:
//   Tells the viewer to show all the iconified windows
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_DeIconifyAllWindows(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->DeIconifyAllWindows();
        if(logging)
            fprintf(logFile, "DeIconifyAllWindows()\n");
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_OpenDatabase
//
// Purpose:
//   Tells the viewer to open a database.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 11:52:03 PDT 2002
//   I made the function return a value that indicates the success of the
//   OpenDatabase operation.
//
//   Mark C. Miller, Wed Mar 17 10:15:11 PST 2004
//   Added support for optional second argument indicating the time index
//   to open the database at.
//
//   Brad Whitlock, Tue Mar 2 10:19:42 PDT 2004
//   I removed the code to set the animation time state to 0.
//
// ****************************************************************************

STATIC PyObject *
visit_OpenDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    int timeIndex = 0;
    if (!PyArg_ParseTuple(args, "s", &fileName))
    {
        if(!PyArg_ParseTuple(args, "si", &fileName, &timeIndex))
            return NULL;
        else
            PyErr_Clear();
    }

    // Open the database.
    MUTEX_LOCK();
        viewer->OpenDatabase(fileName, timeIndex);
        if(logging)
        {
            fprintf(logFile, "OpenDatabase(\"%s\", %d)\n", fileName,
                    timeIndex);
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ReOpenDatabase
//
// Purpose:
//   Tells the viewer to re-open a database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 14:37:57 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Mar 2 10:20:17 PDT 2004
//   I removed the code to set the animation time state to 0.
//
// ****************************************************************************

STATIC PyObject *
visit_ReOpenDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
       return NULL;

    // Open the database.
    MUTEX_LOCK();
        viewer->ReOpenDatabase(fileName);
        if(logging)
            fprintf(logFile, "ReOpenDatabase(\"%s\")\n", fileName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_OverlayDatabase
//
// Purpose:
//   Tells the viewer to overlay a database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 18:05:58 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:11:16 PDT 2002
//   I made it return a success value.
//
//   Brad Whitlock, Tue Mar 2 10:30:38 PDT 2004
//   I removed the code to set the time state.
//
// ****************************************************************************

STATIC PyObject *
visit_OverlayDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
       return NULL;

    // Overlay the database.
    MUTEX_LOCK();
        viewer->OverlayDatabase(fileName);
        if(logging)
            fprintf(logFile, "OverlayDatabase(\"%s\")\n", fileName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ReplaceDatabase
//
// Purpose:
//   Tells the viewer to replace a database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 18:07:42 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:11:27 PDT 2002
//   I made it return a success value.
//
//   Brad Whitlock, Wed Oct 15 16:38:39 PST 2003
//   I made it accept an optional timestate argument.
//
//   Brad Whitlock, Tue Mar 2 10:31:19 PDT 2004
//   I removed the code to set the animation time state.
//
// ****************************************************************************

STATIC PyObject *
visit_ReplaceDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    int timeState = 0;
    if (!PyArg_ParseTuple(args, "si", &fileName, &timeState))
    {
       if (!PyArg_ParseTuple(args, "s", &fileName))
           return NULL;
       else
           PyErr_Clear();
    }

    // Replace the database.
    MUTEX_LOCK();
        viewer->ReplaceDatabase(fileName, timeState);
        if(logging)
        {
            fprintf(logFile, "ReplaceDatabase(\"%s\", %d)\n",
                    fileName, timeState);
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ActivateDatabase
//
// Purpose: 
//   Tells the viewer to make the specified database be the active source.
//   This is not quite as much work as opening the database so the database
//   must already be open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 10:21:24 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ActivateDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
       return NULL;

    // Activate the database.
    MUTEX_LOCK();
        viewer->ActivateDatabase(fileName);
        if(logging)
            fprintf(logFile, "ActivateDatabase(\"%s\")\n", fileName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CheckForNewStates
//
// Purpose: 
//   Tells the viewer to look for new time states for the specified database.
//   This function is meant to be a cheaper version of Reopen that does not
//   mess with the plots or metadata but just adds new time states to the 
//   database.     
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 10:22:34 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_CheckForNewStates(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
       return NULL;

    // Check the database for new states.
    MUTEX_LOCK();
        viewer->CheckForNewStates(fileName);
        if(logging)
            fprintf(logFile, "CheckForNewStates(\"%s\")\n", fileName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CloseDatabase
//
// Purpose: 
//   Tells the viewer to close the specified database.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 18 14:02:38 PST 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_CloseDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
       return NULL;

    // Close the database.
    MUTEX_LOCK();
        viewer->CloseDatabase(fileName);
        if(logging)
            fprintf(logFile, "CloseDatabase(\"%s\")\n", fileName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CreateDatabaseCorrelation
//
// Purpose: 
//   Creates a new database correlation that involves the specified databases
//   and uses the specified correlation method.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:47:25 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_CreateDatabaseCorrelation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int method;
    char *name = 0;
    PyObject *dbTuple = 0;
    if (!PyArg_ParseTuple(args, "sOi", &name, &dbTuple, &method))
        return NULL;

    if(method < 0 || method > 3)
    {
        fprintf(stderr,
        "The correlation method must be one of the following integer values:\n"
        "   0 - IndexForIndexCorrelation\n"
        "   1 - StretchedIndexCorrelation\n"
        "   2 - TimeCorrelation\n"
        "   3 - CycleCorrelation\n");
        return NULL;
    }

    //
    // Get the string vector containing the dbs from the PyObject.
    //
    stringVector dbs;
    if(!GetStringVectorFromPyObject(dbTuple, dbs))
        return NULL;

    // Create the database correlation
    MUTEX_LOCK();
        viewer->CreateDatabaseCorrelation(name, dbs, method);
        if(logging)
        {
            fprintf(logFile, "CreateDatabaseCorrelation(\"%s\",(", name);
            for(int i = 0; i < dbs.size(); ++i)
            {
                fprintf(logFile, "%s", dbs[i].c_str());
                if(i < dbs.size() - 1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "), %d)\n", method);
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_AlterDatabaseCorrelation
//
// Purpose: 
//   Changes the definition for the named database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:46:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_AlterDatabaseCorrelation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int method;
    char *name = 0;
    PyObject *dbTuple = 0;
    if (!PyArg_ParseTuple(args, "sOi", &name, &dbTuple, &method))
        return NULL;

    if(method < 0 || method > 3)
    {
        fprintf(stderr,
        "The correlation method must be one of the following integer values:\n"
        "   0 - IndexForIndexCorrelation\n"
        "   1 - StretchedIndexCorrelation\n"
        "   2 - TimeCorrelation\n"
        "   3 - CycleCorrelation\n");
        return NULL;
    }

    //
    // Get the string vector containing the dbs from the PyObject.
    //
    stringVector dbs;
    if(!GetStringVectorFromPyObject(dbTuple, dbs))
        return NULL;

    // Alter the database correlation
    MUTEX_LOCK();
        viewer->AlterDatabaseCorrelation(name, dbs, method);
        if(logging)
        {
            fprintf(logFile, "AlterDatabaseCorrelation(\"%s\",(", name);
            for(int i = 0; i < dbs.size(); ++i)
            {
                fprintf(logFile, "%s", dbs[i].c_str());
                if(i < dbs.size() - 1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "), %d)\n", method);
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_DeleteDatabaseCorrelation
//
// Purpose: 
//   Deletes the named database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 2 09:45:53 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_DeleteDatabaseCorrelation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *name = 0;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    // Delete the database correlation
    MUTEX_LOCK();
        viewer->DeleteDatabaseCorrelation(name);
        if(logging)
            fprintf(logFile, "DeleteDatabaseCorrelation(\"%s\")\n", name);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_SetDatabaseCorrelationOptions
//
// Purpose: 
//   Set the default correlation options.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 19 17:42:14 PST 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetDatabaseCorrelationOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int defaultCorrelationMethod;  // 0 = PaddedIndex
    int whenToCorrelate = 2;       // only if same length
    if (!PyArg_ParseTuple(args, "ii", &defaultCorrelationMethod,
        &whenToCorrelate))
        return NULL;

    if(defaultCorrelationMethod < 0 || defaultCorrelationMethod > 3 ||
       whenToCorrelate < 0 || whenToCorrelate > 2)
    {
        fprintf(stderr,
        "The correlation method must be one of the following integer values:\n"
        "   0 - IndexForIndexCorrelation\n"
        "   1 - StretchedIndexCorrelation\n"
        "   2 - TimeCorrelation\n"
        "   3 - CycleCorrelation\n"
        "\n"
        "VisIt must be told when you want to automatically correlate:\n"
        "   0 - CorrelateAlways\n"
        "   1 - CorrelateNever\n"
        "   2 - CorrelateOnlyIfSameLength\n"); 
        return NULL;
    }

    // Set the default correlation options.
    MUTEX_LOCK();
        // Let VisIt automatically correlate since we're likely running
        // -nowin and we can't prompt the user.
        DatabaseCorrelationList *cL = viewer->GetDatabaseCorrelationList();
        cL->SetNeedPermission(false);
        cL->SetDefaultCorrelationMethod(defaultCorrelationMethod);
        cL->SetWhenToCorrelate((DatabaseCorrelationList::WhenToCorrelate)
            whenToCorrelate);
        cL->Notify();
        if(logging)
        {
            fprintf(logFile, "SetDatabaseCorrelationOptions(%d, %d)\n",
                    defaultCorrelationMethod, whenToCorrelate);
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: ExpressionDefinitionHelper
//
// Purpose:
//   Tells the viewer to create a new expression.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Jun 28 15:52:21 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Fri Aug 16 10:05:32 PDT 2002
//    At some point this stopped working.  I added the calls to match
//    what the GUI was doing and tell the viewer about our expressions.
//
//    Brad Whitlock, Tue Nov 19 12:37:10 PDT 2002
//    I moved the code into this more general function.
//
//    Sean Ahern, Tue Nov 19 15:51:38 PST 2002
//    Moved ENSURE_VIEWER_EXISTS in here, just to tidy things up a bit.
//
// ****************************************************************************

PyObject *
ExpressionDefinitionHelper(PyObject *args, const char *name, Expression::ExprType t)
{
    ENSURE_VIEWER_EXISTS();

    char *exprName;
    char *exprDef;
    if (!PyArg_ParseTuple(args, "ss", &exprName, &exprDef))
        return NULL;

    // Access the expression list and add a new one.
    MUTEX_LOCK();

        ExpressionList *list = viewer->GetExpressionList();
        Expression *e = new Expression();
        e->SetName(exprName);
        e->SetDefinition(exprDef);
        e->SetType(t);
        list->AddExpression(*e);
        list->Notify();
        viewer->ProcessExpressions();

        if (logging)
            fprintf(logFile, "%s(\"%s\", \"%s\")\n", name, exprName, exprDef);

    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_DefineScalarExpression
//
// Purpose:
//   Tells the viewer to create a new expression.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Jun 28 15:52:21 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Fri Aug 16 10:05:32 PDT 2002
//    At some point this stopped working.  I added the calls to match
//    what the GUI was doing and tell the viewer about our expressions.
//
//    Brad Whitlock, Tue Nov 19 12:38:13 PDT 2002
//    I moved the code into ExpressionDefinitionHelper and added several
//    new expression creation functions.
//
//    Brad Whitlock, Tue May 20 15:21:08 PST 2003
//    Made it work with the regenerated Expression state object.
//
// ****************************************************************************

STATIC PyObject *
visit_DefineScalarExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineScalarExpression", Expression::ScalarMeshVar);
}

STATIC PyObject *
visit_DefineVectorExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineVectorExpression", Expression::VectorMeshVar);
}

STATIC PyObject *
visit_DefineMeshExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineMeshExpression", Expression::Mesh);
}

STATIC PyObject *
visit_DefineMaterialExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineMaterialExpression", Expression::Material);
}

STATIC PyObject *
visit_DefineSpeciesExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineSpeciesExpression", Expression::Species);
}

// ****************************************************************************
// Function: visit_DeleteExpression
//
// Purpose:
//   Tells the viewer to delete an expression.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Jun 28 16:08:14 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_DeleteExpression(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *exprName;
    if (!PyArg_ParseTuple(args, "s", &exprName))
       return NULL;

    // Access the expression list and add a new one.
    MUTEX_LOCK();

        ExpressionList *list = viewer->GetExpressionList();
        for(int i=0;i<list->GetNumExpressions();i++)
        {
            if (strcmp((*list)[i].GetName().c_str(),exprName) == 0)
            {
                list->RemoveExpression(i);
                break;
            }
        }

        if (logging)
            fprintf(logFile, "DeleteExpression(\"%s\")\n", exprName);

    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: OpenComponentHelper
//
// Purpose:
//   Tells the viewer to open a compute engine or an mdserver.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:10:20 PDT 2002
//   I made it return a success value.
//
//   Brad Whitlock, Fri Dec 27 10:53:27 PDT 2002
//   I made it use a stringVector.
//
//   Brad Whitlock, Mon Jan 13 10:57:56 PDT 2003
//   I renamed the function to OpenComponentHelper and I made it 
//   also responsible for telling the viewer to open mdservers.
//
//   Brad Whitlock, Tue Mar 2 09:55:05 PDT 2004
//   I made it use GetStringVectorFromPyObject.
//
// ****************************************************************************

STATIC PyObject *
OpenComponentHelper(PyObject *self, PyObject *args, bool openEngine)
{
    ENSURE_VIEWER_EXISTS();

    static const char *OCEError = "Arguments must be: hostName, arg | (args...)";
    char  *hostName;
    char  *arg1;
    stringVector argv;

    if (!PyArg_ParseTuple(args, "s", &hostName))
    {
        if (!PyArg_ParseTuple(args, "ss", &hostName, &arg1))
        {
            PyObject *tuple;
            if (!PyArg_ParseTuple(args, "sO", &hostName, &tuple))
            {
                if(!PyArg_ParseTuple(args, ""))
                {
                    VisItErrorFunc(OCEError);
                    return NULL;
                }

                // Provide a default engine name.
                hostName = "localhost";
            }
            else
            {
                // Make sure it's a tuple.
                if(!GetStringVectorFromPyObject(tuple, argv))
                {
                    VisItErrorFunc(OCEError);
                    return NULL;
                }
            }
        }
        else
        {
            // Store arg1 in the argument vector.
            argv.push_back(arg1);
        }

        PyErr_Clear();
    }

    MUTEX_LOCK();
        // Either open an engine or an mdserver.
        const char *methodName = "OpenComputeEngine";
        if(openEngine)
            viewer->OpenComputeEngine(hostName, argv);
        else
        {
            viewer->OpenMDServer(hostName, argv);
            methodName = "OpenMDServer";
        }

        if(logging)
        {
            if(argv.size() == 0)
                fprintf(logFile, "%s(\"%s\")\n", methodName, hostName);
            else if(argv.size() == 1)
            {
                fprintf(logFile, "%s(\"%s\", \"%s\")\n",
                        methodName, hostName, argv[0].c_str());
            }
            else
            {
                fprintf(logFile, "launchArguments = (");
                for(int i = 0; i < argv.size(); ++i)
                {
                    fprintf(logFile, "\"%s\"", argv[i].c_str());
                    if(i < argv.size() - 1)
                        fprintf(logFile, ", ");
                }
                fprintf(logFile, ")\n");
                fprintf(logFile, "%s(\"%s\", launchArguments)\n",
                        methodName, hostName);
            }
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

STATIC PyObject *
visit_OpenComputeEngine(PyObject *self, PyObject *args)
{
    return OpenComponentHelper(self, args, true);
}

STATIC PyObject *
visit_OpenMDServer(PyObject *self, PyObject *args)
{
    return OpenComponentHelper(self, args, false);
}

// ****************************************************************************
// Function: visit_InvertBackgroundColor
//
// Purpose:
//   Tells the viewer to swap its background and foreground colors.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_InvertBackgroundColor(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->InvertBackgroundColor();
        viewer->RedrawWindow();
        if(logging)
            fprintf(logFile, "InvertBackgroundColor()\n");
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_AddPlot
//
// Purpose:
//   Tells the viewer to add a plot of the specified type.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:16:56 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_AddPlot(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *plotName;
    char *varName;
    if (!PyArg_ParseTuple(args, "ss", &plotName, &varName))
       return NULL;

    // Find the plot index from the name. Throw a python exception if we are
    // allowing python exceptions and the plot index is -1.
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    int plotTypeIndex = -1;
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info=pluginManager->GetScriptingPluginInfo(id);
        if(info == 0)
            continue;

        if(info->GetName() == std::string(plotName))
        {
            plotTypeIndex = i;
            break;
        }
    }

    // If the plot type was not found, return.
    if(plotTypeIndex < 0)
    {
        VisItErrorFunc("Invalid plot plugin name!");
        return NULL;
    }
   
    MUTEX_LOCK();
        viewer->AddPlot(plotTypeIndex, varName);
        if(logging)
            fprintf(logFile, "AddPlot(\"%s\", \"%s\")\n", plotName, varName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_AddOperator
//
// Purpose:
//   Tells the viewer to add an operator to the selected plots.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 09:49:25 PDT 2002
//   Added an optional second argument that allows the operator to apply to
//   all plots.
//
//   Brad Whitlock, Fri Jul 26 12:17:54 PDT 2002
//   I made it return a sucess value.
//
// ****************************************************************************

STATIC PyObject *
visit_AddOperator(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *operName;
    int  applyToAllPlots = 0;
    if (!PyArg_ParseTuple(args, "s", &operName))
    {
        if(!PyArg_ParseTuple(args, "si", &operName, &applyToAllPlots))
            return NULL;
        else
            PyErr_Clear();
    }

    // Find the oper index from the name. Throw a python exception if we are
    // allowing python exceptions and the operator index is -1.
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    int operTypeIndex = -1;
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info=pluginManager->GetScriptingPluginInfo(id);
        if(info == 0)
            continue;

        if(info->GetName() == std::string(operName))
        {
            operTypeIndex = i;
            break;
        }
    }

    // If the operator type was not found, return.
    if(operTypeIndex < 0)
    {
        VisItErrorFunc("Invalid operator plugin name");
        return NULL;
    }

    MUTEX_LOCK();
        // Set the apply to all plots toggle.
        viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        viewer->GetGlobalAttributes()->Notify();

        // Add the operator
        viewer->AddOperator(operTypeIndex);
        if(logging)
        {
            fprintf(logFile, "AddOperator(\"%s\", %d)\n", operName,
                    applyToAllPlots);
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_DrawPlots
//
// Purpose:
//   Tells the viewer to draw the plots that have not been realized.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:19:47 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_DrawPlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->DrawPlots();
        if(logging)
            fprintf(logFile, "DrawPlots()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CloseComputeEngine
//
// Purpose: 
//   This is a Python callback function that closes a compute engine.
//
// Note:       The hostname of the engine can be omitted and the first engine
//             in the engine list will be closed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 20 12:47:31 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:20:12 PDT 2002
//   I made it return a success value.
//
//   Jeremy Meredith, Tue Mar 30 11:09:06 PST 2004
//   I added support for simulations.
//
// ****************************************************************************

STATIC PyObject *
visit_CloseComputeEngine(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    bool useFirstEngine = false;
    bool useFirstSimulation = false;
    const char *engineName = 0;
    const char *simulationName = 0;
    if(!PyArg_ParseTuple(args, "ss", &engineName, &simulationName))
    {
        if (!PyArg_ParseTuple(args, "s", &engineName))
        {
            if(!PyArg_ParseTuple(args, ""))
                return NULL;

            PyErr_Clear();
            // Indicate that we want to close the first engine in the list.
            useFirstEngine = true;
        }
        else
        {
            PyErr_Clear();
            // Indicate that we want to close the first simulation on that host
            useFirstSimulation = true;
        }
    }

    MUTEX_LOCK();
         if(useFirstEngine)
         {
             const stringVector &engines = viewer->GetEngineList()->GetEngines();
             const stringVector &sims = viewer->GetEngineList()->GetSimulationName();
             if(engines.size() > 0)
             {
                 engineName = engines[0].c_str();
                 simulationName = sims[0].c_str();
             }
         }
         else if (useFirstSimulation)
         {
             const stringVector &engines = viewer->GetEngineList()->GetEngines();
             const stringVector &sims = viewer->GetEngineList()->GetSimulationName();
             for (int i=0; i<engines.size(); i++)
             {
                 if (engines[i] == engineName)
                 {
                     simulationName = sims[i].c_str();
                     break;
                 }
             }
         }

         if (engineName != 0 && simulationName != 0)
         {
             viewer->CloseComputeEngine(engineName, simulationName);

            // Write the command to the file.
            if(logging)
            {
                fprintf(logFile, "CloseComputeEngine(\"%s\",\"%s\")\n",
                        engineName, simulationName);
            }
         }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetPlotFrameRange
//
// Purpose:
//   This is a Python callback that can be used to set the frame range for
//   the specified plot.
//
// Note:
//
// Programmer: Eric Brugger
// Creation:   Tue Nov 19 10:53:43 PST 2002
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_SetPlotFrameRange(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    int plotId, frame0, frame1;
    if(!PyArg_ParseTuple(args, "iii", &plotId, &frame0, &frame1))
        return NULL;
 
    viewer->SetPlotFrameRange(plotId, frame0, frame1);
 
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_DeletePlotKeyframe
//
// Purpose:
//   This is a Python callback that can be used to delete a keyframe for the
//   specified plot.
//
// Note:
//
// Programmer: Eric Brugger
// Creation:   Tue Nov 19 10:53:43 PST 2002
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_DeletePlotKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    int plotId, frame;
    if(!PyArg_ParseTuple(args, "ii", &plotId, &frame))
        return NULL;
 
    viewer->DeletePlotKeyframe(plotId, frame);
 
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_MovePlotKeyframe
//
// Purpose:
//   This is a Python callback that can be used to move a plot keyframe for
//   a specified plot.
//
// Note:
//
// Programmer: Eric Brugger
// Creation:   Tue Jan 28 13:53:33 PST 2003 
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_MovePlotKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    int plotId, oldFrame, newFrame;
    if(!PyArg_ParseTuple(args, "iii", &plotId, &oldFrame, &newFrame))
        return NULL;
 
    viewer->MovePlotKeyframe(plotId, oldFrame, newFrame);
 
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_SetPlotDatabaseState
//
// Purpose:
//   This is a Python callback that can be used to set the database state
//   associated with a specified plot and frame.
//
// Note:
//
// Programmer: Eric Brugger
// Creation:   Mon Dec 30 13:16:58 PST 2002 
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_SetPlotDatabaseState(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    int plotId, frame, state;
    if(!PyArg_ParseTuple(args, "iii", &plotId, &frame, &state))
        return NULL;
 
    viewer->SetPlotDatabaseState(plotId, frame, state);
 
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_DeletePlotDatabaseKeyframe
//
// Purpose:
//   This is a Python callback that can be used to delete a database keyframe
//   for a specified plot.
//
// Note:
//
// Programmer: Eric Brugger
// Creation:   Mon Dec 30 13:16:58 PST 2002 
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_DeletePlotDatabaseKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    int plotId, frame;
    if(!PyArg_ParseTuple(args, "ii", &plotId, &frame))
        return NULL;
 
    viewer->DeletePlotDatabaseKeyframe(plotId, frame);
 
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_MovePlotDatabaseKeyframe
//
// Purpose:
//   This is a Python callback that can be used to move a database keyframe
//   for a specified plot.
//
// Note:
//
// Programmer: Eric Brugger
// Creation:   Tue Jan 28 13:53:33 PST 2003
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_MovePlotDatabaseKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    int plotId, oldFrame, newFrame;
    if(!PyArg_ParseTuple(args, "iii", &plotId, &oldFrame, &newFrame))
        return NULL;
 
    viewer->MovePlotDatabaseKeyframe(plotId, oldFrame, newFrame);
 
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_ChangeActivePlotsVar
//
// Purpose:
//   Tells the viewer to change variables for the selected plots.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:20:58 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_ChangeActivePlotsVar(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *varName;
    if(!PyArg_ParseTuple(args, "s", &varName))
        return NULL;

    MUTEX_LOCK();
         viewer->ChangeActivePlotsVar(varName);

        // Write the command to the file.
        if(logging)
            fprintf(logFile, "ChangeActivePlotsVar(\"%s\")\n", varName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CopyAnnotationsToWindow
//
// Purpose: 
//   Tells the viewer to copy the annotations from one window to another.
//
// Notes:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:55:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:21:57 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_CopyAnnotationsToWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int from, to;
    if(!PyArg_ParseTuple(args, "ii", &from, &to))
        return NULL;

    MUTEX_LOCK();
         viewer->CopyAnnotationsToWindow(from, to);

        // Write the command to the file.
        if(logging)
            fprintf(logFile, "CopyAnnotationsToWindow(%d, %d)\n", from, to);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CopyLightingToWindow
//
// Purpose: 
//   Tells the viewer to copy the lighting from one window to another.
//
// Notes:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:55:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:22:42 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_CopyLightingToWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int from, to;
    if(!PyArg_ParseTuple(args, "ii", &from, &to))
        return NULL;

    MUTEX_LOCK();
         viewer->CopyLightingToWindow(from, to);

        // Write the command to the file.
        if(logging)
            fprintf(logFile, "CopyLightingToWindow(%d, %d)\n", from, to);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CopyViewToWindow
//
// Purpose: 
//   Tells the viewer to copy the view from one window to another.
//
// Notes:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:55:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:23:11 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_CopyViewToWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int from, to;
    if(!PyArg_ParseTuple(args, "ii", &from, &to))
        return NULL;

    MUTEX_LOCK();
         viewer->CopyViewToWindow(from, to);

        // Write the command to the file.
        if(logging)
            fprintf(logFile, "CopyViewToWindow(%d, %d)\n", from, to);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_CopyPlotsToWindow
//
// Purpose: 
//   Tells the viewer to copy the plots from one window to another.
//
// Notes:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 16 10:19:33 PDT 2002
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_CopyPlotsToWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int from, to;
    if(!PyArg_ParseTuple(args, "ii", &from, &to))
        return NULL;

    MUTEX_LOCK();
         viewer->CopyPlotsToWindow(from, to);

        // Write the command to the file.
        if(logging)
            fprintf(logFile, "CopyPlotsToWindow(%d, %d)\n", from, to);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_DeleteActivePlots
//
// Purpose: 
//   This is a Python callback that deletes all active plots in the plot list.
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Thu Sep 20 17:11:32 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:23:44 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_DeleteActivePlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->DeleteActivePlots();
        if(logging)
            fprintf(logFile, "DeleteActivePlots()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_DeleteAllPlots
//
// Purpose: 
//   This is a Python callback that deletes all plots in the plot list.
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Thu Sep 20 17:11:32 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:23:44 PDT 2002
//   I made it return a success value.
//
//   Brad Whitlock, Fri Dec 27 10:57:38 PDT 2002
//   I made it use an intVector.
//
// ****************************************************************************

STATIC PyObject *
visit_DeleteAllPlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        //
        // First set all plots active.
        //
        int nPlots = viewer->GetPlotList()->GetNumPlots();
        intVector plots;
        for(int i = 0; i < nPlots; ++i)
            plots.push_back(i);
        viewer->SetActivePlots(plots);

        viewer->DeleteActivePlots();
        if(logging)
            fprintf(logFile, "DeleteAllPlots()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_RemoveLastOperator
//
// Purpose:
//   Tells the viewer to remove the last operator from the selected plots.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 10:42:18 PDT 2002
//   Added an optional argument to remove the operator from all plots.
//
//   Brad Whitlock, Fri Jul 26 12:23:44 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_RemoveLastOperator(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int applyToAllPlots = 0;
    if(!PyArg_ParseTuple(args, "i", &applyToAllPlots))
        PyErr_Clear();

    MUTEX_LOCK();
        // Set the apply to all plots toggle.
        viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        viewer->GetGlobalAttributes()->Notify();

        // Remove the last operator.
        viewer->RemoveLastOperator();
        if(logging)
            fprintf(logFile, "RemoveLastOperator(%d)\n", applyToAllPlots);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_RemoveAllOperators
//
// Purpose:
//   Tells the viewer to remove all operators from the selected plots.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 10:41:15 PDT 2002
//   Added an optional argument to remove operators from all plots.
//
//   Brad Whitlock, Fri Jul 26 12:23:44 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_RemoveAllOperators(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int applyToAllPlots = 0;
    if(!PyArg_ParseTuple(args, "i", &applyToAllPlots))
        PyErr_Clear();

    MUTEX_LOCK();
        // Set the apply to all plots toggle.
        viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        viewer->GetGlobalAttributes()->Notify();

        // Remove all operators.
        viewer->RemoveAllOperators();
        if(logging)
            fprintf(logFile, "RemoveAllOperators(%d)\n", applyToAllPlots);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ClearWindow
//
// Purpose:
//   Tells the viewer to clear the window of plots.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ClearWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ClearWindow();
        if(logging)
            fprintf(logFile, "ClearWindow()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ClearAllWindows
//
// Purpose:
//   Tells the viewer to clear all of its windows.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ClearAllWindows(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ClearAllWindows();
        if(logging)
            fprintf(logFile, "ClearAllWindows()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ClearCache
//
// Purpose:
//   Tells the viewer to clear the cache for the compute engine running on
//   the specified host.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 13:55:52 PST 2002
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 11:10:48 PST 2004
//    Added support for simulations.
//
//    Brad Whitlock, Thu Apr 1 11:40:23 PDT 2004
//    I fixed an argument passing bug.
//
// ****************************************************************************

STATIC PyObject *
visit_ClearCache(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *engineName = 0;
    char *simulationName = 0;
    if(!PyArg_ParseTuple(args, "ss", &engineName, &simulationName))
    {
        if (!PyArg_ParseTuple(args, "s", &engineName))
            return NULL;
        simulationName = "";
        PyErr_Clear();
    }

    MUTEX_LOCK();
        viewer->ClearCache(engineName, simulationName);
        if(logging)
            fprintf(logFile, "ClearCache(\"%s\",\"%s\")\n",
                    engineName, simulationName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ClearCacheForAllEngines
//
// Purpose:
//   Tells the viewer to clear the cache for all compute engines.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 13:39:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ClearCacheForAllEngines(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ClearCacheForAllEngines();
        if(logging)
            fprintf(logFile, "ClearCacheForAllEngines()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ClearPickPoints
//
// Purpose:
//   Tells the viewer to clear all pick points.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ClearPickPoints(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ClearPickPoints();
        viewer->RedrawWindow();
        if(logging)
            fprintf(logFile, "ClearPickPoints()\n");
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_ClearReferenceLines
//
// Purpose:
//   Tells the viewer to clear all reference lines.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ClearReferenceLines(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ClearReferenceLines();
        viewer->RedrawWindow();
        if(logging)
            fprintf(logFile, "ClearReferenceLines()\n");
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_SaveWindow
//
// Purpose:
//   Tells the viewer to save the active window.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Thu Jul 25 11:46:02 PDT 2002
//    Added code to get the last real filename to use as a return value.
//
// ****************************************************************************

STATIC PyObject *
visit_SaveWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->SaveWindow();
        if(logging)
            fprintf(logFile, "SaveWindow()\n");
    MUTEX_UNLOCK();
    int errorFlag = Synchronize();

    PyObject *retval = NULL;
    if(errorFlag == 0)
    {
        std::string realname =
            viewer->GetSaveWindowAttributes()->GetLastRealFilename();
        retval = PyString_FromString(realname.c_str());
    }

    return retval;
}

// ****************************************************************************
// Function: visit_DeleteWindow
//
// Purpose:
//   Tells the viewer to delete the active window.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:26:26 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_DeleteWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->DeleteWindow();
        if(logging)
            fprintf(logFile, "DeleteWindow()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_DisableRedraw
//
// Purpose:
//   Tells the viewer to disable redrawing.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_DisableRedraw(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->DisableRedraw();
        if(logging)
            fprintf(logFile, "DisableRedraw()\n");
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_RedrawWindow
//
// Purpose:
//   Tells the viewer to redraw the window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RedrawWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->RedrawWindow();
        if(logging)
            fprintf(logFile, "RedrawWindow()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_RecenterView
//
// Purpose:
//   Tells the viewer to recenter the view.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:12:11 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RecenterView(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->RecenterView();
        if(logging)
            fprintf(logFile, "RecenterView()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ResetView
//
// Purpose:
//   Tells the viewer to reset the view.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ResetView(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ResetView();
        if(logging)
            fprintf(logFile, "ResetView()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetCenterOfRotation
//
// Purpose:
//   Sets the center of rotation for the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetCenterOfRotation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    double c0, c1, c2;
    if(!PyArg_ParseTuple(args, "ddd", &c0, &c1, &c2))
        return NULL;

    MUTEX_LOCK();
        viewer->SetCenterOfRotation(c0, c1, c2);
        if(logging)
        {
            fprintf(logFile, "SetCenterOfRotation(%g, %g, %g)\n",
                    c0, c1, c2);
        }
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ChooseCenterOfRotation
//
// Purpose:
//   Tells the viewer to use the center of the screen to update the center
//   of rotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ChooseCenterOfRotation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    bool havePoint = true;
    double sx, sy;
    if(!PyArg_ParseTuple(args, "dd", &sx, &sy))
    {
        havePoint = false;
        PyErr_Clear(); 
    }
    

    MUTEX_LOCK();
        if(havePoint)
        {
            // We know where we want to pick.
            viewer->ChooseCenterOfRotation(sx, sy);
            if(logging)
                fprintf(logFile, "ChooseCenterOfRotation(%g ,%g)\n", sx, sy);
        }
        else
        {
            // Choose the point interactively
            viewer->ChooseCenterOfRotation();
            if(logging)
                fprintf(logFile, "ChooseCenterOfRotation()\n");
        }
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_RestoreSession
//
// Purpose:
//   Tells the viewer to read in a session file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 30 14:36:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RestoreSession(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *filename;    
    int sessionStoredInVisItDir = 1;
    if(!PyArg_ParseTuple(args, "si", &filename, &sessionStoredInVisItDir))
        return NULL;

    MUTEX_LOCK();
        viewer->ImportEntireState(filename, sessionStoredInVisItDir!=0);
        if(logging)
            fprintf(logFile, "RestoreSession(%s, %d)\n", filename,
                    sessionStoredInVisItDir);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SaveSession
//
// Purpose:
//   Tells the viewer to save the session to the named session file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 30 14:43:02 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SaveSession(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *filename;    
    if(!PyArg_ParseTuple(args, "s", &filename))
        return NULL;

    MUTEX_LOCK();
        viewer->ExportEntireState(filename);
        if(logging)
            fprintf(logFile, "SaveSession(%s)\n", filename);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetEngineList
//
// Purpose:
//   Returns a tuple containing the names of the hosts that we're running
//   engines on.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetEngineList(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    // Allocate a tuple the with enough entries to hold the engine list.
    const stringVector &engines = viewer->GetEngineList()->GetEngines();
    PyObject *retval = PyTuple_New(engines.size());
    for(int i = 0; i < engines.size(); ++i)
    {
        PyObject *name = PyString_FromString(engines[i].c_str());
        if(name == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, name);
    }

    return retval;
}

// ****************************************************************************
// Function: visit_HideActivePlots
//
// Purpose:
//   Tells the viewer to hide the active plots.
//
// Notes:      
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:27:51 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_HideActivePlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->HideActivePlots();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_HideToolbars
//
// Purpose: 
//   Hides the toolbars for the active window or for all windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 11:18:05 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_HideToolbars(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    int hideForAllWindows = 0;

    if (!PyArg_ParseTuple(args, "i", &hideForAllWindows))
    {
        PyErr_Clear();
    }

    MUTEX_LOCK();
        // If we passed an argument then hide toolbars for all windows.
        viewer->HideToolbars(hideForAllWindows != 0);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ShowToolbars
//
// Purpose: 
//   Shows the toolbars for the active window or for all windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 11:18:05 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ShowToolbars(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    int showForAllWindows = 0;

    if (!PyArg_ParseTuple(args, "i", &showForAllWindows))
    {
        PyErr_Clear();
    }

    MUTEX_LOCK();
        // If we passed an argument then show toolbars for all windows.
        viewer->ShowToolbars(showForAllWindows != 0);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetAnnotationAttributes
//
// Purpose:
//   Tells the viewer to use the new annotation attributes we're sending.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetAnnotationAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *annot = NULL;
    // Try and get the annotation pointer.
    if(!PyArg_ParseTuple(args,"O",&annot))
    {
        VisItErrorFunc("SetAnnotationAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyAnnotationAttributes_Check(annot))
    {
        VisItErrorFunc("Argument is not a AnnotationAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        AnnotationAttributes *va = PyAnnotationAttributes_FromPyObject(annot);

        // Copy the object into the view attributes.
        *(viewer->GetAnnotationAttributes()) = *va;
        viewer->GetAnnotationAttributes()->Notify();
        viewer->SetAnnotationAttributes();

        if(logging)
            fprintf(logFile, "SetAnnotationAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetCloneWindowOnFirstRef
//
// Purpose:
//   Tells the viewer to use the clone window on first reference flag.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Thu Dec 18 15:29:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetCloneWindowOnFirstRef(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int flag;
    if (!PyArg_ParseTuple(args, "i", &flag))
       return NULL;

    MUTEX_LOCK();
        viewer->GetGlobalAttributes()->SetCloneWindowOnFirstRef(flag);
        viewer->GetGlobalAttributes()->Notify();
        if(logging)
            fprintf(logFile, "SetCloneWindowOnFirstRef(%d)\n", flag);
    MUTEX_UNLOCK();
    int errorFlag = Synchronize();

    // Return the success value.
    return PyLong_FromLong(long(errorFlag == 0));
}

// ****************************************************************************
// Function: visit_SetDefaultAnnotationAttributes
//
// Purpose:
//   Tells the viewer to save the default annotation attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultAnnotationAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *annot = NULL;
    // Try and get the annotation pointer.
    if(!PyArg_ParseTuple(args,"O",&annot))
    {
        VisItErrorFunc("SetDefaultAnnotationAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyAnnotationAttributes_Check(annot))
    {
        VisItErrorFunc("Argument is not a AnnotationAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        AnnotationAttributes *va = PyAnnotationAttributes_FromPyObject(annot);

        // Copy the object into the view attributes.
        *(viewer->GetAnnotationAttributes()) = *va;
        viewer->GetAnnotationAttributes()->Notify();
        viewer->SetDefaultAnnotationAttributes();

        if(logging)
            fprintf(logFile, "SetDefaultAnnotationAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetAnnotationAttributes
//
// Purpose:
//   Returns the annotation attributes for the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetAnnotationAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyAnnotationAttributes_NewPyObject();
    AnnotationAttributes *aa = PyAnnotationAttributes_FromPyObject(retval);

    // Copy the viewer proxy's annotation atts into the return data structure.
    *aa = *(viewer->GetAnnotationAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetKeyframeAttributes
//
// Purpose:
//   Tells the viewer to use the new keyframe attributes we're sending.
//
// Notes:
//
// Programmer: Eric Brugger
// Creation:   Tue Nov 19 10:53:43 PST 2002
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_SetKeyframeAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    PyObject *keyframe = NULL;
    // Try and get the keyframe pointer.
    if(!PyArg_ParseTuple(args,"O",&keyframe))
    {
        VisItErrorFunc("SetKeyframeAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyKeyframeAttributes_Check(keyframe))
    {
        VisItErrorFunc("Argument is not a KeyframeAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        KeyframeAttributes *va = PyKeyframeAttributes_FromPyObject(keyframe);
 
        // Copy the object into the view attributes.
        *(viewer->GetKeyframeAttributes()) = *va;
        viewer->GetKeyframeAttributes()->Notify();
        viewer->SetKeyframeAttributes();
 
        if(logging)
            fprintf(logFile, "SetKeyframeAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetKeyframeAttributes
//
// Purpose:
//   Returns the keyframe attributes for the active window.
//
// Notes:
//
// Programmer: Eric Brugger
// Creation:   Tue Nov 19 10:53:43 PST 2002
//
// Modifications:
//
// ****************************************************************************
 
STATIC PyObject *
visit_GetKeyframeAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
 
    PyObject *retval = PyKeyframeAttributes_NewPyObject();
    KeyframeAttributes *aa = PyKeyframeAttributes_FromPyObject(retval);
 
    // Copy the viewer proxy's keyframe atts into the return data structure.
    *aa = *(viewer->GetKeyframeAttributes());
 
    return retval;
}

// ****************************************************************************
// Function: visit_SetMaterialAttributes
//
// Purpose:
//   Tells the viewer to use the new material attributes we're sending.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetMaterialAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *mat = NULL;
    // Try and get the material pointer.
    if(!PyArg_ParseTuple(args,"O",&mat))
    {
        VisItErrorFunc("SetMaterialAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyMaterialAttributes_Check(mat))
    {
        VisItErrorFunc("Argument is not a MaterialAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        MaterialAttributes *va = PyMaterialAttributes_FromPyObject(mat);

        // Copy the object into the view attributes.
        *(viewer->GetMaterialAttributes()) = *va;
        viewer->GetMaterialAttributes()->Notify();
        viewer->SetMaterialAttributes();

        if(logging)
            fprintf(logFile, "SetMaterialAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetDefaultMaterialAttributes
//
// Purpose:
//   Tells the viewer to save the default material attributes.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultMaterialAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *mat = NULL;
    // Try and get the material pointer.
    if(!PyArg_ParseTuple(args,"O",&mat))
    {
        VisItErrorFunc("SetDefaultMaterialAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyMaterialAttributes_Check(mat))
    {
        VisItErrorFunc("Argument is not a MaterialAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        MaterialAttributes *va = PyMaterialAttributes_FromPyObject(mat);

        // Copy the object into the view attributes.
        *(viewer->GetMaterialAttributes()) = *va;
        viewer->GetMaterialAttributes()->Notify();
        viewer->SetDefaultMaterialAttributes();

        if(logging)
            fprintf(logFile, "SetDefaultMaterialAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetMaterialAttributes
//
// Purpose:
//   Returns the material attributes for the active window.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetMaterialAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyMaterialAttributes_NewPyObject();
    MaterialAttributes *aa = PyMaterialAttributes_FromPyObject(retval);

    // Copy the viewer proxy's material atts into the return data structure.
    *aa = *(viewer->GetMaterialAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetPrinterAttributes
//
// Purpose:
//   Tells the viewer to use the new printer attributes we're sending.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 14:16:49 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetPrinterAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *print = NULL;
    // Try and get the printer attributes pointer.
    if(!PyArg_ParseTuple(args,"O",&print))
    {
        VisItErrorFunc("SetPrinterAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyPrinterAttributes_Check(print))
    {
        VisItErrorFunc("Argument is not a PrinterAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        PrinterAttributes *va = PyPrinterAttributes_FromPyObject(print);

        // Copy the object into the view attributes.
        *(viewer->GetPrinterAttributes()) = *va;
        viewer->GetPrinterAttributes()->Notify();

        if(logging)
            fprintf(logFile, "SetPrinterAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetSaveWindowAttributes
//
// Purpose:
//   Tells the viewer to use the new save window attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
//   Hank Childs, Fri May 24 08:37:28 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.
//
// ****************************************************************************

STATIC PyObject *
visit_SetSaveWindowAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *annot = NULL;
    // Try and get the annotation pointer.
    if(!PyArg_ParseTuple(args,"O",&annot))
    {
        VisItErrorFunc("SetSaveWindowAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PySaveWindowAttributes_Check(annot))
    {
        VisItErrorFunc("Argument is not a SaveWindowAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        SaveWindowAttributes *va = PySaveWindowAttributes_FromPyObject(annot);

        // Copy the object into the view attributes.
        *(viewer->GetSaveWindowAttributes()) = *va;
        viewer->GetSaveWindowAttributes()->Notify();

        if(logging)
            fprintf(logFile, "SetSaveWindowAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetSaveWindowAttributes
//
// Purpose:
//   Returns the save window attributes that we're using.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
//   Hank Childs, Fri May 24 08:37:28 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.
//
// ****************************************************************************

STATIC PyObject *
visit_GetSaveWindowAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PySaveWindowAttributes_NewPyObject();
    SaveWindowAttributes *aa = PySaveWindowAttributes_FromPyObject(retval);

    // Copy the viewer proxy's saveimage atts into the return data structure.
    *aa = *(viewer->GetSaveWindowAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetViewCurve
//
// Purpose:
//   Tells the viewer to use the new curve view we're giving it.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Wed Aug 20 14:20:25 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetViewCurve(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *view = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&view))
    {
        VisItErrorFunc("SetViewCurve: Cannot parse object!");
        return NULL;
    }
    if(!PyViewCurveAttributes_Check(view))
    {
        VisItErrorFunc("Argument is not a ViewCurveAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        ViewCurveAttributes *va = PyViewCurveAttributes_FromPyObject(view);

        // Copy the object into the view attributes.
        *(viewer->GetViewCurveAttributes()) = *va;
        viewer->GetViewCurveAttributes()->Notify();
        viewer->SetViewCurve();

        if(logging)
            fprintf(logFile, "SetViewCurve()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetViewCurve
//
// Purpose:
//   Returns the curve view.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Fri Aug 15 14:34:27 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetViewCurve(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyViewCurveAttributes_NewPyObject();
    ViewCurveAttributes *va = PyViewCurveAttributes_FromPyObject(retval);

    // Copy the viewer proxy's curve view into the return data structure.
    *va = *(viewer->GetViewCurveAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetView2D
//
// Purpose:
//   Tells the viewer to use the new 2D view we're giving it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Modified to handle a generic view attribute or a 2d view attribute.
//   
// ****************************************************************************

STATIC PyObject *
visit_SetView2D(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *view = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&view))
    {
        VisItErrorFunc("SetView2D: Cannot parse object!");
        return NULL;
    }
    if(PyView2DAttributes_Check(view))
    {
        MUTEX_LOCK();
            View2DAttributes *va = PyView2DAttributes_FromPyObject(view);

            // Copy the object into the view attributes.
            *(viewer->GetView2DAttributes()) = *va;
            viewer->GetView2DAttributes()->Notify();
            viewer->SetView2D();

            if(logging)
                fprintf(logFile, "SetView2D()\n");
        MUTEX_UNLOCK();
    }
    else if (PyViewAttributes_Check(view))
    {
        //
        // This feature is deprecated and the coding to support this
        // should be removed in visit 1.3.  The coding should match that
        // in SetViewCurve.
        //
        MUTEX_LOCK();
            ViewAttributes *va = PyViewAttributes_FromPyObject(view);
            View2DAttributes *v2da = viewer->GetView2DAttributes();

            // Copy the object into the view attributes.
            v2da->SetWindowCoords(va->GetWindowCoords());
            v2da->SetViewportCoords(va->GetViewportCoords());
            v2da->Notify();
            viewer->SetView2D();

            cerr << "Warning: Passing a ViewAttribute to SetView2D is"
                 << " deprecated.  Pass a" << endl
                 << " View2DAttribute instead." << endl;
            if(logging)
                fprintf(logFile, "SetView2D()\n");
        MUTEX_UNLOCK();
    }
    else
    {
        VisItErrorFunc("Argument is not a View2DAttributes object");
        return NULL;
    }

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetView2D
//
// Purpose:
//   Returns the 2D view.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Modify to return a 2d view attribute.
//   
// ****************************************************************************

STATIC PyObject *
visit_GetView2D(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyView2DAttributes_NewPyObject();
    View2DAttributes *va = PyView2DAttributes_FromPyObject(retval);

    // Copy the viewer proxy's 2d view into the return data structure.
    *va = *(viewer->GetView2DAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetView3D
//
// Purpose:
//   Tells the viewer to use the 3D view we're giving it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Modified to handle a generic view attribute or a 3d view attribute.
//   
// ****************************************************************************

STATIC PyObject *
visit_SetView3D(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *view = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&view))
    {
        VisItErrorFunc("SetView3D: Cannot parse object!");
        return NULL;
    }
    if(PyView3DAttributes_Check(view))
    {
        MUTEX_LOCK();
            View3DAttributes *va = PyView3DAttributes_FromPyObject(view);

            // Copy the object into the view attributes.
            *(viewer->GetView3DAttributes()) = *va;
            viewer->GetView3DAttributes()->Notify();
            viewer->SetView3D();

            if(logging)
                fprintf(logFile, "SetView3D()\n");
        MUTEX_UNLOCK();
    }
    else if (PyViewAttributes_Check(view))
    {
        //
        // This feature is deprecated and the coding to support this
        // should be removed in visit 1.3.  The coding should match that
        // in SetViewCurve.
        //
        MUTEX_LOCK();
            ViewAttributes *va = PyViewAttributes_FromPyObject(view);
            View3DAttributes *v3da = viewer->GetView3DAttributes();

            // Copy the object into the view attributes.
            v3da->SetViewNormal(va->GetViewNormal());
            v3da->SetFocus(va->GetFocus());
            v3da->SetViewUp(va->GetViewUp());
            v3da->SetViewAngle(va->GetViewAngle());
            v3da->SetParallelScale(va->GetParallelScale());
            v3da->SetNearPlane(va->GetNearPlane());
            v3da->SetFarPlane(va->GetFarPlane());
            v3da->SetImagePan(va->GetImagePan());
            v3da->SetImageZoom(va->GetImageZoom());
            v3da->SetPerspective(va->GetPerspective());
            v3da->Notify();
            viewer->SetView3D();

            cerr << "Warning: Passing a ViewAttribute to SetView3D is"
                 << " deprecated.  Pass a" << endl
                 << " View3DAttribute instead." << endl;
            if(logging)
                fprintf(logFile, "SetView3D()\n");
        MUTEX_UNLOCK();
    }
    else
    {
        VisItErrorFunc("Argument is not a View3DAttributes object");
        return NULL;
    }

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetView3D
//
// Purpose:
//   Returns the 3D view.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Modify to return a 3d view attribute.
//   
// ****************************************************************************

STATIC PyObject *
visit_GetView3D(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyView3DAttributes_NewPyObject();
    View3DAttributes *va = PyView3DAttributes_FromPyObject(retval);

    // Copy the viewer proxy's 3d view into the return data structure.
    *va = *(viewer->GetView3DAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_ClearViewKeyframes
//
// Purpose:
//   Tells the viewer to clear the view keyframes for the active window.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Fri Jan  3 16:18:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ClearViewKeyframes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ClearViewKeyframes();
        if(logging)
            fprintf(logFile, "ClearViewKeyframes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_DeleteViewKeyframe
//
// Purpose:
//   Tells the viewer to delete a view keyframe from the active window.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Fri Jan  3 16:18:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_DeleteViewKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int frame;
    if(!PyArg_ParseTuple(args, "i", &frame))
        return NULL;

    MUTEX_LOCK();
        viewer->DeleteViewKeyframe(frame);
        if(logging)
            fprintf(logFile, "DeleteViewKeyframe(%d)\n", frame);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_MoveViewKeyframe
//
// Purpose:
//   Tells the viewer to move a view keyframe for the active window.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Tue Jan 28 13:53:33 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_MoveViewKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int oldFrame, newFrame;
    if(!PyArg_ParseTuple(args, "ii", &oldFrame, &newFrame))
        return NULL;

    MUTEX_LOCK();
        viewer->MoveViewKeyframe(oldFrame, newFrame);
        if(logging)
            fprintf(logFile, "MoveViewKeyframe(%d, %d)\n", oldFrame, newFrame);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetViewKeyframe
//
// Purpose:
//   Tells the viewer to set a view keyframe for the active window.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Fri Jan  3 16:18:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetViewKeyframe(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->SetViewKeyframe();
        if(logging)
            fprintf(logFile, "SetViewKeyframe()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetViewExtentsType
//
// Purpose: 
//   Sets the view extents type. This determines how the viewer sets the view.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 09:42:53 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetViewExtentsType(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int extType = 0; // original extents
    if (!PyArg_ParseTuple(args, "i", &extType))
    {
        char *cMode;
        if (!PyArg_ParseTuple(args, "s", &cMode))
            return NULL;

        if(strcmp(cMode, "original") == 0)
            extType = 0;
        else if(strcmp(cMode, "actual") == 0)
            extType = 1;
        else
            extType = 0;

        PyErr_Clear();
    }

    // Make sure the number is valid.
    if(extType != 0 && extType != 1)
    {
        VisItErrorFunc("An invalid extents type was provided! "
                       "Use \"original\" or \"actual\".");
        return NULL;
    }

    MUTEX_LOCK();
        viewer->SetViewExtentsType(extType);
        if(logging)
            fprintf(logFile, "SetViewExtentsType(%d)\n", extType);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetGlobalAttributes
//
// Purpose: 
//   Returns a GlobalAttributes object with the current state of the active
//   window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 19 08:54:23 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetGlobalAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyGlobalAttributes_NewPyObject();
    GlobalAttributes *ga = PyGlobalAttributes_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *ga = *(viewer->GetGlobalAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_GetWindowInformation
//
// Purpose:
//   Returns a window information object with the current state of the
//   active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 09:45:07 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetWindowInformation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyWindowInformation_NewPyObject();
    WindowInformation *wi = PyWindowInformation_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *wi = *(viewer->GetWindowInformation());

    return retval;
}

// ****************************************************************************
// Function: visit_GetRenderingAttributes
//
// Purpose:
//   Returns a rendering attributes object with the current state of the
//   active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 09:45:07 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetRenderingAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyRenderingAttributes_NewPyObject();
    RenderingAttributes *ra = PyRenderingAttributes_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *ra = *(viewer->GetRenderingAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetRenderingAttributes
//
// Purpose:
//   Tells the viewer to use the rendering attributes we're giving it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 24 09:51:19 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetRenderingAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *renderAtts = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&renderAtts))
    {
        VisItErrorFunc("SetRenderingAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyRenderingAttributes_Check(renderAtts))
    {
        VisItErrorFunc("Argument is not a RenderingAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        RenderingAttributes *ra = PyRenderingAttributes_FromPyObject(renderAtts);

        // Copy the object into the view attributes.
        *(viewer->GetRenderingAttributes()) = *ra;
        viewer->GetRenderingAttributes()->Notify();
        viewer->SetRenderingAttributes();

        if(logging)
            fprintf(logFile, "SetRenderingAttributes()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetWindowMode
//
// Purpose:
//   Tells the viewer to set the window mode.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 24 12:53:03 PDT 2002
//   Cleared the error flag.
//
//   Kathleen Bonnell, Tue Jul 22 10:47:46 PDT 2003 
//   Added zone pick, node pick and lineout, ensured mode numbers being
//   set from string names is correct. (based on WindowActions).
//
// ****************************************************************************

STATIC PyObject *
visit_SetWindowMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int mode;
    if (!PyArg_ParseTuple(args, "i", &mode))
    {
        char *cMode;
        if (!PyArg_ParseTuple(args, "s", &cMode))
            return NULL;

        if(strcmp(cMode, "navigate") == 0)
            mode = 0;
        else if((strcmp(cMode, "pick") == 0) ||
               (strcmp(cMode, "zone pick") == 0))
            mode = 1;
        else if(strcmp(cMode, "node pick") == 0)
            mode = 2;
        else if(strcmp(cMode, "zoom") == 0)
            mode = 3;
        else if(strcmp(cMode, "lineout") == 0)
            mode = 4;
        else
            mode = 0;

        PyErr_Clear();
    }

    MUTEX_LOCK();
        viewer->SetWindowMode(mode);
        if(logging)
            fprintf(logFile, "SetWindowMode(%d)\n", mode);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_EnableTool
//
// Purpose:
//   Tells the viewer to enable or disable certain interactive tools.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 24 12:53:28 PDT 2002
//   Cleared the error flag.
//
// ****************************************************************************

STATIC PyObject *
visit_EnableTool(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int tool, enabled;
    if (!PyArg_ParseTuple(args, "ii", &tool, &enabled))
    {
        char *toolName;
        if (!PyArg_ParseTuple(args, "si", &toolName, &enabled))
            return NULL;

        if(strcmp(toolName, "plane") == 0)
            tool = 0;
        if(strcmp(toolName, "sphere") == 0)
            tool = 1;
        else
            tool = 0;

        PyErr_Clear();
    }

    MUTEX_LOCK();
        viewer->EnableTool(tool, enabled==1);
        if(logging)
            fprintf(logFile, "EnableTool(%d, %d)\n", tool, enabled);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ListPlots
//
// Purpose: 
//   This is a Python callback that prints out the contents of the plot list.
//
// Programmer: John Bemis & Brad Whitlock
// Creation:   Thu Sep 20 16:58:39 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 10:10:37 PDT 2002
//   I made it print out the operators that are applied to each plot.
//
//   Eric Brugger, Tue Nov 19 10:53:43 PST 2002
//   I made it print out frame range and keyframe locations for each plot.
//
//   Brad Whitlock, Mon Dec 16 17:28:53 PST 2002
//   I fixed a problem that prevented it from building on AIX.
//
//   Eric Brugger, Thu Jan  2 16:00:58 PST 2003
//   I made it print out database keyframe locations for each plot.
//
//   Brad Whitlock, Thu Apr 17 17:22:58 PST 2003
//   I made it print out the active operator index.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_ListPlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
    //
    // Print out the plot list.
    //
    PlotList *pl = viewer->GetPlotList();
    PlotPluginManager *plugins = PlotPluginManager::Instance();

    for(int i = 0; i < pl->GetNumPlots(); ++i)
    {
        const Plot &plot = pl->operator[](i);
        std::string id(plugins->GetEnabledID(int(plot.GetPlotType())));
        CommonPlotPluginInfo *info = plugins->GetCommonPluginInfo(id);

        if(info != 0)
        {
             int j;
             printf("Plot[%d] = {type=\"%s\", database=\"%s\", var=%s, active=%d, framerange=(%d, %d), keyframes={", i,
                    info->GetName(),
                    plot.GetDatabaseName().c_str(),
                    plot.GetPlotVar().c_str(),
                    plot.GetActiveFlag(),
                    plot.GetBeginFrame(),
                    plot.GetEndFrame());
 
             // Print out the keyframes.
             const std::vector<int> &keyframes = plot.GetKeyframes();
             for(j = 0; j < keyframes.size(); ++j)
             {
                 printf("%d", keyframes[j]);
                 if(j < keyframes.size() - 1)
                     printf(", ");
             }
             printf("}, database keyframes={");

             // Print out the database keyframes.
             const std::vector<int> &databaseKeyframes =
                 plot.GetDatabaseKeyframes();
             for(j = 0; j < databaseKeyframes.size(); ++j)
             {
                 printf("%d", databaseKeyframes[j]);
                 if(j < databaseKeyframes.size() - 1)
                     printf(", ");
             }
             printf("}, operators={");

             // Print out the plot operators.
             for(j = 0; j < plot.GetNumOperators(); ++j)
             {
                 int op = plot.GetOperator(j);
                 printf("\"%s\"", OperatorPluginManager::Instance()->GetEnabledID(op).c_str());
                 if(j < plot.GetNumOperators() - 1)
                     printf(", ");
             }
             printf("}, activeOperator=%d}\n", plot.GetActiveOperator());
        }
    }

    // Write the command to the file.
    if(logging)
        fprintf(logFile, "ListPlots()\n");
    MUTEX_UNLOCK();

    // Increment the reference count and return.
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_Expressions
//
// Purpose:
//   Returns a tuple containing the expressions.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Jun 28 16:02:21 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
STATIC PyObject *
visit_Expressions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
    ExpressionList *list = viewer->GetExpressionList();

    // Allocate a tuple the with enough entries to hold the expression list.
    PyObject *retval = PyTuple_New(list->GetNumExpressions());

    for(int i = 0; i < list->GetNumExpressions(); i++)
    {
        PyObject *expr = PyTuple_New(2);
        PyObject *name = PyString_FromString((*list)[i].GetName().c_str());
        PyTuple_SET_ITEM(expr, 0, name);
        PyObject *def = PyString_FromString((*list)[i].GetDefinition().c_str());
        PyTuple_SET_ITEM(expr, 1, def);
        PyTuple_SET_ITEM(retval, i, expr);
    }
    MUTEX_UNLOCK();

    return retval;
}

// ****************************************************************************
// Function: visit_ResetOperatorOptions
//
// Purpose: 
//   This is a Python callback that tells the viewer to reset the operator
//   attributes for the specified operator type.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 20 16:50:04 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 16:59:17 PST 2002
//   I made it use the right function for the determination of the plot
//   index and also added an optional applyToAll argument.
//
//   Brad Whitlock, Fri Jul 26 12:31:21 PDT 2002
//   I made it return a success value.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_ResetOperatorOptions(PyObject *self, PyObject *args)
{
    char *operatorName;
    int  applyToAllPlots = 0;
    if(!PyArg_ParseTuple(args, "s", &operatorName))
    {
        if(!PyArg_ParseTuple(args, "si", &operatorName, &applyToAllPlots))
            return NULL;
        else
            PyErr_Clear();
    }

    // Find the plot index from the name. Throw a python exception if we are
    // allowing python exceptions and the plot index is -1.
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    int operatorTypeIndex = -1;
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info=pluginManager->GetScriptingPluginInfo(id);
        if(info == 0)
            continue;

        if(info->GetName() == std::string(operatorName))
        {
            operatorTypeIndex = i;
            break;
        }
    }

    int errorFlag = 1;

    // If the plot type was not found, return.
    if(operatorTypeIndex < 0)
        VisItErrorFunc("Invalid operator plugin name");
    else
    {
        MUTEX_LOCK();
            // Set the apply to all plots toggle.
            viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
            viewer->GetGlobalAttributes()->Notify();

            // Reset the operator options.
            viewer->ResetOperatorOptions(operatorTypeIndex);
            if(logging)
            {
                fprintf(logFile, "ResetOperatorOptions(\"%s\", %d)\n",
                        operatorName, applyToAllPlots);
            }
        MUTEX_UNLOCK();
        errorFlag = Synchronize();
    }

    // Return the success value.
    return IntReturnValue(errorFlag);
}

// ****************************************************************************
// Function: visit_ResetPlotOptions
//
// Purpose: 
//   This is a Python callback that tells the viewer to reset the plot
//   attributes for the specified plot type.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 20 16:50:04 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 16:59:17 PST 2002
//   I made it use the right function for the determination of the plot index.
//
//   Brad Whitlock, Fri Jul 26 12:32:15 PDT 2002
//   I made it return a success value.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_ResetPlotOptions(PyObject *self, PyObject *args)
{
    char *plotName;
    if(!PyArg_ParseTuple(args, "s", &plotName))
        return NULL;

    // Find the plot index from the name. Throw a python exception if we are
    // allowing python exceptions and the plot index is -1.
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    int plotTypeIndex = -1;
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info=pluginManager->GetScriptingPluginInfo(id);
        if(info == 0)
            continue;

        if(info->GetName() == std::string(plotName))
        {
            plotTypeIndex = i;
            break;
        }
    }

    int errorFlag = 1;

    // If the plot type was not found, return.
    if(plotTypeIndex < 0)
        VisItErrorFunc("Invalid plot plugin name");
    else
    {
        MUTEX_LOCK();
            viewer->ResetPlotOptions(plotTypeIndex);
            if(logging)
                fprintf(logFile, "ResetPlotOptions(\"%s\")\n", plotName);
        MUTEX_UNLOCK();
        errorFlag = Synchronize();
    }

    // Return the success value.
    return IntReturnValue(errorFlag);
}

// ****************************************************************************
// Function: visit_SetActivePlots
//
// Purpose: 
//   This is a Python callback that sets the active plots in the plot list.
//
// Note:       This function accepts a tuple or single int value.
//
// Programmer: John Bemis, Brad Whitlock
// Creation:   Thu Sep 20 16:54:38 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:33:25 PDT 2002
//   Made it return a success value and I made it return an error if any of
//   the input plot indices are bad.
//
//   Brad Whitlock, Fri Dec 27 10:59:12 PDT 2002
//   I passed an intVector to the viewer proxy instead of an array of ints.
//
// ****************************************************************************

STATIC PyObject *
visit_SetActivePlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    intVector  vec;
    PyObject   *tuple;
    if(!PyArg_ParseTuple(args, "O", &tuple))
        return NULL;

    if(PyTuple_Check(tuple))
    {
        vec.reserve(PyTuple_Size(tuple));
        for(int i = 0; i < PyTuple_Size(tuple); ++i)
        {
            PyObject *item = PyTuple_GET_ITEM(tuple, i);
            if(PyFloat_Check(item))
                vec.push_back(int(PyFloat_AS_DOUBLE(item)));
            else if(PyInt_Check(item))
                vec.push_back(int(PyInt_AS_LONG(item)));
            else if(PyLong_Check(item))
                vec.push_back(int(PyLong_AsDouble(item)));
            else
                vec.push_back(0);
        }
    }
    else if(PyFloat_Check(tuple))
        vec.push_back(int(PyFloat_AS_DOUBLE(tuple)));
    else if(PyInt_Check(tuple))
        vec.push_back(int(PyInt_AS_LONG(tuple)));
    else if(PyLong_Check(tuple))
        vec.push_back(int(PyLong_AsDouble(tuple)));
    else
        return NULL;

    //
    // Set the active plots using the indices in the vector.
    //
    bool okayToSet = false;
    MUTEX_LOCK();
    intVector activePlots;
    for(int j = 0; j < vec.size(); ++j)
    {
        if(vec[j] < 0 || vec[j] >= viewer->GetPlotList()->GetNumPlots())
        {
            okayToSet = false;
            break;
        }
        else
        {
            okayToSet = true;
            activePlots.push_back(vec[j]);
        }
    }
    if(okayToSet)
        viewer->SetActivePlots(activePlots);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(okayToSet ? Synchronize() : 1);
}

// ****************************************************************************
// Function: visit_SetOperatorOptions
//
// Purpose:
//   This is a Python callback function that accepts a Python version of an
//   AttributeSubject that is copied to the appropriate operator attributes
//   and applied to the active plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:30:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 09:54:40 PDT 2002
//   I added an optional second argument that allows the operator attributes
//   to be applied to all plots. It is false by default.
//
//   Brad Whitlock, Fri Jul 26 12:37:00 PDT 2002
//   I made it return a success value.
//
//   Brad Whitlock, Thu Apr 17 15:09:53 PST 2003
//   I added an optional active operator index.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_SetOperatorOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *obj = NULL;

    //
    // Try and get the object pointer.
    //
    int activeOperator = -1;
    int applyToAllPlots = 0;
    if(!PyArg_ParseTuple(args, "Oii", &obj, &activeOperator, &applyToAllPlots))
    {
        if(!PyArg_ParseTuple(args, "Oi", &obj, &activeOperator))
        {
            if(!PyArg_ParseTuple(args, "O", &obj))
            {
                VisItErrorFunc("This function takes an operator attributes "
                               "object as the first argument. The second "
                               "argument, which is optional is the active "
                               "operator index. When it is specified, the "
                               "operator attributes will only be applied to "
                               "specified active operator. The third argument, "
                               "which is also optional, is a flag that tells "
                               "VisIt if the operator attributes should be "
                               "applied to all plots instead of just the "
                               "selected plots.");
                return NULL;
            }
            else
                PyErr_Clear();
        }
        else
            PyErr_Clear();
    }

    //
    // Make sure that the object is a operator plugin attributes object.
    //
    int objPluginIndex = -1;
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        if(info->TypesMatch(obj))
        {
            objPluginIndex = i;
            break;
        }
    }
    if(objPluginIndex == -1)
    {
        VisItErrorFunc("The first argument must be an operator attributes object!");
        // Return a failure value.
        return NULL;
    }

    //
    // Copy the object into the appropriate viewer plot attributes.
    //
    MUTEX_LOCK();
    if(viewer)
    {
        // Set the apply to all plots toggle.
        viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        viewer->GetGlobalAttributes()->Notify();

        // If the active operator was set, change the plot selection so we can set
        // the active operator.
        intVector selectedPlots, newActiveOperators, newExpandedPlots;
        intVector oldActiveOperators, oldExpandedPlots;
        if(activeOperator != -1)
        {
            PlotList *plotList = viewer->GetPlotList();
            for(int i = 0; i < plotList->GetNumPlots(); ++i)
            {
                const Plot &current = plotList->operator[](i);
                if(current.GetActiveFlag())
                {
                    selectedPlots.push_back(i);
                    newActiveOperators.push_back(activeOperator);
                }
                else
                    newActiveOperators.push_back(current.GetActiveOperator());
                newExpandedPlots.push_back(current.GetActiveFlag() ? 1 : 0);
                oldActiveOperators.push_back(current.GetActiveOperator());
                oldExpandedPlots.push_back(current.GetExpandedFlag());
            }

            if(selectedPlots.size() > 0)
            {
                viewer->SetActivePlots(selectedPlots, newActiveOperators,
                                       newExpandedPlots);
            }
        }

        // Set the operator attributes.
        AttributeSubject *operAtts = viewer->GetOperatorAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonOperatorPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        general->CopyAttributes(operAtts, ((AttributesObject *)obj)->data);
        operAtts->Notify();
        viewer->SetOperatorOptions(objPluginIndex);

        // Restore the plot selection.
        if(activeOperator != -1 && selectedPlots.size() > 0)
        {
            viewer->SetActivePlots(selectedPlots, oldActiveOperators, oldExpandedPlots);
        }
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: PromoteDemoteRemoveOperatorHelper
//
// Purpose:
//   Tells the viewer to promote, demote, or remove operators.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 15:39:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
PromoteDemoteRemoveOperatorHelper(PyObject *self, PyObject *args, int option)
{
    ENSURE_VIEWER_EXISTS();

    //
    // Try and get the arguments.
    //
    int operatorIndex = 0;
    int applyToAllPlots = 0;
    if(!PyArg_ParseTuple(args, "ii", &operatorIndex, &applyToAllPlots))
    {
        if(!PyArg_ParseTuple(args, "i", &operatorIndex))
        {
            VisItErrorFunc("This function takes an integer operator index "
                           "as the first argument. The second integer argument "
                           "is optional and non-zero values tell VisIt to apply "
                           "this operation to all plots.");
            return NULL;
        }
        else
            PyErr_Clear();
    }

    MUTEX_LOCK();
    if(viewer)
    {
        // Set the apply to all plots toggle.
        viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        viewer->GetGlobalAttributes()->Notify();

        // Do the operation.
        if(option == 0)
            viewer->PromoteOperator(operatorIndex);
        else if(option == 1)
            viewer->DemoteOperator(operatorIndex);
        else if(option == 2)
            viewer->RemoveOperator(operatorIndex);
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_PromoteOperator
//
// Purpose: 
//   Tells the viewer to promote an operator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 15:42:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_PromoteOperator(PyObject *self, PyObject *args)
{
    return PromoteDemoteRemoveOperatorHelper(self, args, 0);
}

// ****************************************************************************
// Method: visit_DemoteOperator
//
// Purpose: 
//   Tells the viewer to demote an operator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 15:42:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_DemoteOperator(PyObject *self, PyObject *args)
{
    return PromoteDemoteRemoveOperatorHelper(self, args, 1);
}

// ****************************************************************************
// Method: visit_RemoveOperator
//
// Purpose: 
//   Tells the viewer to promote an operator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 17 15:42:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RemoveOperator(PyObject *self, PyObject *args)
{
    return PromoteDemoteRemoveOperatorHelper(self, args, 2);
}

// ****************************************************************************
// Function: visit_SetDefaultOperatorOptions
//
// Purpose:
//   This is a Python callback function that accepts a Python version of an
//   AttributeSubject that is copied to the appropriate operator attributes
//   and used to set the default attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:30:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon May 6 14:32:38 PST 2002
//   Added code to set the default operator atts in the scripting plugin.
//
//   Brad Whitlock, Fri Jul 26 12:37:58 PDT 2002
//   I made it return a success value.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultOperatorOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *obj = NULL;

    //
    // Try and get the object pointer.
    //
    if(!PyArg_ParseTuple(args, "O", &obj))
    {
        VisItErrorFunc("The argument must be an operator attributes object.");
        return NULL;
    }

    //
    // Make sure that the object is a operator plugin attributes object.
    //
    int objPluginIndex = -1;
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        if(info->TypesMatch(obj))
        {
            objPluginIndex = i;
            break;
        }
    }
    if(objPluginIndex == -1)
    {
        VisItErrorFunc("The argument must be an operator attributes object.");
        // Return a failure value.
        return NULL;
    }

    //
    // Copy the object into the appropriate viewer plot attributes.
    //
    MUTEX_LOCK();
    if(viewer)
    {
        AttributeSubject *operAtts = viewer->GetOperatorAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonOperatorPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        ScriptingOperatorPluginInfo *scripting = pluginManager->GetScriptingPluginInfo(id);
        general->CopyAttributes(operAtts, ((AttributesObject *)obj)->data);
        scripting->SetDefaults(operAtts);
        operAtts->Notify();
        viewer->SetDefaultOperatorOptions(objPluginIndex);
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetDefaultPlotOptions
//
// Purpose:
//   This is a Python callback function that accepts a Python version of an
//   AttributeSubject that is copied to the appropriate plot attributes and
//   used to set the default attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:30:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon May 6 14:12:59 PST 2002
//   Added code to set the default plot options in the scripting plugin.
//
//   Brad Whitlock, Fri Jul 26 12:38:23 PDT 2002
//   I made it return a success value.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultPlotOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *obj = NULL;

    //
    // Try and get the object pointer.
    //
    if(!PyArg_ParseTuple(args, "O", &obj))
    {
        VisItErrorFunc("The argument must be a plot attributes object.");
        return NULL;
    }

    //
    // Make sure that the object is a plot plugin attributes object.
    //
    int objPluginIndex = -1;
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        if(info->TypesMatch(obj))
        {
            objPluginIndex = i;
            break;
        }
    }
    if(objPluginIndex == -1)
    {
        VisItErrorFunc("The argument must be a plot attributes object.");
        // Return a failure value.
        return NULL;
    }

    //
    // Copy the object into the appropriate viewer plot attributes.
    //
    MUTEX_LOCK();
    if(viewer)
    {
        AttributeSubject *plotAtts = viewer->GetPlotAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonPlotPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        ScriptingPlotPluginInfo *scripting = pluginManager->GetScriptingPluginInfo(id);
        general->CopyAttributes(plotAtts, ((AttributesObject *)obj)->data);
        scripting->SetDefaults(plotAtts);
        plotAtts->Notify();
        viewer->SetDefaultPlotOptions(objPluginIndex);
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetPlotOptions
//
// Purpose:
//   This is a Python callback function that accepts a Python version of an
//   AttributeSubject that is copied to the appropriate plot attributes and
//   applied to the active plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:30:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:40:07 PDT 2002
//   I made it return a success value.
//
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_SetPlotOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *obj = NULL;

    //
    // Try and get the object pointer.
    //
    if(!PyArg_ParseTuple(args, "O", &obj))
    {
        VisItErrorFunc("The argument must be a plot attributes object.");
        return NULL;
    }

    //
    // Make sure that the object is a plot plugin attributes object.
    //
    int objPluginIndex = -1;
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        if(info->TypesMatch(obj))
        {
            objPluginIndex = i;
            break;
        }
    }
    if(objPluginIndex == -1)
    {
        VisItErrorFunc("The argument must be a plot attributes object.");
        // Return a failure value.
        return NULL;
    }

    //
    // Copy the object into the appropriate viewer plot attributes.
    //
    MUTEX_LOCK();
    if(viewer)
    {
        AttributeSubject *plotAtts = viewer->GetPlotAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonPlotPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        general->CopyAttributes(plotAtts, ((AttributesObject *)obj)->data);
        plotAtts->Notify();
        viewer->SetPlotOptions(objPluginIndex);
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetPlotSILRestriction
//
// Purpose:
//   Sets the SIL restriction for the selected plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 11:55:02 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 12:35:45 PDT 2002
//   I added an optional argument for applying the sil restriction to all plots.
//
//   Brad Whitlock, Fri Jul 26 12:41:37 PDT 2002
//   I made it return a success value.
//
//   Hank Childs, Mon Dec  2 14:11:12 PST 2002
//   Use reference counted SIL restrictions to meet new interface.
//
// ****************************************************************************

STATIC PyObject *
visit_SetPlotSILRestriction(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *obj = NULL;

    //
    // Try and get the object pointer.
    //
    bool applyToAllPlots = false;
    if(!PyArg_ParseTuple(args, "O", &obj))
    {
        if(!PyArg_ParseTuple(args, "Oi", &obj, &applyToAllPlots))
        {
            VisItErrorFunc("The argument must be a SIL restriction object.");
            return NULL;
        }
        else
            PyErr_Clear();
    }

    if(!PySILRestriction_Check(obj))
    {
        VisItErrorFunc("The argument must be a SIL restriction object.");
        // Return a failure value.
        return NULL;
    }

    //
    // Copy the SIL restriction into the viewer proxy and apply it.
    //
    MUTEX_LOCK();
    if(viewer)
    {
        // Set the apply to all plots toggle.
        viewer->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        viewer->GetGlobalAttributes()->Notify();

        // Set the sil restriction.
        avtSILRestriction_p silr = PySILRestriction_FromPyObject(obj);
        viewer->SetPlotSILRestriction(silr);
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: visit_GetPickOutput
//
// Purpose:
//   Returns the pick output for the active window.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   May 14, 2003 
//
// Modifications:
//   Kathleen Bonnell, Tue May  4 14:44:33 PDT 2004
//   Use PickAtts' error message if available.
//   
// ****************************************************************************

STATIC PyObject *
visit_GetPickOutput(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    std::string pickOut;
    PickAttributes *pa = viewer->GetPickAttributes();
    if (pa->GetFulfilled())
    {
        pa->CreateOutputString(pickOut);
    }
    else
    {
        if (pa->GetError())
            pickOut = pa->GetErrorMessage();
        else
            pickOut = "Either no Pick has been performed, "
                      "or the last Pick was invalid.\n"; 
    }
    return PyString_FromString(pickOut.c_str());
}

// ****************************************************************************
// Function: visit_GetQueryOutputString
//
// Purpose:
//   Returns the query output as a string for the active window.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   July 11, 2003 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetQueryOutputString(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    std::string queryOut;
    QueryAttributes *qa = viewer->GetQueryAttributes();
    queryOut = qa->GetResultsMessage();
    return PyString_FromString(queryOut.c_str());
}

// ****************************************************************************
// Function: visit_GetQueryOutputValue
//
// Purpose:
//   Returns the query output value for the active window.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   July 11, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Nov 12 17:55:14 PST 2003
//   If the query returned multiple values, return them in a python tuple. 
//
// ****************************************************************************

STATIC PyObject *
visit_GetQueryOutputValue(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    QueryAttributes *qa = viewer->GetQueryAttributes();
    doubleVector vals = qa->GetResultsValue();
    PyObject *retval;
    if (vals.size() == 1)
        retval = PyFloat_FromDouble(vals[0]);
    else
    {
        PyObject *tuple = PyTuple_New(vals.size());
        for(int j = 0; j < vals.size(); ++j)
        {
            PyObject *item = PyFloat_FromDouble(vals[j]);
            if(item == NULL)
                continue;
            PyTuple_SET_ITEM(tuple, j, item);
        }
        retval = tuple;
    }
    return retval;
}


// ****************************************************************************
// Function: ListCategoryHelper
//
// Purpose: 
//   This is a helper function that lists the members of a category like
//   domains or materials.
//
// Arguments:
//   role : The category that we want to print.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:30:52 PST 2002
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:11:12 PST 2002
//   Use reference counted SIL restrictions to meet new interface.
//
// ****************************************************************************

void
ListCategoryHelper(SILCategoryRole role)
{
    MUTEX_LOCK();
    if(viewer)
    {
        avtSILRestriction_p silr = viewer->GetPlotSILRestriction();

        // Print the material collections for each whole set in the SIL.
        avtSILRestrictionTraverser trav(silr);
        for(int i = 0; i < silr->GetNumCollections(); ++i)
        {
            avtSILCollection_p collection = silr->GetSILCollection(i);
            if(collection->GetRole() == role)
            {
                const std::vector<int> &sets = collection->GetSubsetList();
                if(sets.size() > 0)
                {
                    // If more than one collection maps into this set, we don't
                    // want to display it because it's crossed with something else.
                    if(collection->GetSupersetIndex() == silr->GetTopSet() &&
                       silr->GetSILSet(sets[0])->GetMapsIn().size() < 2)
                    {
                        printf("%s:\n", collection->GetCategory().c_str());
                        for(int j = 0; j < sets.size(); ++j)
                        {
                            printf("\t\"%s\"   ", silr->GetSILSet(sets[j])->GetName().c_str());
                            if(trav.UsesSetData(sets[j]) == NoneUsed)
                                printf("off\n");
                            else
                                printf("on\n");
                        }
                    }
                }
                else
                    printf("%s:", collection->GetCategory().c_str());
            }
        }
    }
    MUTEX_UNLOCK();
}

// ****************************************************************************
// Function: GetCategoryTupleHelper
//
// Purpose: 
//   Returns a tuple of the names of the sets that belong to a certain category.
//
// Arguments:
//   role : The category that we want.
//
// Returns:    A Python tuple of strings.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:31:46 PST 2002
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:11:12 PST 2002
//   Use reference counted SIL restrictions to meet new interface.
//
// ****************************************************************************

PyObject *
GetCategoryTupleHelper(SILCategoryRole role)
{
    PyObject *retval = NULL;

    MUTEX_LOCK();
    if(viewer)
    {
        avtSILRestriction_p silr = viewer->GetPlotSILRestriction();

        // Print the material collections for each whole set in the SIL.
        for(int i = 0; i < silr->GetNumCollections(); ++i)
        {
            avtSILCollection_p collection = silr->GetSILCollection(i);
            if(collection->GetRole() == role)
            {
                const std::vector<int> &sets = collection->GetSubsetList();
                if(sets.size() > 0)
                {
                    // If more than one collection maps into this set, we don't
                    // want to display it because it's crossed with something else.
                    if(collection->GetSupersetIndex() == silr->GetTopSet() &&
                       silr->GetSILSet(sets[0])->GetMapsIn().size() < 2)
                    {
                        PyObject *tuple = PyTuple_New(sets.size());
                        for(int j = 0; j < sets.size(); ++j)
                        {
                            PyObject *item = PyString_FromString(silr->GetSILSet(sets[j])->GetName().c_str());
                            if(item == NULL)
                                continue;
                            PyTuple_SET_ITEM(tuple, j, item);
                        }

                        retval = tuple;
                        break;
                    }
                }
            }
        }
    }
    MUTEX_UNLOCK();

    // If the tuple is not populated by now, return an empty tuple.
    if(retval == NULL)
        retval = PyTuple_New(0);

    return retval;
}

// ****************************************************************************
// Function: TurnOnOffHelper
//
// Purpose: 
//   This is a helper function for turning sets on and off in the SIL restriction.
//
// Arguments:
//   role  : The category of the sets that we want to turn on/off.
//   val   : The new on/off value.
//   names : The names of the sets to modify.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:32:42 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Dec 16 13:25:31 PST 2002
//   I added code to turn off "Apply to all" so the SIL restriction only
//   gets set for the selected plots.
//
//   Hank Childs, Mon Dec  2 14:11:12 PST 2002
//   Use reference counted SIL restrictions to meet new interface.
//
//   Brad Whitlock, Fri Jan 17 12:10:56 PDT 2003
//   I made it return a boolean value indicating whether or not there were
//   errors.
//
// ****************************************************************************

bool
TurnOnOffHelper(SILCategoryRole role, bool val, const stringVector &names)
{
    bool retval = true;
    avtSILRestriction_p silr = viewer->GetPlotSILRestriction();

    // Print the material collections for each whole set in the SIL.
    for(int i = 0; i < silr->GetNumCollections(); ++i)
    {
        avtSILCollection_p collection = silr->GetSILCollection(i);
        if(collection->GetRole() == role)
        {
            const std::vector<int> &sets = collection->GetSubsetList();
            if(sets.size() > 0)
            {
                // If more than one collection maps into this set, we don't
                // want to display it because it's crossed with something else.
                if(collection->GetSupersetIndex() == silr->GetTopSet() &&
                   silr->GetSILSet(sets[0])->GetMapsIn().size() < 2)
                {
                    silr->SuspendCorrectnessChecking();

                    if(names.size() > 0)
                    {
                        for(int j = 0; j < names.size(); ++j)
                        {
                            bool nameMatched = false;
                            for(int k = 0; k < sets.size(); ++k)
                            {
                                if(names[j] == silr->GetSILSet(sets[k])->GetName())
                                {
                                    if(val)
                                        silr->TurnOnSet(sets[k]);
                                    else
                                        silr->TurnOffSet(sets[k]);

                                    nameMatched = true;
                                    break;
                                }
                            }

                            if(!nameMatched)
                            {
                                fprintf(stderr, "%s is an invalid set name.\n",
                                        names[j].c_str());
                                retval = false;
                            }
                        }
                    }
                    else if(val)
                        silr->TurnOnAll();
                    else
                        silr->TurnOffAll();                    

                    silr->EnableCorrectnessChecking();
                    break;
                }
            }
        }
    }

    // Set the apply to all plots toggle.
    viewer->GetGlobalAttributes()->SetApplyOperator(false);
    viewer->GetGlobalAttributes()->Notify();

    // Send the modified SIL restriction to the viewer.
    viewer->SetPlotSILRestriction(silr);

    return retval;
}

// ****************************************************************************
// Function: GetNamesHelper
//
// Purpose: 
//   Reads the names of the sets that we want to turn on/off.
//
// Arguments:
//   names : A string vector in which to store the names.
//
// Returns:    true if the arguments were okay; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:34:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
GetNamesHelper(PyObject *self, PyObject *args, stringVector &names)
{
    PyObject *tuple;
    char *str;
    bool retval = false;

    if(PyArg_ParseTuple(args, "s", &str))
    {
        names.push_back(str);
        retval = true;
    }
    else if(PyArg_ParseTuple(args, "O", &tuple))
    {
        if(PyTuple_Check(tuple))
        {
            names.reserve(PyTuple_Size(tuple));
            for(int i = 0; i < PyTuple_Size(tuple); ++i)
            {
                PyObject *item = PyTuple_GET_ITEM(tuple, i);
                if(PyString_Check(item))
                    names.push_back(PyString_AS_STRING(item));
                else
                    names.push_back("invalid");
            }
            retval = true;
            PyErr_Clear();
        }
    }
    else
    {
        // no arguments or no suitable arguments given.
        retval = true;
        PyErr_Clear();
    }

    return retval;
}

// ****************************************************************************
// Function: TurnCategoryHelper
//
// Purpose: 
//   This is a helper function for turning categories on/off.
//
// Arguments:
//   role : The category of the sets that we want to change.
//   val  : The new on/off value.
//
// Returns:    Whether or not the SIL was successfully changed.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:35:43 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jan 17 12:19:34 PDT 2003
//   I made the return value of TurnOnOffHelper influence the return
//   value of the Python function.
//
// ****************************************************************************

STATIC PyObject *
TurnCategoryHelper(PyObject *self, PyObject *args, SILCategoryRole role, bool val)
{
    stringVector names;
    int errorFlag = 0;
    bool locked = true;

    MUTEX_LOCK();
    if(viewer)
    {
        if(GetNamesHelper(self, args, names))
        {
            errorFlag = TurnOnOffHelper(role, val, names) ? 0 : 1;
            MUTEX_UNLOCK();
            locked = false;
            int syncErrorFlag = Synchronize();
            if(syncErrorFlag < 0)
                errorFlag = syncErrorFlag;
            else
                errorFlag |= syncErrorFlag;
        }
    }
    if(locked)
    {
        MUTEX_UNLOCK();
    }

    return IntReturnValue(errorFlag);
}

// ****************************************************************************
// Function: visit_TurnMaterialsOn
//
// Purpose: 
//   Turns materials on.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:37:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_TurnMaterialsOn(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return TurnCategoryHelper(self, args, SIL_MATERIAL, true);
}

// ****************************************************************************
// Function: visit_TurnMaterialsOff
//
// Purpose: 
//   Turns materials off.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:37:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_TurnMaterialsOff(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return TurnCategoryHelper(self, args, SIL_MATERIAL, false);
}

// ****************************************************************************
// Function: visit_TurnDomainsOn
//
// Purpose: 
//   Turns domains on.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:37:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_TurnDomainsOn(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return TurnCategoryHelper(self, args, SIL_DOMAIN, true);
}

// ****************************************************************************
// Function: visit_TurnDomainsOff
//
// Purpose: 
//   Turns domains off.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:37:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_TurnDomainsOff(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return TurnCategoryHelper(self, args, SIL_DOMAIN, false);
}

// ****************************************************************************
// Function: visit_ListMaterials
//
// Purpose: 
//   Prints a list of materials for the selected plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 07:46:34 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ListMaterials(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    ListCategoryHelper(SIL_MATERIAL);

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_GetMaterials
//
// Purpose: 
//   Returns a tuple of material names for the selected plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:37:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetMaterials(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return GetCategoryTupleHelper(SIL_MATERIAL);
}

// ****************************************************************************
// Function: visit_ListDomains
//
// Purpose: 
//   Prints a list of domains for the selected plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 07:46:34 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ListDomains(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    ListCategoryHelper(SIL_DOMAIN);

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_GetDomains
//
// Purpose: 
//   Returns a tuple of domain names for the selected plot.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 13 13:37:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetDomains(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return GetCategoryTupleHelper(SIL_DOMAIN);
}

// ****************************************************************************
// Function: visit_ColorTableNames
//
// Purpose:
//   Returns a tuple containins the names of the colortables.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ColorTableNames(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    const stringVector &ctNames = viewer->GetColorTableAttributes()->GetNames();
    PyObject *retval = PyTuple_New(ctNames.size());

    for(int i = 0; i < ctNames.size(); ++i)
    {
        PyObject *dval = PyString_FromString(ctNames[i].c_str());
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }
    MUTEX_UNLOCK();

    return retval;
}

// ****************************************************************************
// Function: visit_NumColorTables
//
// Purpose:
//   Returns the number of colortables.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_NumColorTables(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const stringVector &ctNames = viewer->GetColorTableAttributes()->GetNames();
    PyObject *retval = PyLong_FromLong(ctNames.size());

    return retval;
}

// ****************************************************************************
// Function: visit_SetActiveContinuousColorTable
//
// Purpose:
//   Tells the viewer to set a new active continuous colortable.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Dec 3 11:07:48 PDT 2002
//   Renamed the function.
//
// ****************************************************************************

STATIC PyObject *
visit_SetActiveContinuousColorTable(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *ctName;
    if(!PyArg_ParseTuple(args, "s", &ctName))
    {
        VisItErrorFunc("The argument must be a color table name.");
        return NULL;
    }

    MUTEX_LOCK();
        viewer->SetActiveContinuousColorTable(ctName);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetActiveDiscreteColorTable
//
// Purpose:
//   Tells the viewer to set a new active discrete colortable.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 3 11:08:02 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetActiveDiscreteColorTable(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *ctName;
    if(!PyArg_ParseTuple(args, "s", &ctName))
    {
        VisItErrorFunc("The argument must be a color table name.");
        return NULL;
    }

    MUTEX_LOCK();
        viewer->SetActiveDiscreteColorTable(ctName);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetActiveContinuousColorTable
//
// Purpose:
//   Returns the active continuous colortable.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetActiveContinuousColorTable(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const std::string &ct = viewer->GetColorTableAttributes()->GetActiveContinuous();
    PyObject *retval = PyString_FromString(ct.c_str());

    return retval;
}

// ****************************************************************************
// Function: visit_GetActiveDiscreteColorTable
//
// Purpose:
//   Returns the active discrete colortable.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 3 11:10:10 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetActiveDiscreteColorTable(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const std::string &ct = viewer->GetColorTableAttributes()->GetActiveDiscrete();
    PyObject *retval = PyString_FromString(ct.c_str());

    return retval;
}

// ****************************************************************************
// Function: visit_GetNumPlots
//
// Purpose:
//   Returns the number of plots in the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetNumPlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    return PyInt_FromLong(long(viewer->GetPlotList()->GetNumPlots()));
}

// ****************************************************************************
// Function: visit_PlotPlugins
//
// Purpose:
//   Returns a tuple containing the plot plugin names.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_PlotPlugins(PyObject *self, PyObject *args)
{
    PlotPluginManager *plugins = PlotPluginManager::Instance();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    PyObject *retval = PyTuple_New(plugins->GetNEnabledPlugins());

    for(int i = 0; i < plugins->GetNEnabledPlugins(); ++i)
    {
        std::string id(plugins->GetEnabledID(i));
        CommonPlotPluginInfo *info = plugins->GetScriptingPluginInfo(id);
        PyObject *dval = PyString_FromString(info->GetName());
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Function: visit_NumPlotPlugins
//
// Purpose:
//   Returns the number of plot plugins.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_NumPlotPlugins(PyObject *self, PyObject *args)
{
    PlotPluginManager *plugins = PlotPluginManager::Instance();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    PyObject *retval = PyLong_FromLong((long)plugins->GetNEnabledPlugins());

    return retval;
}

// ****************************************************************************
// Function: visit_OperatorPlugins
//
// Purpose:
//   Returns a tuple containing the names of the operator plugins.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
//   Jeremy Meredith, Tue Jun 17 17:41:04 PDT 2003
//   Made it use the "enabled" plugin index instead the "all" index.
//
// ****************************************************************************

STATIC PyObject *
visit_OperatorPlugins(PyObject *self, PyObject *args)
{
    OperatorPluginManager *plugins = OperatorPluginManager::Instance();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    PyObject *retval = PyTuple_New(plugins->GetNEnabledPlugins());

    for(int i = 0; i < plugins->GetNEnabledPlugins(); ++i)
    {
        std::string id(plugins->GetEnabledID(i));
        CommonOperatorPluginInfo *info = plugins->GetScriptingPluginInfo(id);
        PyObject *dval = PyString_FromString(info->GetName());
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Function: visit_Queries
//
// Purpose:
//   Returns a tuple containing the names of queries that can be used with
//   the Query command.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   November 12, 2003 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_Queries(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    stringVector queries = viewer->GetQueryList()->GetNames();

    // We only want to include Database queries, so count them.
    intVector types = viewer->GetQueryList()->GetTypes();
    int nQueries = 0; 
    for(int i = 0; i < types.size(); ++i)
    {
        if (types[i] == QueryList::DatabaseQuery)
            nQueries++;
    }

    // Allocate a tuple with enough entries to hold the queries name list.
    PyObject *retval = PyTuple_New(nQueries);

    for(int j = 0, k = 0; j < queries.size(); ++j)
    {
        if (types[j] == QueryList::DatabaseQuery)
        {
            PyObject *dval = PyString_FromString(queries[j].c_str());
            if(dval == NULL)
                continue;
            PyTuple_SET_ITEM(retval, k++, dval);
        }
    }

    return retval;
}


// ****************************************************************************
// Function: visit_QueriesOverTime
//
// Purpose:
//   Returns a tuple containing the names of queries that can be used with
//   the QueryOverTime command.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   March 23, 2004 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_QueriesOverTime(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    stringVector queries = viewer->GetQueryList()->GetNames();

    // We only want to include Database time queries, so count them.
    intVector types = viewer->GetQueryList()->GetTypes();
    intVector time = viewer->GetQueryList()->GetTimeQuery();
    int nQueries = 0; 
    for(int i = 0; i < types.size(); ++i)
    {
        if (types[i] == QueryList::DatabaseQuery && time[i] == 1)
            nQueries++;
    }

    // Allocate a tuple with enough entries to hold the queries name list.
    PyObject *retval = PyTuple_New(nQueries);

    for(int j = 0, k = 0; j < queries.size(); ++j)
    {
        if (types[j] == QueryList::DatabaseQuery && time[j] == 1)
        {
            PyObject *dval = PyString_FromString(queries[j].c_str());
            if(dval == NULL)
                continue;
            PyTuple_SET_ITEM(retval, k++, dval);
        }
    }

    return retval;
}


// ****************************************************************************
// Function: visit_NumOperatorPlugins
//
// Purpose:
//   Returns the number of operator plugins.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_NumOperatorPlugins(PyObject *self, PyObject *args)
{
    OperatorPluginManager *plugins = OperatorPluginManager::Instance();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    PyObject *retval = PyLong_FromLong((long)plugins->GetNEnabledPlugins());

    return retval;
}

// ****************************************************************************
// Function: visit_PrintWindow
//
// Purpose:
//   Tells the viewer to print the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 14:04:50 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 12:42:22 PDT 2002
//   I made it return a success value.
//
// ****************************************************************************

STATIC PyObject *
visit_PrintWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->PrintWindow();
        if(logging)
            fprintf(logFile, "PrintWindow()\n");
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetWindowArea
//
// Purpose: 
//   This a Python callback that sets the viewer's window area (its workspace)
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 16:55:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetWindowArea(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int x, y, w, h;
    if (!PyArg_ParseTuple(args, "iiii", &x, &y, &w, &h))
        return NULL;

    MUTEX_LOCK();
        viewer->SetWindowArea(x, y, w, h);
        if(logging)
            fprintf(logFile, "SetWindowArea(%d, %d, %d, %d)\n",x,y,w,h);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetAnimationTimeout
//
// Purpose:
//   Tells the viewer to change the animation timeout
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 16 15:01:39 PST 2002
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetAnimationTimeout(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int milliSeconds;
    if (!PyArg_ParseTuple(args, "i", &milliSeconds))
        return NULL;

    if(milliSeconds < 1)
    {
        VisItErrorFunc("The timeout cannot be less than 1.");
        return NULL;
    }

    MUTEX_LOCK();
        AnimationAttributes *atts = viewer->GetAnimationAttributes();
        atts->SetTimeout(milliSeconds);
        atts->Notify();
        viewer->SetAnimationAttributes();
        if(logging)
            fprintf(logFile, "SetAnimationTimeout(%d)\n", milliSeconds);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetAnimationTimeout
//
// Purpose: 
//   Returns the animation timeout.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 16 15:04:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetAnimationTimeout(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    return PyLong_FromLong(long(viewer->GetAnimationAttributes()->GetTimeout()));
}

// ****************************************************************************
// Function: visit_SetPipelineCachingMode
//
// Purpose:
//   Tells the viewer whether or not to cache animations.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 16 15:01:39 PST 2002
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetPipelineCachingMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int val;
    if (!PyArg_ParseTuple(args, "i", &val))
        return NULL;

    MUTEX_LOCK();
        AnimationAttributes *atts = viewer->GetAnimationAttributes();
        atts->SetPipelineCachingMode(val != 0);
        atts->Notify();
        viewer->SetAnimationAttributes();
        if(logging)
            fprintf(logFile, "SetPipelineCachingMode(%d)\n", val);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetPipelineCachingMode
//
// Purpose: 
//   Returns the pipeline caching mode.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 16 15:04:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetPipelineCachingMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    return PyLong_FromLong(long(viewer->GetAnimationAttributes()->GetPipelineCachingMode()?1:0));
}

// ****************************************************************************
// Function: visit_Source
//
// Purpose: 
//   This is a Python callback that opens a Python file and executes it.
//
// Programmer: John Bemis & Brad Whitlock
// Creation:   Tue Sep 18 14:10:54 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Nov 7 09:53:55 PDT 2002
//   I added a check to see if we're logging output before I write the
//   Source command to the log.
//
// ****************************************************************************

STATIC PyObject *
visit_Source(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
        return NULL;

    //
    // Try and open the file.
    //
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        //
        // Add a ".py" extension and try to open the file.
        //
        char fileName2[1024];
        SNPRINTF(fileName2, 1024, "%s.py", fileName);
        fp = fopen(fileName2, "rb");
        if(fp == NULL)
        {
            VisItErrorFunc("Could not find file for sourcing.");
            return NULL;
        }
    }

    //
    // Turn logging off.
    //
    if(logging)
        fprintf(logFile, "Source(\"%s\")\n", fileName);
    SetLogging(false);

    //
    // Execute the commands in the file.
    //
    PyRun_SimpleFile(fp, fileName);
    fclose(fp);

    //
    // Turn logging back on.
    //
    SetLogging(true);

    //
    // Increment the reference count and return.
    //
    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_ToggleMaintainViewMode
//
// Purpose: 
//   Tells the viewer whether or not it should maintain the view limits.
//
// Programmer: Eric Brugger
// Creation:   Fri Apr 18 17:03:31 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleMaintainViewMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleMaintainViewMode();
        if(logging)
            fprintf(logFile, "ToggleMaintainViewMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ToggleMaintainDataMode
//
// Purpose: 
//   Tells the viewer whether or not it should maintain the data limits.
//
// Programmer: Eric Brugger
// Creation:   Mon Apr  5 12:14:06 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleMaintainDataMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleMaintainDataMode();
        if(logging)
            fprintf(logFile, "ToggleMaintainDataMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ToggleLockTime
//
// Purpose: 
//   Tells the viewer to toggle time locking for the active vis window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 23 16:04:03 PST 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleLockTime(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleLockTime();
        if(logging)
            fprintf(logFile, "ToggleLockTime()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ToggleBoundingBoxMode
//
// Purpose:
//   Tells the viewer to toggle its bbox mode.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleBoundingBoxMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleBoundingBoxMode();
        if(logging)
            fprintf(logFile, "ToggleBoundingBoxMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ToggleLockViewMode
//
// Purpose:
//   Tells the viewer to lock current window's view.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:19:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleLockViewMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleLockViewMode();
        if(logging)
            fprintf(logFile, "ToggleLockViewMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ToggleSpinMode
//
// Purpose:
//   Tells the viewer to toggle the spin mode for the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:51:17 PST 2002
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleSpinMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleSpinMode();
        if(logging)
            fprintf(logFile, "ToggleSpinMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_ToggleCameraViewMode
//
// Purpose:
//   Tells the viewer to toggle the camera view mode for the active window.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Fri Jan  3 16:18:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleCameraViewMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleCameraViewMode();
        if(logging)
            fprintf(logFile, "ToggleViewMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}


// ****************************************************************************
// Function: visit_ToggleFullFrameMode
//
// Purpose: 
//   Tells the viewer whether or not it should use full frame mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   May 13, 2003 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleFullFrameMode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ToggleFullFrameMode();
        if(logging)
            fprintf(logFile, "ToggleFullFrameMode()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_UndoView
//
// Purpose:
//   Tells the viewer to undo the last view operation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_UndoView(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->UndoView();
        if(logging)
            fprintf(logFile, "UndoView()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_WriteConfigFile
//
// Purpose:
//   Tells the viewer to write its config file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 12 12:15:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_WriteConfigFile(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->WriteConfigFile();
    MUTEX_LOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_Query
//
// Purpose:
//   Tells the viewer to do a query.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   July 11, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 23 17:37:33 PDT 2003
//   Allow for two optional integer args.
//
//   Kathleen Bonnell, Tue Dec  2 07:34:48 PST 2003  
//   Parse correctly if only 1 integer argument is given. Use new helper 
//   method ParseTupleForVars.
//
//   Kathleen Bonnell, Wed Dec  3 13:11:34 PST 2003 
//   Allow "original" and "actual" to substitute in the vars arg for arg1.
//
//   Brad Whitlock, Tue Mar 2 10:12:44 PDT 2004
//   Made it use GetStringVectorFromPyObject.
//
//   Kathleen Bonnell, Thu Apr 22 15:28:31 PDT 2004 
//   Changed arg1 default to 1 (is used to specify 'actual' data). 
//
//   Kathleen Bonnell, Tue Aug 24 15:31:56 PDT 2004 
//   Changed arg1 default to 0 (is used to specify 'original' data). 
//
// ****************************************************************************

STATIC PyObject *
visit_Query(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *queryName;
    int arg1 = 0, arg2 = 0;
    PyObject *tuple = NULL;
    if (!PyArg_ParseTuple(args, "sii|O", &queryName, &arg1, &arg2, &tuple))
    {
        if (!PyArg_ParseTuple(args, "si|O", &queryName, &arg1, &tuple))
        {
            if (!PyArg_ParseTuple(args, "s|O", &queryName, &tuple))
            {
                return NULL;
            }
        }
        PyErr_Clear();
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);
    if (vars.size() == 1)
    {
        if (strcmp(vars[0].c_str(), "original") == 0)
        {
            arg1 = 0;
            vars.clear();
        }
        else if (strcmp(vars[0].c_str(), "actual") == 0)
        {
            arg1 = 1;
            vars.clear();
        }
    }

    MUTEX_LOCK();
        viewer->DatabaseQuery(queryName, vars, false, arg1, arg2);
        if(logging)
        {
            fprintf(logFile, "Query(%s, (", queryName);
            for(int i = 0; i < vars.size(); ++i)
            {
                fprintf(logFile, "\"%s\"", vars[i].c_str());
                if(i < vars.size()-1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "))\n");
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_QueryOverTime
//
// Purpose:
//   Tells the viewer to do a time-query.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   March 23, 2004 
//
// Modifications:
//   Kathleen Bonnell, Thu Apr 22 15:28:31 PDT 2004 
//   Changed arg1 default to 1 (is used to specify 'actual' data). 
//
// ****************************************************************************

STATIC PyObject *
visit_QueryOverTime(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *queryName;
    int arg1 = 1, arg2 = 0;
    PyObject *tuple = NULL;
    if (!PyArg_ParseTuple(args, "sii|O", &queryName, &arg1, &arg2, &tuple))
    {
        if (!PyArg_ParseTuple(args, "si|O", &queryName, &arg1, &tuple))
        {
            if (!PyArg_ParseTuple(args, "s|O", &queryName, &tuple))
            {
                return NULL;
            }
        }
        PyErr_Clear();
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);
    if (vars.size() == 1)
    {
        if (strcmp(vars[0].c_str(), "original") == 0)
        {
            arg1 = 0;
            vars.clear();
        }
        else if (strcmp(vars[0].c_str(), "actual") == 0)
        {
            arg1 = 1;
            vars.clear();
        }
    }

    MUTEX_LOCK();
        viewer->DatabaseQuery(queryName, vars, true, arg1, arg2);
        if(logging)
        {
            fprintf(logFile, "Query(%s, (", queryName);
            for(int i = 0; i < vars.size(); ++i)
            {
                fprintf(logFile, "\"%s\"", vars[i].c_str());
                if(i < vars.size()-1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "))\n");
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: visit_Pick
//
// Purpose:
//   Tells the viewer to do pick.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 6 14:45:46 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 27 11:18:44 PDT 2002
//   I made it package the pick arguments in a stringVector.
//
//   Kathleen Bonnell, Mon Jun 30 10:48:25 PDT 2003  
//   Allow the vars tuple argument to be optional. 
//
//   Kathleen Bonnell, Wed Jul 23 13:05:01 PDT 2003 
//   Allow pick coordinates to be expressed in a tuple (world Pick). 
//
//   Kathleen Bonnell, Tue Dec  2 07:34:48 PST 2003  
//   Use new helper method ParseTupleForVars.
//
//   Brad Whitlock, Tue Mar 2 10:11:57 PDT 2004
//   I made it use GetDoubleArrayFromPyObject and GetStringVectorFromPyObject.
//
// ****************************************************************************

STATIC PyObject *
visit_Pick(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int x, y;
    PyObject *tuple = NULL;
    PyObject *pt_tuple = NULL;
    bool wp = false;
    if (!PyArg_ParseTuple(args, "ii|O", &x, &y, &tuple))
    {
        if (!PyArg_ParseTuple(args, "O|O", &pt_tuple, &tuple))
        {
            return NULL;
        }
        wp = true;
        PyErr_Clear();
    }

    double pt[3] = {0.,0.,0.};
    if (wp)
    {
        // Extract the world-coordinate point from the first object.
        if(!GetDoubleArrayFromPyObject(pt_tuple, pt, 3))
        {
            VisItErrorFunc("The first argument to WorldPick must be a point "
                           "specified as a tuple of coordinates.");
            return NULL;
        }
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    MUTEX_LOCK();
        if (!wp)
            viewer->Pick(x, y, vars);
        else 
            viewer->Pick(pt,  vars);
        if(logging)
        {
            fprintf(logFile, "Pick(%d, %d, (", x, y);
            for(int i = 0; i < vars.size(); ++i)
            {
                fprintf(logFile, "\"%s\"", vars[i].c_str());
                if(i < vars.size()-1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "))\n");
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: visit_NodePick
//
// Purpose:
//   Tells the viewer to do NodePick.
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   June 25, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 23 13:05:01 PDT 2003 
//   Allow pick coordinates to be expressed in a tuple (world Pick). 
//
//   Kathleen Bonnell, Tue Dec  2 07:34:48 PST 2003  
//   Use new helper method ParseTupleForVars.
//
//   Brad Whitlock, Tue Mar 2 10:10:45 PDT 2004
//   I made it use GetDoubleArrayFromPyObject and GetStringVectorFromPyObject.
//
// ****************************************************************************

STATIC PyObject *
visit_NodePick(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int x, y;
    PyObject *tuple = NULL;
    PyObject *pt_tuple = NULL;
    bool wp = false;
    if (!PyArg_ParseTuple(args, "ii|O", &x, &y, &tuple))
    {
        if (!PyArg_ParseTuple(args, "O|O", &pt_tuple, &tuple))
        {
            return NULL;
        }
        wp = true;
        PyErr_Clear(); 
    }
    // Extract the world-coordinate point from the first object.
    double pt[3] = {0.,0.,0.};
    if (wp)
    {
        if(!GetDoubleArrayFromPyObject(pt_tuple, pt, 3))
        {
            VisItErrorFunc("The first argument to WorldNodePick must be a "
                           "point specified as a tuple of coordinates.");
            return NULL;
        }
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    MUTEX_LOCK();
        if (!wp)
            viewer->NodePick(x, y, vars);
        else 
            viewer->NodePick(pt, vars);
        if(logging)
        {
            fprintf(logFile, "Pick(%d, %d, (", x, y);
            for(int i = 0; i < vars.size(); ++i)
            {
                fprintf(logFile, "\"%s\"", vars[i].c_str());
                if(i < vars.size()-1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "))\n");
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ResetPickLetter
//
// Purpose:
//   Tells the viewer to reset the pick letter used in pick output. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   December 9, 2003
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_ResetPickLetter(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ResetPickLetter();

        if(logging)
            fprintf(logFile, "ResetPickLetter()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ResetLineoutColor
//
// Purpose:
//   Tells the viewer to reset the color used in lineout. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   August 5, 2005 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_ResetLineoutColor(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ResetLineoutColor();

        if(logging)
            fprintf(logFile, "ResetLineoutColor()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ResetPickAttributes
//
// Purpose:
//   Tells the viewer to reset the new pick attributes to default. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   December 1, 2003
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_ResetPickAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ResetPickAttributes();

        if(logging)
            fprintf(logFile, "ResetPickAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_SetPickAttributes
//
// Purpose:
//   Tells the viewer to use the new pick attributes we're sending.
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   June 30, 2003 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetPickAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *pick = NULL;
    // Try and get the pick pointer.
    if(!PyArg_ParseTuple(args,"O",&pick))
    {
        VisItErrorFunc("SetPickAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyPickAttributes_Check(pick))
    {
        VisItErrorFunc("Argument is not a PickAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        PickAttributes *pa = PyPickAttributes_FromPyObject(pick);

        // Copy the object into the pick attributes.
        *(viewer->GetPickAttributes()) = *pa;
        viewer->GetPickAttributes()->Notify();
        viewer->SetPickAttributes();

        if(logging)
            fprintf(logFile, "SetPickAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}


// ****************************************************************************
// Function: visit_SetDefaultPickAttributes
//
// Purpose:
//   Tells the viewer to save the default pick attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 9, 2003 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultPickAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *pick = NULL;
    // Try and get the annotation pointer.
    if(!PyArg_ParseTuple(args,"O",&pick))
    {
        VisItErrorFunc("SetDefaultPickAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyPickAttributes_Check(pick))
    {
        VisItErrorFunc("Argument is not a PickAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        PickAttributes *pa = PyPickAttributes_FromPyObject(pick);

        // Copy the object into the view attributes.
        *(viewer->GetPickAttributes()) = *pa;
        viewer->GetPickAttributes()->Notify();
        viewer->SetDefaultPickAttributes();

        if(logging)
            fprintf(logFile, "SetDefaultPickAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_GetPickAttributes
//
// Purpose:
//   Returns the current pick attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   June 30, 2003 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetPickAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyPickAttributes_NewPyObject();
    PickAttributes *pa = PyPickAttributes_FromPyObject(retval);

    // Copy the viewer proxy's pick atts into the return data structure.
    *pa = *(viewer->GetPickAttributes());

    return retval;
}


// ****************************************************************************
// Function: visit_ResetInteractorAttributes
//
// Purpose:
//   Tells the viewer to reset the new interactor attributes to default. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   August 16, 2004 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_ResetInteractorAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ResetInteractorAttributes();

        if(logging)
            fprintf(logFile, "ResetInteractorAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}


// ****************************************************************************
// Function: visit_SetInteractorAttributes
//
// Purpose:
//   Tells the viewer to use the new Interactor attributes we're sending.
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   August 16, 2004 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetInteractorAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *interactor = NULL;
    // Try and get the interactor attributes pointer.
    if(!PyArg_ParseTuple(args,"O",&interactor))
    {
        VisItErrorFunc("SetInteractorAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyInteractorAttributes_Check(interactor))
    {
        VisItErrorFunc("Argument is not an InteractorAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        InteractorAttributes *ia = PyInteractorAttributes_FromPyObject(interactor);

        // Copy the object into the pick attributes.
        *(viewer->GetInteractorAttributes()) = *ia;
        viewer->GetInteractorAttributes()->Notify();
        viewer->SetInteractorAttributes();

        if(logging)
            fprintf(logFile, "SetInteractorAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}


// ****************************************************************************
// Function: visit_SetDefaultInteractorAttributes
//
// Purpose:
//   Tells the viewer to save the default interactor attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultInteractorAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *interactor = NULL;
    // Try and get the interactorAttributes pointer.
    if(!PyArg_ParseTuple(args,"O",&interactor))
    {
        VisItErrorFunc("SetDefaultInteracotrAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyInteractorAttributes_Check(interactor))
    {
        VisItErrorFunc("Argument is not a InteractorAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        InteractorAttributes *ia = PyInteractorAttributes_FromPyObject(interactor);

        // Copy the object into the view attributes.
        *(viewer->GetInteractorAttributes()) = *ia;
        viewer->GetInteractorAttributes()->Notify();
        viewer->SetDefaultInteractorAttributes();

        if(logging)
            fprintf(logFile, "SetDefaultInteractorAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_GetInteractorAttributes
//
// Purpose:
//   Returns the current interactor attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetInteractorAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyInteractorAttributes_NewPyObject();
    InteractorAttributes *ia = PyInteractorAttributes_FromPyObject(retval);

    // Copy the viewer proxy's pick atts into the return data structure.
    *ia = *(viewer->GetInteractorAttributes());

    return retval;
}


// ****************************************************************************
// Function: visit_ResetQueryOverTimeAttributes
//
// Purpose:
//   Tells the viewer to reset the time query attributes to default. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   March 30, 2004 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_ResetQueryOverTimeAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    MUTEX_LOCK();
        viewer->ResetQueryOverTimeAttributes();

        if(logging)
            fprintf(logFile, "ResetQueryOverTimeAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_SetQueryOverTimeAttributes
//
// Purpose:
//   Tells the viewer to use the time query attributes we're sending.
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   March 30, 2004 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetQueryOverTimeAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *queryOverTime = NULL;
    // Try and get the time query pointer.
    if(!PyArg_ParseTuple(args,"O",&queryOverTime))
    {
        VisItErrorFunc("SetQueryOverTimeAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyQueryOverTimeAttributes_Check(queryOverTime))
    {
        VisItErrorFunc("Argument is not a QueryOverTimeAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        QueryOverTimeAttributes *tqa = 
            PyQueryOverTimeAttributes_FromPyObject(queryOverTime);

        // Copy the object into the pick attributes.
        *(viewer->GetQueryOverTimeAttributes()) = *tqa;
        viewer->GetQueryOverTimeAttributes()->Notify();
        viewer->SetQueryOverTimeAttributes();

        if(logging)
            fprintf(logFile, "SetQueryOverTimeAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}


// ****************************************************************************
// Function: visit_SetDefaultQueryOverTimeAttributes
//
// Purpose:
//   Tells the viewer to save the default time query attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   March 30, 2004 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultQueryOverTimeAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *queryOverTime = NULL;
    // Try and get the queryOverTime pointer.
    if(!PyArg_ParseTuple(args,"O",&queryOverTime))
    {
        VisItErrorFunc("SetDefaultQueryOverTimeAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyQueryOverTimeAttributes_Check(queryOverTime))
    {
        VisItErrorFunc("Argument is not a QueryOverTimeAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        QueryOverTimeAttributes *tqa = 
            PyQueryOverTimeAttributes_FromPyObject(queryOverTime);

        // Copy the object into the time query attributes.
        *(viewer->GetQueryOverTimeAttributes()) = *tqa;
        viewer->GetQueryOverTimeAttributes()->Notify();
        viewer->SetDefaultQueryOverTimeAttributes();

        if(logging)
            fprintf(logFile, "SetDefaultQueryOverTimeAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}

// ****************************************************************************
// Function: visit_GetQueryOverTimeAttributes
//
// Purpose:
//   Returns the current time query attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   March 30, 2004 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetQueryOverTimeAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyQueryOverTimeAttributes_NewPyObject();
    QueryOverTimeAttributes *tqa = PyQueryOverTimeAttributes_FromPyObject(retval);

    // Copy the viewer proxy's pick atts into the return data structure.
    *tqa = *(viewer->GetQueryOverTimeAttributes());

    return retval;
}


// ****************************************************************************
// Function: visit_SetGlobalLineoutAttributes
//
// Purpose:
//   Tells the viewer to use the global lineout attributes we're sending.
//
// Notes:      
//
// Programmer: Kathleen Bonnell
// Creation:   July 22, 2004 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SetGlobalLineoutAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *globalLineout = NULL;
    // Try and get the global lineout pointer.
    if(!PyArg_ParseTuple(args,"O",&globalLineout))
    {
        VisItErrorFunc("SetGlobalLineoutAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyGlobalLineoutAttributes_Check(globalLineout))
    {
        VisItErrorFunc("Argument is not a GlobalLineoutAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        GlobalLineoutAttributes *gla = 
            PyGlobalLineoutAttributes_FromPyObject(globalLineout);

        // Copy the object into the global lineout attributes.
        *(viewer->GetGlobalLineoutAttributes()) = *gla;
        viewer->GetGlobalLineoutAttributes()->Notify();
        viewer->SetGlobalLineoutAttributes();

        if(logging)
            fprintf(logFile, "SetGlobalLineoutAttributes()\n");
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());;
}


// ****************************************************************************
// Function: visit_GetGlobalLineoutAttributes
//
// Purpose:
//   Returns the current global lineout attributes.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   July 22, 2004 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetGlobalLineoutAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = PyGlobalLineoutAttributes_NewPyObject();
    GlobalLineoutAttributes *gla = PyGlobalLineoutAttributes_FromPyObject(retval);

    // Copy the viewer proxy's global lineout atts into the return data structure.
    *gla = *(viewer->GetGlobalLineoutAttributes());

    return retval;
}


// ****************************************************************************
// Function: visit_DomainPick
//
// Purpose:
//   The generic method for PickByZone, PickByNode.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 1, 2003
//
// Modifications:
//   Kathleen Bonnell, Thu Apr  1 20:12:56 PST 2004
//   New bool arg required by PointQuery. 
//
//   Kathleen Bonnell, Tue Jun  1 08:29:54 PDT 2004 
//   Swapped order of dom/el in args list, and in call to PointQuery. 
//
// ****************************************************************************

STATIC PyObject *
visit_DomainPick(const char *type, int el, int dom, stringVector vars)
{
    double pt[3] = {0., 0., 0};

    MUTEX_LOCK();
        viewer->PointQuery(type, pt, vars, false, el, dom);
        if(logging)
        {
            fprintf(logFile, "%s(%d, %d (", type, el, dom);
            for(int i = 0; i < vars.size(); ++i)
            {
                fprintf(logFile, "\"%s\"", vars[i].c_str());
                if(i < vars.size()-1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "))\n");
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: visit_PickByZone
//
// Purpose:
//   Tells the viewer to perform Pick via domain and zone. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 1, 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 2 09:59:30 PDT 2004
//   I made it use GetStringVectorFromPyObject. which is slightly more general.
//
//   Kathleen Bonnell, Tue Jun  1 08:29:54 PDT 2004 
//   Swapped order of dom/zone in call to DomainPick. 
//
// ****************************************************************************

STATIC PyObject *
visit_PickByZone(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *type = "PickByZone";
    int dom = 0, zone = 0;
    PyObject *tuple = NULL;
    if (!PyArg_ParseTuple(args, "ii|O", &zone, &dom, &tuple))
    {
        if (!PyArg_ParseTuple(args, "i|O", &zone, &tuple))
        {
            return NULL;
        }
        PyErr_Clear(); 
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    // Return the success value.
    return visit_DomainPick(type, zone, dom, vars);
}


// ****************************************************************************
// Function: visit_PickByNode
//
// Purpose:
//   Tells the viewer to perform Pick via domain and node. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 1, 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 2 09:59:30 PDT 2004
//   I made it use GetStringVectorFromPyObject. which is slightly more general.
//
//   Kathleen Bonnell, Tue Jun  1 08:29:54 PDT 2004 
//   Swapped order of dom/node in call to DomainPick. 
//
// ****************************************************************************
STATIC PyObject *
visit_PickByNode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *type = "PickByNode";
    int dom = 0, node = 0;
    PyObject *tuple = NULL;
    if (!PyArg_ParseTuple(args, "ii|O", &node, &dom, &tuple))
    {
        if (!PyArg_ParseTuple(args, "i|O", &node, &tuple))
        {
            return NULL;
        }
        PyErr_Clear(); 
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    // Return the success value.
    return visit_DomainPick(type, node, dom, vars);
}


// ****************************************************************************
// Function: visit_Lineout
//
// Purpose:
//   Tells the viewer to perform a lineout.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 6 14:44:58 PST 2002
//
// Modifications:
//    Brad Whitlock, Fri Dec 27 11:20:46 PDT 2002
//    I changed the routine so its arguments are world coordinates.
//
//    Kathleen Bonnell, Wed Jul 23 17:37:33 PDT 2003 
//    Allow an optional integer argument for samples.
//
//    Kathleen Bonnell, Tue Dec  2 07:34:48 PST 2003  
//    Use new helper method ParseTupleForVars.
//
//    Kathleen Bonnell, Fri Mar  5 16:07:06 PST 2004 
//    Set ApplyOperator (to all plots ) to false. 
//
//    Brad Whitlock, Tue Mar 2 10:08:54 PDT 2004
//    I made it use GetDoubleArrayFromPyObject and GetStringVectorFromPyObject.
//
// ****************************************************************************

STATIC PyObject *
visit_Lineout(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *p0tuple = NULL;
    PyObject *p1tuple = NULL;
    PyObject *tuple = NULL;
    int samples = 50;
    // Allow both vars and samples to be optional, but if both are listed,
    // then vars should come first.
    if (!PyArg_ParseTuple(args, "OOOi", &p0tuple, &p1tuple, &tuple, &samples))
    {
        if (!PyArg_ParseTuple(args, "OOi", &p0tuple, &p1tuple, &samples))
        {
            if (!PyArg_ParseTuple(args, "OOO", &p0tuple, &p1tuple, &tuple))
            {
                if (!PyArg_ParseTuple(args, "OO", &p0tuple, &p1tuple ))
                {
                return NULL;
                }
             }
         }
         PyErr_Clear();
    }

    // Extract the starting point from the first object.
    double p0[3] = {0.,0.,0.};
    if(!GetDoubleArrayFromPyObject(p0tuple, p0, 3))
    {
        VisItErrorFunc("The first argument to Lineout must be a point "
                       "specified as a tuple of coordinates.");
        return NULL;
    }
 
    // Extract the starting point from the second object.
    double p1[3] = {0.,0.,0.};
    if(!GetDoubleArrayFromPyObject(p1tuple, p1, 3))
    {
        VisItErrorFunc("The second argument to Lineout must be a point "
                       "specified as a tuple of coordinates.");
        return NULL;
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    MUTEX_LOCK();
        // Lineout should not be applied to more than one plot at a time. 
        viewer->GetGlobalAttributes()->SetApplyOperator(false);
        viewer->GetGlobalAttributes()->Notify();
        viewer->Lineout(p0, p1, vars, samples);
        if(logging)
        {
            fprintf(logFile, "Lineout((%g, %g, %g), (%g, %g, %g), (",
                    p0[0], p0[1], p0[2], p1[0], p1[1], p1[2]);
            for(int i = 0; i < vars.size(); ++i)
            {
                fprintf(logFile, "\"%s\"", vars[i].c_str());
                if(i < vars.size()-1)
                    fprintf(logFile, ", ");
            }
            fprintf(logFile, "))\n");
        }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: CreateAnnotationWrapper
//
// Purpose: 
//   Factory method for creating annotation wrapper objects.
//
// Arguments:
//   annot : The object that we want to wrap.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 17:23:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PyObject *
CreateAnnotationWrapper(AnnotationObject *annot)
{
    PyObject *retval = NULL;

    if(annot->GetObjectType() == AnnotationObject::Text2D)
    {
        // Create a time slider wrapper for the new annotation object.
        retval = PyText2DObject_WrapPyObject(annot);
    }
    else if(annot->GetObjectType() == AnnotationObject::TimeSlider)
    {
        // Create a Text2D wrapper for the new annotation object.
        retval = PyTimeSliderObject_WrapPyObject(annot);
    }

    // Add more cases here later...

    return retval;
}

// ****************************************************************************
// Function: visit_CreateAnnotationObject
//
// Purpose: 
//   Creates an annotation object of the named type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 17:22:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_CreateAnnotationObject(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *annotType;
    if (!PyArg_ParseTuple(args, "s", &annotType))
       return NULL;

    // See if it is an annotation type that we know about
    int annotTypeIndex;
    if(strcmp(annotType, "TimeSlider") == 0)
        annotTypeIndex = 2;
    else if(strcmp(annotType, "Text2D") == 0)
        annotTypeIndex = 0;
    else
    {
        char message[400];
        SNPRINTF(message, 400, "%s is not a recognized annotation type.",
           annotType);
        VisItErrorFunc(message);
        return NULL;
    }

    // Create the annotation.
    MUTEX_LOCK();
        viewer->AddAnnotationObject(annotTypeIndex);
    MUTEX_UNLOCK();
    int errorFlag = Synchronize();

    // If it was a success, then get the last annotation object in the
    // annotation object list and copy it into a Python wrapper class.
    PyObject *retval = 0;
    if(errorFlag == 0)
    {
        MUTEX_LOCK();
            AnnotationObjectList *aol = viewer->GetAnnotationObjectList();
            const AnnotationObject &newObject = aol->operator[](aol->GetNumAnnotationObjects() - 1);

            //
            // Create a copy of the new annotation object that we'll keep in the
            // module's own annotation object list.
            //
            AnnotationObject *localCopy = new AnnotationObject(newObject);
            localObjectList.push_back(localCopy);
            localObjectReferenceCount[localCopy] = 1;
            retval = CreateAnnotationWrapper(localCopy);
        MUTEX_UNLOCK();
    }
    else if(errorFlag > 0)
    {
        char message[400];
        SNPRINTF(message, 400, "VisIt could not create an annotation object "
            "of type: %s.", annotType);
        VisItErrorFunc(message);
        return NULL;
    }

    // Return the success value.
    return retval;
}

// ****************************************************************************
// Function: visit_GetAnnotationObject
//
// Purpose: 
//   Creates an annotation wrapper object for the i'th annotation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 17:22:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetAnnotationObject(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int annotIndex;
    if (!PyArg_ParseTuple(args, "i", &annotIndex))
        return NULL;

    PyObject *retval = NULL;
    if(annotIndex >= 0 && annotIndex < localObjectList.size())
    {
        AnnotationObject *annot = localObjectList[annotIndex];
        retval = CreateAnnotationWrapper(annot);

        // Increase the object's reference count.
        if(retval != 0)
            ++(localObjectReferenceCount[annot]);
    }
    else
    {
        VisItErrorFunc("An invalid annotation object index was given!");
    }

    return retval;
}

// ****************************************************************************
// Function: UpdateAnnotationsHelper
//
// Purpose: 
//   This is a helper function that is called when setting the attributes
//   of an annotation object. The purpose is to send the annotation
//   object list to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 17:21:21 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
UpdateAnnotationHelper(AnnotationObject *annot)
{
    // Make sure that the annotation object annot is in the localObjectList.
    int i, index = -1;
    for(i = 0; i < localObjectList.size(); ++i)
    {
        if(localObjectList[i] == annot)
        {
            index = i;
            break;
        }
    }

    if(index == -1)
    {
        // The annotation was not in the local object list so it must have
        // been deleted.
        VisItErrorFunc("Setting the annotation object attributes for this "
                       "object has no effect because the annotation to which "
                       "it corresponds has been deleted.");
    }
    else
    {
        // The annotation was in the local object list.
        AnnotationObjectList *aol = viewer->GetAnnotationObjectList();
        if(aol->GetNumAnnotationObjects() == localObjectList.size())
        {
            MUTEX_LOCK();

            // Copy the local annotation object's list into the viewer proxy's
            // annotation object list.
            for(i = 0; i < aol->GetNumAnnotationObjects(); ++i)
            {
                AnnotationObject &viewerAnnot = aol->operator[](i);
                viewerAnnot = *(localObjectList[i]);
            }

            // Send the options to the viewer.
            aol->Notify();

            // Make the viewer use the options.
            viewer->SetAnnotationObjectOptions();

            MUTEX_UNLOCK();

            // Synchronize so we don't send more operations than we can handle.
            Synchronize();
        }
        else
        {
            debug1 << "The local annotation object list does not match the viewer's "
                   << "annotation object list!" << endl;
        }
    }
}

// ****************************************************************************
// Function: DeleteAnnotationObjectHelper
//
// Purpose: 
//   This method is called by AnnotationObject wrapper classes so they can
//   delete themselces from the annotation object list.
//
// Arguments:
//   annot : Pointer to the object that we want to delete.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 3 17:20:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
DeleteAnnotationObjectHelper(AnnotationObject *annot)
{
    bool transferOwnership = false;

    MUTEX_LOCK();

    bool needToDelete = false;
    AnnotationObjectList *aol = viewer->GetAnnotationObjectList();
    if(aol->GetNumAnnotationObjects() == localObjectList.size())
    {
        std::vector<AnnotationObject *> compactedObjectList;

        // Delete the specified annotation object and make it active so the viewer
        // can delete it in the vis window.
        for(int i = 0; i < localObjectList.size(); ++i)
        { 
            AnnotationObject &viewerAnnot = aol->operator[](i);
            if(localObjectList[i] == annot)
            {
                //
                // Decrement the AnnotationObject's reference count. If the reference
                // count reaches zero, remove it from the localObjectReferenceCount
                // map and set transferOwnership to true so that when the object that's
                // deleting the annotation is the last one to reference the annotation
                // object, it gets the annotation object to keep. That way, when the
                // wrapper object gets deleted, so does the annotation object.
                //
                --(localObjectReferenceCount[annot]);
                if(localObjectReferenceCount[annot] < 2)
                {
                    localObjectReferenceCount.erase(localObjectReferenceCount.find(annot));
                    transferOwnership = true;
                }

                viewerAnnot.SetActive(true);
                needToDelete = true;
            }
            else
            {
                compactedObjectList.push_back(localObjectList[i]);
                viewerAnnot.SetActive(false);
                localObjectList[i]->SetActive(false);
            }
        }

        //
        // Replace the old local object list with the new one without the object
        // that was removed. After that, make sure that the first object in the
        // local object list is made active.
        //
        localObjectList = compactedObjectList;
        if(localObjectList.size() > 0)
            localObjectList[0]->SetActive(true);

        // Now that we've modified the annotation object list, we have to send
        // it to the viewer.
        aol->SelectAll();
        aol->Notify();
        viewer->SetAnnotationObjectOptions();
    }
    MUTEX_UNLOCK();
    Synchronize();

    // Tell the viewer to delete the active annotations.
    if(needToDelete)
    {
        MUTEX_LOCK();
        viewer->DeleteActiveAnnotationObjects();
        MUTEX_UNLOCK();

        Synchronize();
    }

    return transferOwnership;
}

//
// Method table
//

std::vector<PyMethodDef> VisItMethods;

// ****************************************************************************
// Function: AddMethod
//
// Purpose:
//   This function adds a method to the VisIt module's Python method table.
//
// Arguments:
//   methodName : The name of the method.
//   cb         : The Python callback function.
//   doc        : The documentation string for the method.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:36:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

static void
AddMethod(const char *methodName, PyObject *(cb)(PyObject *, PyObject *),
          const char *doc = NULL)
{
    PyMethodDef newMethod;
    newMethod.ml_name = (char *)methodName;
    newMethod.ml_meth = cb;
    newMethod.ml_flags = METH_VARARGS;
    newMethod.ml_doc = (char *)doc;
    VisItMethods.push_back(newMethod);
}

// ****************************************************************************
// Function: AddDefaultMethods
//
// Purpose:
//   This function adds all of the default methods to the VisIt module's Python
//   method table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:36:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 20 14:02:18 PST 2002
//   Added GetLocalHostName, GetLocalUserName, PrintWindow, SetWindowArea
//   methods.
//
//   Brad Whitlock, Thu Mar 7 18:04:01 PST 2002
//   Added ReplaceDatabase and OverlayDatabase methods.
//
//   Brad Whitlock, Mon May 6 17:31:18 PST 2002
//   Added a bunch of new methods that can control the viewer's popup menu.
//
//   Hank Childs, Fri May 24 08:37:28 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.
//
//   Brad Whitlock, Thu Jun 27 16:50:17 PST 2002
//   Added new methods to copy attributes from one window to anothe window.
//
//   Brad Whitlock, Fri Jul 26 12:13:41 PDT 2002
//   I removed InterruptComputeEngine and added GetLastError. I also added
//   ReOpenDatabase and ClearCache functions.
//
//   Brad Whitlock, Fri Aug 16 15:09:48 PST 2002
//   I added functions to set/get the animation timeout and the pipeline
//   caching mode.
//
//   Brad Whitlock, Tue Sep 24 09:34:30 PDT 2002
//   I added methods to set the view extents type and clear the reflines.
//
//   Brad Whitlock, Fri Sep 6 14:43:48 PST 2002
//   I added Pick and Lineout methods.
//
//   Brad Whitlock, Wed Oct 16 10:20:28 PDT 2002
//   I added CloneWindow and CopyPlotsToWindow methods.
//
//   Jeremy Meredith, Thu Oct 24 16:48:15 PDT 2002
//   Added material options.
//
//   Brad Whitlock, Wed Nov 13 09:26:14 PDT 2002
//   I added GetDomains, GetMaterials, ListDomains, ListMaterials,
//   TurnDomainsOff, TurnDomainsOn, TurnMaterialsOff, TurnMaterialsOn.
//
//   Eric Brugger, Tue Nov 19 10:53:43 PST 2002
//   I added GetKeyframeAttributes, SetKeyframeAttributes, DeletePlotKeyframe
//   and SetPlotFrameRange.
//
//   Brad Whitlock, Tue Nov 19 12:44:09 PDT 2002
//   I added more expression definition methods.
//
//   Eric Brugger, Mon Dec 16 10:20:27 PST 2002
//   I added AnimationSetNFrames.  I renamed GetAnimationNumStates to
//   GetDatabaseNStates.
//
//   Eric Brugger, Mon Dec 30 13:16:58 PST 2002 
//   I added SetPlotDatabaseState and DeletePlotDatabaseKeyframe.
//
//   Eric Brugger, Fri Jan  3 16:18:44 PST 2003
//   I added ClearViewKeyframes, DeleteViewKeyframe, SetViewKeyframe and
//   ToggleCameraViewMode.
//
//   Brad Whitlock, Mon Jan 13 10:59:03 PDT 2003
//   I added OpenMDServer.
//
//   Eric Brugger, Tue Jan 28 13:53:33 PST 2003
//   I added MovePlotKeyframe, MovePlotDatabaseKeyframe and MoveViewKeyframe.
// 
//   Brad Whitlock, Thu Apr 17 15:43:12 PST 2003
//   I added PromoteOperator, DemoteOperator, and RemoveOperator.
//
//   Eric Brugger, Fri Apr 18 17:03:31 PDT 2003
//   I replaced ToggleAutoCenterMode with ToggleMaintainViewMode.
//
//   Kathleen Bonnell, Thu May 15 10:26:00 PDT 2003  
//   I added ToggleFullFrameMode.
//
//   Hank Childs, Thu May 22 18:34:24 PDT 2003
//   Added SurfaceArea.
//
//   Kathleen Bonnell, Wed Jun 25 13:27:59 PDT 2003 
//   Added NodePick, ZonePick. (ZonePick == Pick).
//
//   Kathleen Bonnell, Fri Jul 11 15:53:04 PDT 2003 
//   Replace SurfaceArea with generic Query.  Added GetQueryOutputString,
//   GetQueryOutputValue. 
//
//   Kathleen Bonnell, Wed Jul 23 13:05:01 PDT 2003 
//   Added WorldPick and WorldNodePick. 
//
//   Brad Whitlock, Mon Jul 28 16:40:37 PST 2003
//   Added AnimationGetNFrames, SaveSession, RestoreSession.
//
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Added SetViewCurve.
//
//   Brad Whitlock, Fri Aug 29 11:11:30 PDT 2003
//   Added HideToolbars and ShowToolbars.
//
//   Kathleen Bonnell, Mon Dec  1 18:04:41 PST 2003 
//   Added PickByNode, PickByZone and ResetPickAttributes.
//
//   Brad Whitlock, Wed Dec 3 17:25:06 PST 2003
//   Added CreateAnnotationObject and GetAnnotationObject.
//
//   Brad Whitlock, Fri Dec 5 14:15:04 PST 2003
//   Added LongFileName and ShowAllWindows.
//
//   Eric Brugger, Thu Dec 18 15:29:44 PST 2003
//   Added SetCloneWindowOnFirstRef.
//
//   Brad Whitlock, Tue Dec 30 10:58:07 PDT 2003
//   Added SetCenterOfRotation and ChooseCenterOfRotation.
//
//   Brad Whitlock, Thu Feb 26 13:40:40 PST 2004
//   Added ClearCacheForAllEngines.
//
//   Brad Whitlock, Tue Mar 2 09:21:19 PDT 2004
//   Added methods to set the time slider state. I also deprected most of
//   the old Animation functions. Added ActivateDatabase,
//   AlterDatabaseCorrelation, DeleteDatabaseCorrelation,
//   CheckForNewStates, CloseDatabase, GetGlobalAttributes,
//   GetActiveTimeSlider, GetTimeSliders, SetDatabaseCorrelationOptions,
//   ToggleLockTime.
//
//   Kathleen Bonnell, Wed Mar 31 11:38:36 PST 2004 
//   Added methods in support of queries over time: GetQueryOverTimeAttributes,
//   QueryOverTime, SetQueryOverTimeAttributes, ResetQueryOverTimeAttributes,
//   SetDefaultQueryOverTimeAttributes, QueriesOverTime.
//
//   Eric Brugger, Mon Apr  5 12:14:06 PDT 2004
//   Added ToggleMaintainDataMode.
//
//   Kathleen Bonnell, Thu Jul 22 15:57:23 PDT 2004
//   Added Set/Get GlobalLineoutAttributes.
//
//   Kathleen Bonnell, Thu Aug  5 10:44:22 PDT 2004 
//   Added resetLineoutColor.
//
// ****************************************************************************

static void
AddDefaultMethods()
{
    //
    // Methods that can affect how the viewer is run. They also provide
    // information to the script about how VisIt is being run. These can be
    // called before the viewer is launched.
    //
    AddMethod("AddArgument", visit_AddArgument);
    AddMethod("Close",  visit_Close);
    AddMethod("Launch", visit_Launch);
    AddMethod("LaunchNowin", visit_LaunchNowin);
    AddMethod("LocalNameSpace", visit_LocalNameSpace);
    AddMethod("GetDebugLevel", visit_GetDebugLevel);
    AddMethod("GetLastError", visit_GetLastError);
    AddMethod("SetDebugLevel", visit_SetDebugLevel);
    AddMethod("Version", visit_Version);
    AddMethod("LongFileName", visit_LongFileName);

    //
    // Viewer proxy methods.
    //
    AddMethod("ActivateDatabase", visit_ActivateDatabase);
    AddMethod("AddOperator", visit_AddOperator);
    AddMethod("AddPlot", visit_AddPlot);
    AddMethod("AddWindow",  visit_AddWindow);
    AddMethod("AlterDatabaseCorrelation", visit_AlterDatabaseCorrelation);
    AddMethod("AnimationSetNFrames", visit_AnimationSetNFrames);
    AddMethod("ChangeActivePlotsVar", visit_ChangeActivePlotsVar);
    AddMethod("CheckForNewStates", visit_CheckForNewStates);
    AddMethod("ChooseCenterOfRotation",  visit_ChooseCenterOfRotation);
    AddMethod("ClearAllWindows", visit_ClearAllWindows);
    AddMethod("ClearCache", visit_ClearCache);
    AddMethod("ClearCacheForAllEngines", visit_ClearCacheForAllEngines);
    AddMethod("ClearPickPoints", visit_ClearPickPoints);
    AddMethod("ClearReferenceLines", visit_ClearReferenceLines);
    AddMethod("ClearViewKeyframes", visit_ClearViewKeyframes);
    AddMethod("ClearWindow", visit_ClearWindow);
    AddMethod("CloneWindow",  visit_CloneWindow);
    AddMethod("CloseComputeEngine", visit_CloseComputeEngine);
    AddMethod("CloseDatabase", visit_CloseDatabase);
    AddMethod("CopyAnnotationsToWindow", visit_CopyAnnotationsToWindow);
    AddMethod("CopyLightingToWindow", visit_CopyLightingToWindow);
    AddMethod("CopyPlotsToWindow", visit_CopyPlotsToWindow);
    AddMethod("CopyViewToWindow", visit_CopyViewToWindow);
    AddMethod("CreateAnnotationObject", visit_CreateAnnotationObject);
    AddMethod("CreateDatabaseCorrelation", visit_CreateDatabaseCorrelation);
    AddMethod("DefineMeshExpression", visit_DefineMeshExpression);
    AddMethod("DefineMaterialExpression", visit_DefineMaterialExpression);
    AddMethod("DefineScalarExpression", visit_DefineScalarExpression);
    AddMethod("DefineSpeciesExpression", visit_DefineSpeciesExpression);
    AddMethod("DefineVectorExpression", visit_DefineVectorExpression);
    AddMethod("DeleteDatabaseCorrelation", visit_DeleteDatabaseCorrelation);
    AddMethod("DeleteExpression", visit_DeleteExpression);
    AddMethod("DeIconifyAllWindows", visit_DeIconifyAllWindows);
    AddMethod("DeleteActivePlots", visit_DeleteActivePlots);
    AddMethod("DeleteAllPlots", visit_DeleteAllPlots);
    AddMethod("DeletePlotDatabaseKeyframe", visit_DeletePlotDatabaseKeyframe);
    AddMethod("DeletePlotKeyframe", visit_DeletePlotKeyframe);
    AddMethod("DeleteViewKeyframe", visit_DeleteViewKeyframe);
    AddMethod("DeleteWindow", visit_DeleteWindow);
    AddMethod("DemoteOperator", visit_DemoteOperator);
    AddMethod("DisableRedraw", visit_DisableRedraw);
    AddMethod("DrawPlots", visit_DrawPlots);
    AddMethod("EnableTool", visit_EnableTool);
    AddMethod("GetAnimationTimeout", visit_GetAnimationTimeout);
    AddMethod("GetAnnotationObject", visit_GetAnnotationObject);
    AddMethod("GetLocalHostName", visit_GetLocalHostName);
    AddMethod("GetLocalUserName", visit_GetLocalUserName);
    AddMethod("GetSaveWindowAttributes", visit_GetSaveWindowAttributes);
    AddMethod("GetView2D", visit_GetView2D);
    AddMethod("GetView3D", visit_GetView3D);
    AddMethod("GetAnnotationAttributes", visit_GetAnnotationAttributes);
    AddMethod("GetDatabaseNStates", visit_GetDatabaseNStates);
    AddMethod("GetEngineList", visit_GetEngineList);
    AddMethod("GetGlobalAttributes", visit_GetGlobalAttributes);
    AddMethod("GetGlobalLineoutAttributes", visit_GetGlobalLineoutAttributes);
    AddMethod("GetInteractorAttributes", visit_GetInteractorAttributes);
    AddMethod("GetKeyframeAttributes", visit_GetKeyframeAttributes);
    AddMethod("GetMaterialAttributes", visit_GetMaterialAttributes);
    AddMethod("GetPickAttributes", visit_GetPickAttributes);
    AddMethod("GetPickOutput", visit_GetPickOutput);
    AddMethod("GetPipelineCachingMode", visit_GetPipelineCachingMode);
    AddMethod("GetQueryOutputString", visit_GetQueryOutputString);
    AddMethod("GetQueryOutputValue", visit_GetQueryOutputValue);
    AddMethod("GetRenderingAttributes", visit_GetRenderingAttributes);
    AddMethod("GetQueryOverTimeAttributes", visit_GetQueryOverTimeAttributes);
    AddMethod("GetWindowInformation", visit_GetWindowInformation);
    AddMethod("HideActivePlots", visit_HideActivePlots);
    AddMethod("HideToolbars", visit_HideToolbars);
    AddMethod("IconifyAllWindows", visit_IconifyAllWindows);
    AddMethod("InvertBackgroundColor", visit_InvertBackgroundColor);
    AddMethod("Lineout", visit_Lineout);
    AddMethod("MovePlotDatabaseKeyframe", visit_MovePlotDatabaseKeyframe);
    AddMethod("MovePlotKeyframe", visit_MovePlotKeyframe);
    AddMethod("MoveViewKeyframe", visit_MoveViewKeyframe);
    AddMethod("NodePick", visit_NodePick);
    AddMethod("OpenDatabase", visit_OpenDatabase);
    AddMethod("OpenComputeEngine", visit_OpenComputeEngine);
    AddMethod("OpenMDServer", visit_OpenMDServer);
    AddMethod("OverlayDatabase", visit_OverlayDatabase);
    AddMethod("Pick", visit_Pick);
    AddMethod("PickByNode", visit_PickByNode);
    AddMethod("PickByZone", visit_PickByZone);
    AddMethod("PrintWindow", visit_PrintWindow);
    AddMethod("PromoteOperator", visit_PromoteOperator);
    AddMethod("Query", visit_Query);
    AddMethod("QueryOverTime", visit_QueryOverTime);
    AddMethod("RecenterView", visit_RecenterView);
    AddMethod("RedrawWindow", visit_RedrawWindow);
    AddMethod("RemoveAllOperators", visit_RemoveAllOperators);
    AddMethod("RemoveLastOperator", visit_RemoveLastOperator);
    AddMethod("RemoveOperator", visit_RemoveOperator);
    AddMethod("ReOpenDatabase", visit_ReOpenDatabase);
    AddMethod("ReplaceDatabase", visit_ReplaceDatabase);
    AddMethod("ResetLineoutColor", visit_ResetLineoutColor);
    AddMethod("ResetOperatorOptions", visit_ResetOperatorOptions);
    AddMethod("ResetPickAttributes", visit_ResetPickAttributes);
    AddMethod("ResetPickLetter", visit_ResetPickLetter);
    AddMethod("ResetPlotOptions", visit_ResetPlotOptions);
    AddMethod("ResetQueryOverTimeAttributes", visit_ResetQueryOverTimeAttributes);
    AddMethod("ResetView", visit_ResetView);
    AddMethod("RestoreSession", visit_RestoreSession);
    AddMethod("SaveSession", visit_SaveSession);
    AddMethod("SaveWindow", visit_SaveWindow);
    AddMethod("SetActivePlots", visit_SetActivePlots);
    AddMethod("SetActiveTimeSlider", visit_SetActiveTimeSlider);
    AddMethod("SetActiveWindow", visit_SetActiveWindow);
    AddMethod("SetAnimationTimeout", visit_SetAnimationTimeout);
    AddMethod("SetAnnotationAttributes", visit_SetAnnotationAttributes);
    AddMethod("SetCenterOfRotation",  visit_SetCenterOfRotation);
    AddMethod("SetCloneWindowOnFirstRef", visit_SetCloneWindowOnFirstRef);
    AddMethod("SetDefaultAnnotationAttributes", visit_SetDefaultAnnotationAttributes);
    AddMethod("SetDefaultInteractorAttributes", visit_SetDefaultInteractorAttributes);
    AddMethod("SetDefaultMaterialAttributes", visit_SetDefaultMaterialAttributes);
    AddMethod("SetDefaultOperatorOptions", visit_SetDefaultOperatorOptions);
    AddMethod("SetDefaultPickAttributes", visit_SetDefaultPickAttributes);
    AddMethod("SetDefaultPlotOptions", visit_SetDefaultPlotOptions);
    AddMethod("SetDefaultQueryOverTimeAttributes", visit_SetDefaultQueryOverTimeAttributes);
    AddMethod("SetGlobalLineoutAttributes", visit_SetGlobalLineoutAttributes);
    AddMethod("SetInteractorAttributes", visit_SetInteractorAttributes);
    AddMethod("SetKeyframeAttributes", visit_SetKeyframeAttributes);
    AddMethod("SetMaterialAttributes", visit_SetMaterialAttributes);
    AddMethod("SetOperatorOptions", visit_SetOperatorOptions);
    AddMethod("SetPickAttributes", visit_SetPickAttributes);
    AddMethod("SetPipelineCachingMode", visit_SetPipelineCachingMode);
    AddMethod("SetPlotDatabaseState", visit_SetPlotDatabaseState);
    AddMethod("SetPlotFrameRange", visit_SetPlotFrameRange);
    AddMethod("SetPlotOptions", visit_SetPlotOptions);
    AddMethod("SetPlotSILRestriction", visit_SetPlotSILRestriction);
    AddMethod("SetPrinterAttributes", visit_SetPrinterAttributes);
    AddMethod("SetRenderingAttributes", visit_SetRenderingAttributes);
    AddMethod("SetSaveWindowAttributes", visit_SetSaveWindowAttributes);
    AddMethod("SetQueryOverTimeAttributes", visit_SetQueryOverTimeAttributes);
    AddMethod("SetTimeSliderState", visit_SetTimeSliderState);
    AddMethod("SetViewExtentsType", visit_SetViewExtentsType);
    AddMethod("SetViewCurve", visit_SetViewCurve);
    AddMethod("SetView2D", visit_SetView2D);
    AddMethod("SetView3D", visit_SetView3D);
    AddMethod("SetViewKeyframe", visit_SetViewKeyframe);
    AddMethod("SetWindowArea",  visit_SetWindowArea);
    AddMethod("SetWindowLayout",  visit_SetWindowLayout);
    AddMethod("SetWindowMode",  visit_SetWindowMode);
    AddMethod("ShowAllWindows",  visit_ShowAllWindows);
    AddMethod("ShowToolbars", visit_ShowToolbars);
    AddMethod("TimeSliderGetNStates",visit_TimeSliderGetNStates );
    AddMethod("TimeSliderNextState", visit_TimeSliderNextState);
    AddMethod("TimeSliderPreviousState", visit_TimeSliderPreviousState);
    AddMethod("ToggleBoundingBoxMode", visit_ToggleBoundingBoxMode);
    AddMethod("ToggleCameraViewMode", visit_ToggleCameraViewMode);
    AddMethod("ToggleFullFrameMode", visit_ToggleFullFrameMode);
    AddMethod("ToggleLockTime", visit_ToggleLockTime);
    AddMethod("ToggleLockViewMode", visit_ToggleLockViewMode);
    AddMethod("ToggleMaintainViewMode", visit_ToggleMaintainViewMode);
    AddMethod("ToggleMaintainDataMode", visit_ToggleMaintainDataMode);
    AddMethod("ToggleSpinMode", visit_ToggleSpinMode);
    AddMethod("UndoView",  visit_UndoView);
    AddMethod("WorldPick", visit_Pick);
    AddMethod("WorldNodePick", visit_NodePick);
    AddMethod("WriteConfigFile",  visit_WriteConfigFile);
    AddMethod("ZonePick", visit_Pick);

    //
    // Deprecated ViewerProxy methods. Remove in 1.4.
    //
    AddMethod("AnimationNextFrame", visit_AnimationNextFrame);
    AddMethod("AnimationPreviousFrame", visit_AnimationPreviousFrame);
    AddMethod("AnimationSetFrame", visit_AnimationSetFrame);
    AddMethod("AnimationGetNFrames", visit_AnimationGetNFrames);

    //
    // Extra methods that are not part of the ViewerProxy but allow the
    // script writer to do interesting things.
    //
    AddMethod("Source", visit_Source);
    AddMethod("ListPlots", visit_ListPlots);
    AddMethod("Expressions", visit_Expressions);
    AddMethod("GetActiveTimeSlider", visit_GetActiveTimeSlider);
    AddMethod("GetDomains", visit_GetDomains);
    AddMethod("GetMaterials", visit_GetMaterials);
    AddMethod("GetTimeSliders", visit_GetTimeSliders);
    AddMethod("ListDomains", visit_ListDomains);
    AddMethod("ListMaterials", visit_ListMaterials);
    AddMethod("NumOperatorPlugins", visit_NumOperatorPlugins);
    AddMethod("NumPlotPlugins", visit_NumPlotPlugins);
    AddMethod("OperatorPlugins", visit_OperatorPlugins);
    AddMethod("PlotPlugins", visit_PlotPlugins);
    AddMethod("Queries",  visit_Queries);
    AddMethod("SetDatabaseCorrelationOptions", visit_SetDatabaseCorrelationOptions);
    AddMethod("TurnDomainsOff",  visit_TurnDomainsOff);
    AddMethod("TurnDomainsOn",  visit_TurnDomainsOn);
    AddMethod("TurnMaterialsOff",  visit_TurnMaterialsOff);
    AddMethod("TurnMaterialsOn",  visit_TurnMaterialsOn);
    AddMethod("QueriesOverTime",  visit_QueriesOverTime);

    // Temporary methods
    AddMethod("ColorTableNames", visit_ColorTableNames);
    AddMethod("NumColorTableNames", visit_NumColorTables);
    AddMethod("SetActiveContinuousColorTable", visit_SetActiveContinuousColorTable);
    AddMethod("SetActiveDiscreteColorTable", visit_SetActiveDiscreteColorTable);
    AddMethod("GetActiveContinuousColorTable", visit_GetActiveContinuousColorTable);
    AddMethod("GetActiveDiscreteColorTable", visit_GetActiveDiscreteColorTable);
    AddMethod("GetNumPlots", visit_GetNumPlots);
}

// ****************************************************************************
// Function: AddExtensions
//
// Purpose: 
//   This function adds all of the type extensions to the VisIt module.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:43:13 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 20 14:18:03 PST 2002
//   Added the PrinterAttributes extension.
//
//   Hank Childs, Fri May 24 08:37:28 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.
//
//   Brad Whitlock, Tue Sep 24 09:47:16 PDT 2002
//   I added WindowInformation and RenderingAttributes.
//
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Added ViewCurveAttributes, View2DAttributes and View3DAttributes.
//
//   Brad Whitlock, Fri Mar 19 08:50:53 PDT 2004
//   Added GlobalAttributes.
//
// ****************************************************************************

static void
AddExtensions()
{
    int          i, nMethods;
    PyMethodDef *methods;

    ADD_EXTENSION(PyAnnotationAttributes_GetMethodTable);
    ADD_EXTENSION(PyGlobalAttributes_GetMethodTable);
    ADD_EXTENSION(PyHostProfile_GetMethodTable);
    ADD_EXTENSION(PyMaterialAttributes_GetMethodTable);
    ADD_EXTENSION(PyPrinterAttributes_GetMethodTable);
    ADD_EXTENSION(PyRenderingAttributes_GetMethodTable);
    ADD_EXTENSION(PySaveWindowAttributes_GetMethodTable);
    ADD_EXTENSION(PySILRestriction_GetMethodTable);
    ADD_EXTENSION(PyViewAttributes_GetMethodTable);
    ADD_EXTENSION(PyViewCurveAttributes_GetMethodTable);
    ADD_EXTENSION(PyView2DAttributes_GetMethodTable);
    ADD_EXTENSION(PyView3DAttributes_GetMethodTable);
    ADD_EXTENSION(PyWindowInformation_GetMethodTable);
}

// ****************************************************************************
// Function: InitializeExtensions
//
// Purpose: 
//   This function is called after the viewer is running and we have the
//   default values from the config file. This function calls the startup
//   function for the extensions.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 7 11:20:02 PDT 2002
//
// Modifications:
//   Hank Childs, Fri May 24 08:37:28 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.
//
//   Jeremy Meredith, Thu Oct 24 16:48:15 PDT 2002
//   Added material options.
//
//   Brad Whitlock, Thu Nov 7 09:57:35 PDT 2002
//   I initialized some extensions.
//
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Removed ViewAttributes and added ViewCurveAttributes, View2DAttributes
//   and View3DAttributes.
//
//   Brad Whitlock, Fri Mar 19 08:51:39 PDT 2004
//   Added GlobalAttributes.
//
// ****************************************************************************

static void
InitializeExtensions()
{
    PyAnnotationAttributes_StartUp(viewer->GetAnnotationAttributes(), logFile);
    PyGlobalAttributes_StartUp(viewer->GetGlobalAttributes(), logFile);
    PyHostProfile_StartUp(0, logFile);
    PyMaterialAttributes_StartUp(viewer->GetMaterialAttributes(), logFile);
    PyPrinterAttributes_StartUp(viewer->GetPrinterAttributes(), logFile);
    PyRenderingAttributes_StartUp(viewer->GetRenderingAttributes(), logFile);
    PySaveWindowAttributes_StartUp(viewer->GetSaveWindowAttributes(), logFile);
    PyViewCurveAttributes_StartUp(viewer->GetViewCurveAttributes(), logFile);
    PyView2DAttributes_StartUp(viewer->GetView2DAttributes(), logFile);
    PyView3DAttributes_StartUp(viewer->GetView3DAttributes(), logFile);
    PyWindowInformation_StartUp(viewer->GetWindowInformation(), logFile);
}

// ****************************************************************************
// Function: CloseExtensions
//
// Purpose: 
//   This method closes down all of the extensions.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:42:44 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Thu Oct 24 16:48:15 PDT 2002
//   Added material options.
//
//   Eric Brugger, Wed Aug 20 14:20:25 PDT 2003
//   Removed ViewAttributes and added ViewCurveAttributes, View2DAttributes
//   and View3DAttributes.
//
//   Brad Whitlock, Fri Mar 19 08:52:05 PDT 2004
//   Added GlobalAttributes.
//
// ****************************************************************************

static void
CloseExtensions()
{
    PyAnnotationAttributes_CloseDown();
    PyGlobalAttributes_CloseDown();
    PyMaterialAttributes_CloseDown();
    PyPrinterAttributes_CloseDown();
    PySaveWindowAttributes_CloseDown();
    PyViewCurveAttributes_CloseDown();
    PyView2DAttributes_CloseDown();
    PyView3DAttributes_CloseDown();
}

// ****************************************************************************
// Function: SetLogging
//
// Purpose: 
//   This function sets the flag that determines whether or not extensions
//   and plugins write their changes to the log file when they change.
//
// Arguments:
//   val : Whether or not to log output.
//
// Note:       This function is used to prevent extra information from being
//             added to the log file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:40:50 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Thu Oct 24 16:48:15 PDT 2002
//   Added material options.
//
//   Brad Whitlock, Thu Nov 7 09:50:03 PDT 2002
//   I added some modules that should have been called but were not.
//
//   Brad Whitlock, Fri Mar 19 08:52:27 PDT 2004
//   Added GlobalAttributes.
//
// ****************************************************************************

static void
SetLogging(bool val)
{
    if(logFile == NULL)
        logging = false;
    else
        logging = val;

    //
    // Set the logging flag in the extensions.
    //
    PyAnnotationAttributes_SetLogging(val);
    PyGlobalAttributes_SetLogging(val);
    PyMaterialAttributes_SetLogging(val);
    PyPrinterAttributes_SetLogging(val);
    PyRenderingAttributes_SetLogging(val);
    PySaveWindowAttributes_SetLogging(val);
    PyViewAttributes_SetLogging(val);
    PyWindowInformation_SetLogging(val);

    //
    // Set the logging flag in the plot plugins.
    //
    int i;
    PlotPluginManager *ppManager = PlotPluginManager::Instance();
    for(i = 0; i < ppManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(ppManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info = ppManager->GetScriptingPluginInfo(id);
        info->SetLogging(val);
    }

    //
    // Set the logging flag in the operator plugins.
    //
    OperatorPluginManager *opManager = OperatorPluginManager::Instance();
    for(i = 0; i < opManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(opManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info = opManager->GetScriptingPluginInfo(id);
        info->SetLogging(val);
    }
}

// ****************************************************************************
// Function: PlotPluginAddInterface
//
// Purpose:
//   This function loads all of the plot scripting plugins.
//
// Programmer: John Bemis & Brad Whitlock
// Creation:   Tue Sep 4 15:36:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 14:08:20 PST 2003
//   I made it use a mutex so the debug logs don't get clobbered by competing
//   threads.
//
// ****************************************************************************

static void
PlotPluginAddInterface()
{
    MUTEX_LOCK();

    // Get a pointer to the plot plugin manager.
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();

    // Create the window and populate the menu for each plot plugin.
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        // Initialize the plugin.
        debug1 << "Initializing "
               << info->GetName()
               << " plot plugin." << endl;
        info->InitializePlugin(viewer->GetPlotAttributes(i), logFile);

        // Add the plugin's methods to the visit module's table of methods.
        int  nMethods = 0;
        void *methods = info->GetMethodTable(&nMethods);
        if(nMethods > 0)
        {
            PyObject *d;
            if(localNameSpace)
                d = PyEval_GetLocals();
            else
                d = PyModule_GetDict(visitModule);

            PyMethodDef *method = (PyMethodDef *)methods;
            for(int j = 0; j < nMethods; ++j, ++method)
            {
                debug1 << "\tAdded \"" << method->ml_name << "\" method."
                       << endl;

                // Add the method to the dictionary.
                PyObject *v = PyCFunction_New(method, Py_None);
                if(v == NULL)
                    continue;
                if(PyDict_SetItemString(d, method->ml_name, v) != 0)
                    continue;
                Py_DECREF(v);
            }
        }
    }

    MUTEX_UNLOCK();
}

// ****************************************************************************
// Function: OperatorPluginAddInterface
//
// Purpose:
//   This function loads all of the operator scripting plugins.
//
// Programmer: John Bemis & Brad Whitlock
// Creation:   Tue Sep 4 15:36:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 14:08:20 PST 2003
//   I made it use a mutex so the debug logs don't get clobbered by competing
//   threads.
//   
// ****************************************************************************

static void
OperatorPluginAddInterface()
{
    MUTEX_LOCK();

    // Get a pointer to the plot plugin manager.
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();

    // Create the window and populate the menu for each plot plugin.
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        // Initialize the plugin.
        debug1 << "Initializing "
               << pluginManager->GetCommonPluginInfo(id)->GetName()
               << " operator plugin." << endl;
        info->InitializePlugin(viewer->GetOperatorAttributes(i), logFile);

        // Add the plugin's methods to the visit module's table of methods.
        int  nMethods = 0;
        void *methods = info->GetMethodTable(&nMethods);
        if(nMethods > 0)
        {
            PyObject *d;
            if(localNameSpace)
                d = PyEval_GetLocals();
            else
                d = PyModule_GetDict(visitModule);

            PyMethodDef *method = (PyMethodDef *)methods;
            for(int j = 0; j < nMethods; ++j, ++method)
            {
                debug1 << "\tAdded \"" << method->ml_name << "\" method."
                       << endl;

                // Add the method to the dictionary.
                PyObject *v = PyCFunction_New(method, Py_None);
                if(v == NULL)
                    continue;
                if(PyDict_SetItemString(d, method->ml_name, v) != 0)
                    continue;
                Py_DECREF(v);
            }
        }
    }

    MUTEX_UNLOCK();
}

// ****************************************************************************
// Method: DelayedLoadPlugins
//
// Purpose: 
//   This function is called when the viewer tells us what plugins it has.
//   With that information, we load plugins and add them to the interface
//   that the VisIt module exposes to Python.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 9 16:25:32 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon May 6 14:00:34 PST 2002
//   Moved to function calls up one level.
//
// ****************************************************************************

static void
DelayedLoadPlugins()
{
    // Start reading from the viewer. This will quit when we get a signal
    // from the viewer insicating that we need to load plugins. This means
    // that the NeedToLoadPlugins() function will be called from
    visit_eventloop(0);

    // Delete the plugin loader observer so the NeedToLoadPlugins() function
    // is never called again.
    delete pluginLoader;
    pluginLoader = 0;

    // Tell the viewer proxy to load its plugins now that we know which
    // ones we need to load.
    viewer->LoadPlugins();
}

// ****************************************************************************
// Function: NeedToLoadPlugins
//
// Purpose: 
//   This is a callback function for the pluginLoader observer to callback
//   object.
//
// Notes:
//   It looks for the first occurrance of the plugin attributes in the stream
//   of information coming from the viewer. This function is only called once.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 9 19:57:29 PST 2001
//
// Modifications:
//   
// ****************************************************************************

static void
NeedToLoadPlugins(Subject *, void *)
{
    keepGoing = false;
}

// ****************************************************************************
// Function: InitializeModule
//
// Purpose:
//   This function initializes the viewer proxy and creates a second thread of
//   execution that reads state back from the viewer.
//
// Programmer: John Bemis & Brad Whitlock
// Creation:   Tue Sep 4 15:36:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jan 4 17:34:04 PST 2002
//   Added -noint flag to the viewer command line.
//
//   Brad Whitlock, Tue Feb 12 16:36:27 PST 2002
//   Fixed -debug arugments.
//
//   Brad Whitlock, Thu Nov 7 09:51:07 PDT 2002
//   I changed the code so that if the log file cannot be opened, we
//   disable logging.
//
//   Brad Whitlock, Mon Dec 16 13:42:24 PST 2002
//   I added a status observer.
//
//   Brad Whitlock, Tue Jan 7 16:30:01 PST 2003
//   I moved the Python termination function for VisIt so it is before
//   the call to Init::Initialize because I wanted to add some debug stream
//   code into the termination function.
//
//   Brad Whitlock, Thu Dec 18 16:00:04 PST 2003
//   I added an observer that helps us do thread synchronization that does
//   not involve polling so we don't waste the CPU.
//
// ****************************************************************************

static int
InitializeModule()
{
    bool ret = false;

    // Register a close-down function to close the viewer.
    Py_AtExit(terminatevisit);

    TRY
    {
        int argc = 1;
        char *argv[3];
        argv[0] = "cli";

        if(moduleDebugLevel > 0)
        {
            static char *nums[] = {"1", "2", "3", "4", "5"};
            argv[argc++] = "-debug";
            argv[argc++] = nums[moduleDebugLevel - 1];
        }

        Init::Initialize(argc, argv, 0, 1, false);
        Init::SetComponentName("cli");

        // Read the plugins.
        PlotPluginManager::Initialize(PlotPluginManager::Scripting);
        OperatorPluginManager::Initialize(OperatorPluginManager::Scripting);
    }
    CATCH(VisItException)
    {
        // Return that we could not initialize VisIt.
        ret = true;
    }
    ENDTRY

    // If there was an initialization error, return.
    if(ret)
    {
        viewer = 0;
        return 1;
    }

    //
    // Create the viewer proxy and add some default arguments.
    //
    viewer = new ViewerProxy;

    //
    // Ensure that the viewer will be run in a mode that does not check for
    // interruption when reading back from the engine.
    //
    viewer->AddArgument("-noint");

    //
    // Add the debuglevel argument to the viewer proxy.
    //
    if(moduleDebugLevel > 0)
    {
        viewer->AddArgument("-debug");
        char tmp[10];
        SNPRINTF(tmp, 10, "%d", moduleDebugLevel);
        viewer->AddArgument(tmp);        
    }

    //
    // Add the optional command line arguments coming from cli_argv.
    //
    for(int i = 0; i < cli_argc; ++i)
        viewer->AddArgument(cli_argv[i]);

    //
    // Hook up observers
    //
    messageObserver = new VisItMessageObserver(viewer->GetMessageAttributes());
    statusObserver = new VisItStatusObserver(viewer->GetStatusAttributes());
    statusObserver->SetVerbose(moduleVerbose);
    pluginLoader = new ObserverToCallback(viewer->GetPluginManagerAttributes(),
                                          NeedToLoadPlugins);
#ifndef POLLING_SYNCHRONIZE
    synchronizeCallback = new ObserverToCallback(viewer->GetSyncAttributes(),
                                                 WakeMainThread);
#endif

    //
    // Open the log file
    //
    logFile = fopen("visit.py", "wb");
    if(logFile)
    {
        fprintf(logFile, "# Visit %s log file\n", VERSION);
        fprintf(logFile, "ScriptVersion = \"%s\"\n", VERSION);
        fprintf(logFile, "if ScriptVersion != Version():\n");
        fprintf(logFile, "    print \"This script is for VisIt %%s. "
                "It may not work with version %%s\" %% "
                "(ScriptVersion, Version())\n");
    }
    else
    {
        fprintf(stderr, "Could not open visit.py log file.\n");
        logging = false;
    }

    // Add the default methods to the module's method table.
    AddDefaultMethods();
    // Add extension methods to the module's method table.
    AddExtensions();
    // Mark the end of the method table.
    AddMethod(NULL, NULL);

    // Set the module initialized flag.
    moduleInitialized = true;

    return 0;
}

// ****************************************************************************
// Function: LaunchViewer
//
// Purpose:
//   This function launches the viewer and creates the listening thread.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 16:57:11 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 4 16:54:56 PST 2002
//   Removed code to set the geometry, borders, shift, preshift.
//
//   Sean Ahern, Tue Apr 16 14:37:47 PDT 2002
//   I made the viewer not show windows by default.  Thus, after we start
//   up the viewer, we have to tell it to show its windows.
//
// ****************************************************************************

static void
LaunchViewer()
{
    TRY
    {
        //
        // Try and connect to the viewer.
        //
        viewer->Create();

        //
        // Tell the windows to show themselves
        //
        viewer->ShowAllWindows();

        //
        // Set a flag indicating the viewer exists.
        //
        noViewer = false;
    }
    CATCH(VisItException)
    {
        noViewer = true;
    }
    ENDTRY
}

// ****************************************************************************
// Function: CreateListenerThread
//
// Purpose:
//   Creates that thread that listens for and processes information from
//   the viewer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 9 19:29:16 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Apr 18 10:03:09 PDT 2002
//   Added a Windows implementation.
//
// ****************************************************************************

static void
CreateListenerThread()
{
    keepGoing = true;

#ifdef THREADS
    //
    // Create a new thread and make it execute the event loop that reads
    // input from the viewer.
    //
#if defined(_WIN32)
    // Create the thread with the WIN32 API.
    DWORD Id;
    if(CreateThread(0, 0, visit_eventloop, (LPVOID)0, 0, &Id) == INVALID_HANDLE_VALUE)
    {
        moduleUseThreads = false;
        fprintf(stderr, "Could not create event loop thread.\n");
    }
#else
    // Create the thread using PThreads.
    pthread_t tid;
    if(pthread_create(&tid, &thread_atts, visit_eventloop, (void *)0) == -1)
    {
        moduleUseThreads = false;
        fprintf(stderr, "Could not create event loop thread.\n");
    }
#endif
#endif
}

// ****************************************************************************
// Function: CloseModule
//
// Purpose: 
//   This function is used to close down the viewer and all of the objects
//   that are used by the VisIt module.
//
// Note:       This function is called by terminatevisit.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:45:53 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 7 16:25:24 PST 2003
//   I added code to write closing information to the debug logs.
//
// ****************************************************************************

static void
CloseModule()
{
    keepGoing = false;

    // Delete the observers
    delete messageObserver;
    delete pluginLoader;

    // Make each extension delete its observer
    debug1 << "Closing the extensions." << endl;
    CloseExtensions();

    // Close the viewer.
    if(viewer)
    {
        debug1 << "Telling the viewer to close." << endl;
        viewer->Close();
        delete viewer;
        viewer = 0;
        debug1 << "The viewer closed." << endl;
    }
    else
        debug1 << "There was no viewer to close." << endl;

    // Indicate that the module has been de-initialized.
    moduleInitialized = false;

    // Indicate that the viewer no longer exists.
    noViewer = true;
}

// ****************************************************************************
// Function: VisItErrorFunc
//
// Purpose: 
//   This function sets the Python error string if we're allowing Python
//   exceptions to be thrown from the VisIt module.
//
// Arguments:
//   errString : The error string that is "thrown".
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:44:43 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisItErrorFunc(const char *errString)
{
    PyErr_SetString(VisItError, errString);
}

// ****************************************************************************
// Function: cli_initvisit
//
// Purpose:
//   This function is called by programs wishing to embed the Python
//   interpreter and VisIt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 7 15:43:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Dec 16 13:45:28 PST 2002
//   I added moduleVerbose.
//
// ****************************************************************************

void
cli_initvisit(int debugLevel, bool verbose, int argc, char **argv)
{
    moduleDebugLevel = debugLevel;
    moduleVerbose = verbose;
    localNameSpace = true;
    cli_argc = argc;
    cli_argv = argv;
    initvisit();
}

// ****************************************************************************
// Function: cli_runscript
//
// Purpose: 
//   This function executes the Python script stored in the specified file.
//
// Arguments:
//   fileName : The name of the file to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 10:03:17 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
cli_runscript(const char *fileName)
{
    // If there was a file to execute, do it.
    if(fileName != 0)
    {
        FILE *fp = fopen(fileName, "r");
        if(fp)
        {
            PyRun_SimpleFile(fp, (char *)fileName);
            fclose(fp);
        }
        else
        {
            fprintf(stderr, "The file %s could not be opened.\n", fileName);
        }
    }
}

// ****************************************************************************
// Function: VisItViewer
//
// Purpose:
//   Returns a read-only pointer to the viewer proxy.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 18 11:19:00 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const ViewerProxy *
VisItViewer()
{
    return viewer;
}

// ****************************************************************************
// Function: initscriptfunctions
//
// Purpose:
//   This function executes some Python commands that define new VisIt module
//   functions. This allows us to include Python functions in this C++
//   extension module.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 17 17:24:00 PST 2001
//
// Modifications:
//   
// ****************************************************************************

static void
initscriptfunctions()
{
    PyRun_SimpleString((char*)(visit_EvalLinear));
    PyRun_SimpleString((char*)(visit_EvalQuadratic));
    PyRun_SimpleString((char*)(visit_EvalCubic));
    PyRun_SimpleString((char*)(visit_EvalCubicSpline));
}

// ****************************************************************************
// Function: initvisit
//
// Purpose: 
//   This is the module initialization function that is called by Python.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:57:53 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Dec 17 17:22:06 PST 2001
//   Added a call to initscriptfunctions.
//
//   Brad Whitlock, Thu Dec 18 16:10:30 PST 2003
//   Added a call to create the mutex used for a condition variable. I also
//   renamed the error exception to VisItException to make it more like the
//   C++ version since we can actually trap for the exception now.
//
// ****************************************************************************

void
initvisit()
{
    int initCode = 0;

    //
    // Initialize the module, but only do it one time.
    //
    if(!moduleInitialized)
    {
        MUTEX_CREATE();
#ifndef POLLING_SYNCHRONIZE
        SYNC_CREATE();
#endif
        THREAD_INIT();
        initCode = InitializeModule();
    }

    PyObject *d;

    // Add the VisIt module to Python. Note that we're passing the address
    // of the first element of a vector.
    visitModule = Py_InitModule("visit", &VisItMethods[0]);

    // Add the Python error message.
    d = PyModule_GetDict(visitModule);
    VisItError = PyErr_NewException("visit.VisItException", NULL, NULL);
    PyDict_SetItemString(d, "VisItException", VisItError);

    // Define builtin visit functions that are written in python.
    initscriptfunctions();

    // If the call to LaunchViewer returned an error code then print an error.
    if(initCode == 1)
        VisItErrorFunc("Could not initialize VisIt!");
}

// ****************************************************************************
// Function: terminatevisit
//
// Purpose: 
//   This function is called when Python exits. Its job is to close down the 
//   viewer if it is not already closed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 15:58:44 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Dec 18 16:11:25 PST 2003
//   Added a call to destroy the synchronization mutex.
//
// ****************************************************************************

static void
terminatevisit()
{
    // Delete the VisIt objects.
    CloseModule();

    // Close the log file
    if(logFile)
        fclose(logFile);

    MUTEX_DESTROY();
#ifndef POLLING_SYNCHRONIZE
    SYNC_DESTROY();
#endif
}

// ****************************************************************************
// Function: visit_eventloop
//
// Purpose:
//   This function is a thread callback function that is executed by the
//   thread we created to read new state information back from the viewer.
//
// Notes:      This function makes use of the global variable "keepGoing" to
//             determine if it should keep executing the event loop.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 25 15:29:35 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Apr 3 13:05:03 PST 2002
//   Rewrote so it is more portable.
//
//   Brad Whitlock, Fri Dec 19 13:20:47 PST 2003
//   I added code to unlock the mutex if there is a lost connection exception
//   so control is able to go back to thread 1, which is still alive. I also
//   set the noViewer flag to true to indicate that the viewer is dead so
//   the appropriate error message gets set when we attempt to enter one of
//   our visit methods without a live viewer. Finally, I added code to wake up
//   thread 1 if we're not using a polling synchronize.
//
// ****************************************************************************

#if defined(_WIN32)
static DWORD WINAPI
visit_eventloop(LPVOID)
#else
static void *
visit_eventloop(void *)
#endif
{
    // This is the event loop for the messaging thread. If it needs to read
    // input from the viewer, it does so and executes the Notify method of
    // all subjects that changed.
    while(keepGoing)
    {
        // Block until we have input to read.
        if(viewer->GetWriteConnection()->NeedsRead(true))
        {
            TRY
            {
                // Process input.
                MUTEX_LOCK();
                    viewer->ProcessInput();
                MUTEX_UNLOCK();
            }
            CATCH(LostConnectionException)
            {
                // We lost the viewer, terminate the event loop.
                keepGoing = false;

                // If we got to this point, it means that the mutex is
                // still locked and we must unlock it to ensure that
                // thread 1 can continue.
                MUTEX_UNLOCK();

                //
                // Indicate that there is no viewer.
                //
                noViewer = true;

#ifndef POLLING_SYNCHRONIZE
                SYNC_WAKE_MAIN_THREAD();
#endif
            }
            ENDTRY
        }
        else
        {
            keepGoing = false;
        }
    }

    return NULL;
}

// ****************************************************************************
// Function: Synchronize
//
// Purpose: 
//   This function sends a syncAtts object to the viewer and waits for the
//   viewer to send it back.
//
// Note:       This function must *not* be called within a critical section.
//
// Returns:    0 - success
//             1 - failure
//            -1 - failure and viewer died
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:28:30 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 26 11:55:14 PDT 2002
//   I made it return a value indicating if VisIt had an error while
//   processing the remaining commands.
//
//   Brad Whitlock, Thu Dec 18 16:04:02 PST 2003
//   I added support for synchonization that does not involve polling. I also
//   made it return a value of -1 if the viewer happened to die while we were
//   synchronizing. In that case, I also added code to set the error string.
//
// ****************************************************************************

static int
Synchronize()
{
    const char *terminationMsg = "VisIt's viewer has terminated abnormally!";

    // Clear any error flag in the message observer.
    messageObserver->ClearError();

    // Return if the thread initialization failed.
    if(!moduleUseThreads)
        return 0;

    //
    // If the 2nd thread is not running, don't enter this method or we'll
    // be in here forever.
    //
    if(!keepGoing)
    {
        VisItErrorFunc(terminationMsg);
        return -1;
    }

    SyncAttributes *syncAtts = viewer->GetSyncAttributes();

#ifndef POLLING_SYNCHRONIZE
    SYNC_MUTEX_LOCK();
#endif

    // Send the syncAtts to the viewer and then set the proxy's syncAtts tag
    // to -1. This will allow us to loop until the viewer sends back the
    // correct value.
    ++syncCount;
    MUTEX_LOCK();
    syncAtts->SetSyncTag(syncCount);
#ifndef POLLING_SYNCHRONIZE
    synchronizeCallback->SetUpdate(false);
#endif
    syncAtts->Notify();
    syncAtts->SetSyncTag(-1);
    MUTEX_UNLOCK();

    // Disable logging.
    bool logEnabled = logging;
    if(logEnabled)
        SetLogging(false);

#ifndef POLLING_SYNCHRONIZE
    SYNC_COND_WAIT();
#else
    while((syncAtts->GetSyncTag() != syncCount) && keepGoing)
    {
        // Nothing here.
    }
#endif

    // Enable logging.
    if(logEnabled)
        SetLogging(true);

    // If the viewer has terminated while we were waiting for the sync tag
    // then call the error function now that we're in thread 1.
    if(!keepGoing)
    {
        VisItErrorFunc(terminationMsg);
    }

    // Return whether or not there is an error in the message observer.
    // Also indicate an error if the viewer is dead.
    return keepGoing ? messageObserver->ErrorFlag() : -1;
}
