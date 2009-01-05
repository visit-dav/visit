/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <stdio.h>
#include <visit-config.h> // for VERSION
#if !defined(_WIN32)
#include <strings.h>
#else
#include <process.h> // for _getpid
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
#else
#include <pthread.h>
#endif
#endif

#include <visitmodulehelpers.h>
#include <Connection.h>
#include <Observer.h>
#include <ObserverToCallback.h>
#include <ViewerProxy.h>
#include <ViewerRPC.h>
#include <VisItException.h>
#include <LostConnectionException.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <QueryList.h>
#include <VisItInit.h>
#include <DebugStream.h>
#include <StringHelpers.h>
#include <Logging.h>
#include <SingleAttributeConfigManager.h>

//
// State object include files.
//
#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <ClientMethod.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <ColorTableAttributes.h>
#include <ConstructDDFAttributes.h>
#include <DatabaseCorrelationList.h>
#include <DatabaseCorrelation.h>
#include <DBPluginInfoAttributes.h>
#include <EngineList.h>
#include <ExportDBAttributes.h>
#include <FileOpenOptions.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <InteractorAttributes.h>
#include <KeyframeAttributes.h>
#include <MeshManagementAttributes.h>
#include <MessageAttributes.h>
#include <PickAttributes.h>
#include <Plot.h>
#include <PlotInfoAttributes.h>
#include <PlotList.h>
#include <PluginManagerAttributes.h>
#include <ProcessAttributes.h>
#include <QueryAttributes.h>
#include <PrinterAttributes.h>
#include <RenderingAttributes.h>
#include <StatusAttributes.h>
#include <SyncAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <ViewAttributes.h>
#include <ViewAxisArrayAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <LightList.h>

//
// Extension include files.
//
#include <PyAnimationAttributes.h>
#include <PyAnnotationAttributes.h>
#include <PyAxes2D.h>
#include <PyAxes3D.h>
#include <PyAxisAttributes.h>
#include <PyAxisLabels.h>
#include <PyAxisTickMarks.h>
#include <PyAxisTitles.h>
#include <PyColorAttribute.h>
#include <PyColorAttributeList.h>
#include <PyColorControlPoint.h>
#include <PyColorControlPointList.h>
#include <PyConstructDDFAttributes.h>
#include <PyDatabaseCorrelation.h>
#include <PyExportDBAttributes.h>
#include <PyFileOpenOptions.h>
#include <PyFontAttributes.h>
#include <PyGaussianControlPoint.h>
#include <PyGaussianControlPointList.h>
#include <PyGlobalAttributes.h>
#include <PyGlobalLineoutAttributes.h>
#include <PyHostProfile.h>
#include <PyImageObject.h>
#include <PyInteractorAttributes.h>
#include <PyKeyframeAttributes.h>
#include <PyLegendAttributesObject.h>
#include <PyLineObject.h>
#include <PyLightAttributes.h>
#include <PyMaterialAttributes.h>
#include <PyMeshManagementAttributes.h>
#include <PyPickAttributes.h>
#include <PyPlotList.h>
#include <PyPrinterAttributes.h>
#include <PyProcessAttributes.h>
#include <PyRenderingAttributes.h>
#include <PySaveWindowAttributes.h>
#include <PySILRestriction.h>
#include <PyText2DObject.h>
#include <PyText3DObject.h>
#include <PyQueryOverTimeAttributes.h>
#include <PyTimeSliderObject.h>
#include <PyViewAttributes.h>
#include <PyViewAxisArrayAttributes.h>
#include <PyViewCurveAttributes.h>
#include <PyView2DAttributes.h>
#include <PyView3DAttributes.h>
#include <PyWindowInformation.h>
#include <PyavtDatabaseMetaData.h>
#include <PyViewerRPC.h>

// Variant & MapNode Helpers:
#include <PyVariant.h>
#include <PyMapNode.h>

#include <CallbackManager.h>
#include <ViewerRPCCallbacks.h>
#include <CallbackHandlers.h>
#include <MethodDoc.h>

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

#define NO_ARGUMENTS() if(!PyArg_ParseTuple(args, "")) return NULL;

//
// Make the initvisit function callable from C.
//
extern "C"
{
    void initvisit();
    void initvisit2();
    void cli_initvisit(int, bool, int, char **, int, char **);
    void cli_runscript(const char *);

    // Expose these functions so we can call them from a facade
    // VisIt module from "import visit" inside of Python.
    PyObject *visit_Launch(PyObject *, PyObject *);
    PyObject *visit_SetDebugLevel(PyObject *, PyObject *);
    PyObject *visit_AddArgument(PyObject *, PyObject *);
}

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
static void LaunchViewer(const char *);
static int  Synchronize();
static void DelayedLoadPlugins();
static void PlotPluginAddInterface();
static void OperatorPluginAddInterface();
static void InitializeExtensions();
static void ExecuteClientMethod(ClientMethod *method, bool onNewThread);

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
//   Mark C. Miller, Tue Nov 27 10:44:29 PST 2007
//   Added suppression level; 4 means nothing is suppressed; 1 means
//   everything is suppressed.
//
//   Brad Whitlock, Fri Jan 18 15:00:41 PST 2008
//   Added Information printing.
//
class VisItMessageObserver : public Observer
{
public:
    VisItMessageObserver(Subject *s) : Observer(s), lastError("")
    {
        errorFlag = 0;
        suppressLevel = 4;
    };

    virtual ~VisItMessageObserver() { };

    void ClearError()
    {
        errorFlag = 0;
    };
    int SetSuppressLevel(int newLevel)
    {
        int oldLevel = suppressLevel;
        suppressLevel = newLevel;
        return oldLevel;
    };

    int GetSuppressLevel() const { return suppressLevel; };
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

            if (suppressLevel > 1)
                fprintf(stderr, "VisIt: Error - %s\n", m->GetText().c_str());
//            // This can't really be done this way since this code is only
//            // ever called by the 2nd thread. It cannot use Python.
//            VisItErrorFunc(m->GetText().c_str());
        }
        else if(m->GetSeverity() == MessageAttributes::ErrorClear)
            errorFlag = 0;
        else if(m->GetSeverity() == MessageAttributes::Warning &&
            suppressLevel > 2)
            fprintf(stderr, "VisIt: Warning - %s\n", m->GetText().c_str());
        else if (suppressLevel > 3)
        {
            if(m->GetSeverity() == MessageAttributes::Message)
                fprintf(stderr, "VisIt: Message - %s\n", m->GetText().c_str());
            else
                fprintf(stderr, "VisIt: Information - %s\n", m->GetText().c_str());
        }
    }
private:
    int         errorFlag;
    int         suppressLevel;
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
                fprintf(stderr, "%s\n", statusAtts->GetStatusMessage().c_str());
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
ViewerProxy                 *viewer = 0;

static PyObject             *visitModule = 0;
static bool                  moduleInitialized = false;
static bool                  keepGoing = true;
static bool                  viewerInitiatedQuit = false;
static bool                  viewerBlockingRead = false;
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
static ObserverToCallback   *clientMethodObserver = 0;
static ObserverToCallback   *stateLoggingObserver = 0;
static bool                  localNameSpace = false;
static bool                  interruptScript = false;
static int                   syncCount = 1000;
static PyObject             *VisItError;
static PyObject             *VisItInterrupt;

static int                   cli_argc = 0;
static char                **cli_argv = 0;
static int                   cli_argc_after_s = 0;
static char                **cli_argv_after_s = 0;

static PyThreadState        *mainThreadState = NULL;

static bool                  clientMethodsAllowed = false;
static std::vector<ClientMethod *> cachedClientMethods;

static std::map<std::string, PyObject*> macroFunctions;

static CallbackManager      *callbackMgr = NULL;
static ViewerRPCCallbacks   *rpcCallbacks = NULL;

typedef struct
{
    AnnotationObject *object;
    int               index;
    int               refCount;
} AnnotationObjectRef;

static std::map<std::string, AnnotationObjectRef> localObjectMap;

#ifdef THREADS
#if defined(_WIN32)
static CRITICAL_SECTION      mutex;
static HANDLE                threadHandle = INVALID_HANDLE_VALUE;
static bool                  waitingForViewer = false;
static ObserverToCallback   *synchronizeCallback = 0;
static HANDLE                received_sync_from_viewer = INVALID_HANDLE_VALUE;
#define THREAD_INIT()
#define MUTEX_CREATE()       InitializeCriticalSection(&mutex)
#define MUTEX_DESTROY() 
#define MUTEX_LOCK()         EnterCriticalSection(&mutex)
#define MUTEX_UNLOCK()       LeaveCriticalSection(&mutex)

#define SYNC_COND_WAIT()     MUTEX_UNLOCK(); \
                             if(keepGoing) \
                             {\
                                 waitingForViewer = true; \
                                 WaitForSingleObject(received_sync_from_viewer, INFINITE); \
                                 waitingForViewer = false; \
                             }

#define SYNC_WAKE_MAIN_THREAD() if(waitingForViewer) \
                                    WakeMainThread(0, 0);

#define SYNC_CREATE()        received_sync_from_viewer = CreateEvent(NULL, FALSE, FALSE, NULL);

#define SYNC_DESTROY()       CloseHandle(received_sync_from_viewer);

// ****************************************************************************
// Function: WakeMainThread
//
// Purpose: 
//   Called by the listener thread when SyncAttributes is read from the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 20 12:00:06 PST 2006
//
// Modifications:
//
// ****************************************************************************

static void WakeMainThread(Subject *, void *)
{
    if(GetViewerState()->GetSyncAttributes()->GetSyncTag() == syncCount || !keepGoing)
        SetEvent(received_sync_from_viewer);
}

#else
// pthreads thread-related stuff.
static pthread_attr_t        thread_atts;
static pthread_mutex_t       mutex;
static pthread_cond_t        received_sync_from_viewer;
static bool waitingForViewer = false;
static ObserverToCallback   *synchronizeCallback = 0;
#define THREAD_INIT()        pthread_attr_init(&thread_atts)
#define MUTEX_CREATE()       pthread_mutex_init(&mutex, NULL)
#define MUTEX_DESTROY()      pthread_mutex_destroy(&mutex)
#define MUTEX_LOCK()         pthread_mutex_lock(&mutex)
#define MUTEX_UNLOCK()       pthread_mutex_unlock(&mutex)

#define SYNC_COND_WAIT()     if(keepGoing) \
                             {\
                                 waitingForViewer = true; \
                                 pthread_cond_wait(&received_sync_from_viewer, \
                                                   &mutex); \
                                 waitingForViewer = false; \
                             }\
                             MUTEX_UNLOCK();

#define SYNC_WAKE_MAIN_THREAD() if(waitingForViewer) \
                                    WakeMainThread(0, 0);
#define SYNC_CREATE()        pthread_cond_init(&received_sync_from_viewer, NULL);

#define SYNC_DESTROY()       pthread_cond_destroy(&received_sync_from_viewer);


// ****************************************************************************
// Function: WakeMainThread
//
// Purpose: 
//   Called by the listener thread when SyncAttributes is read from the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 27 09:58:06 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Jun 27 10:01:06 PDT 2005
//   I removed the secondary mutexes since this method gets called only from
//   the listener thread, which has already locked the viewer mutex.
//
// ****************************************************************************

static void WakeMainThread(Subject *, void *)
{
    if(GetViewerState()->GetSyncAttributes()->GetSyncTag() == syncCount || !keepGoing)
        pthread_cond_signal(&received_sync_from_viewer);
}

#endif
#else
#define POLLING_SYNCHRONIZE
#define MUTEX_CREATE()
#define MUTEX_DESTROY()
#define MUTEX_LOCK()
#define MUTEX_UNLOCK()
#define THREAD_INIT()
#endif

// Locks the Python interpreter by one thread.
PyThreadState *
VisItLockPythonInterpreter()
{
    // get the global lock
    PyEval_AcquireLock();

    // get a reference to the PyInterpreterState
    PyInterpreterState * mainInterpreterState = mainThreadState->interp;
    // create a thread state object for this thread
    PyThreadState *myThreadState = PyThreadState_New(mainInterpreterState);
    // swap in my thread state
    PyThreadState_Swap(myThreadState);
    return myThreadState;
}

// Unlocks the Python interpreter by one thread.
void
VisItUnlockPythonInterpreter(PyThreadState *myThreadState)
{
    // clear the thread state
    PyThreadState_Swap(NULL);
    // clear out any cruft from thread state object
    PyThreadState_Clear(myThreadState);
    // delete my thread state object
    PyThreadState_Delete(myThreadState);
    // release our hold on the global interpreter
    PyEval_ReleaseLock();
}

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
// Method: StringVectorToTupleString
//
// Purpose: 
//   Converts a stringVector into a suitable string representation of a Python
//   tuple.
//
// Arguments:
//   s : The string vector.
//
// Returns:    A string representation of the string tuple.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 10 14:02:23 PST 2006
//
// Modifications:
//   
// ****************************************************************************

static std::string
StringVectorToTupleString(const stringVector &s)
{
    std::string str;

    if(s.size() > 1)
        str += "(";
    for(int i = 0; i < s.size(); ++i)
    {
        str += "\"";
        str += s[i];
        str += "\"";
        if(i < s.size()-1)
            str += ", ";
    }
    if(s.size() > 1)
        str += "(";

    return str;
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


// ****************************************************************************
//  Method:  FillDBOptionsFromDictionary
//
//  Purpose:
//    Take a dictionary of key-value pairs, ("boolval":1, "stringval":"foobar")
//    and applies it to a DBOptionsAttributes structure.  The error
//    reporting should hopefully be sufficient to guide users.
//
//  Arguments:
//    obj        the dictionary
//    opts       the DBOptionsAttributes to fill
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 11, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 25 10:05:37 EDT 2007
//    Added support for pre-2.5 versions of Python.
//
// ****************************************************************************
bool
FillDBOptionsFromDictionary(PyObject *obj, DBOptionsAttributes &opts)
{
    if (!obj)
        return false;

    char msg[256];
    bool isdict = PyDict_Check(obj);
    if (!isdict)
    {
        VisItErrorFunc("Expected a dictionary for DB Options");
        return false;
    }

    PyObject *key, *value;
#if PY_VERSION_HEX >= 0x02050000
    Py_ssize_t pos = 0;
#else
    int pos = 0;
#endif

    while (PyDict_Next(obj, &pos, &key, &value))
    {
        std::string name;
        if (PyString_Check(key))
            name = PyString_AS_STRING(key);
        else
        {
            VisItErrorFunc("The key for an option must be a string.");
            return false;
        }

        int index = -1;
        for (int j=0; j<opts.GetNumberOfOptions(); j++)
        {
            if (name == opts.GetName(j))
            {
                index = j;
                break;
            }
        }
        if (index == -1)
        {
            sprintf(msg, "There was no '%s' in the DB options.", name.c_str());
            VisItErrorFunc(msg);
            return false;
        }

        switch (opts.GetType(index))
        {
          case DBOptionsAttributes::Bool:
            if (PyInt_Check(value))
                opts.SetBool(name, PyInt_AS_LONG(value));
            else
            {
                sprintf(msg, "Expected int to set boolean '%s'", name.c_str());
                VisItErrorFunc(msg);
                return false;
            }
            break;
          case DBOptionsAttributes::Int:
            if (PyInt_Check(value))
                opts.SetInt(name, PyInt_AS_LONG(value));
            else
            {
                sprintf(msg, "Expected integer to set '%s'", name.c_str());
                VisItErrorFunc(msg);
                return false;
            }
            break;
          case DBOptionsAttributes::Float:
            if (PyFloat_Check(value))
                opts.SetFloat(name, PyFloat_AS_DOUBLE(value));
            else if (PyInt_Check(value))
                opts.SetFloat(name, PyInt_AS_LONG(value));
            else
            {
                sprintf(msg, "Expected float to set '%s'", name.c_str());
                VisItErrorFunc(msg);
                return false;
            }
            break;
          case DBOptionsAttributes::Double:
            if (PyFloat_Check(value))
                opts.SetDouble(name, PyFloat_AS_DOUBLE(value));
            else if (PyInt_Check(value))
                opts.SetDouble(name, PyInt_AS_LONG(value));
            else
            {
                sprintf(msg, "Expected float to set '%s'", name.c_str());
                VisItErrorFunc(msg);
                return false;
            }
            break;
          case DBOptionsAttributes::String:
            if (PyString_Check(value))
                opts.SetString(name, PyString_AS_STRING(value));
            else
            {
                sprintf(msg, "Expected string to set '%s'", name.c_str());
                VisItErrorFunc(msg);
                return false;
            }
            break;
          case DBOptionsAttributes::Enum:
            if (PyInt_Check(value))
                opts.SetEnum(name, PyInt_AS_LONG(value));
            else
            {
                sprintf(msg, "Expected int to set enum '%s'", name.c_str());
                VisItErrorFunc(msg);
                return false;
            }
            break;
        }
    }

    return true;
}

// ****************************************************************************
//  Method:  CreateDictionaryFromDBOptions
//
//  Purpose:
//    Create a dictionary of key-value pairs ("boolval":1,"stringval":"foobar")
//    from a DBOptionsAttributes structure.
//
//  Arguments:
//    opts       the DBOptionsAttributes to convert
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 11, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 25 10:05:37 EDT 2007
//    Added support for pre-2.5 versions of Python.
//
//    Jeremy Meredith, Tue Apr 29 15:18:13 EDT 2008
//    Fixing ABW bug.
//
// ****************************************************************************
PyObject *
CreateDictionaryFromDBOptions(DBOptionsAttributes &opts)
{
    PyObject *dict = PyDict_New();
    for (int j=0; j<opts.GetNumberOfOptions(); j++)
    {
        // Older pythons don't support const char* in the PyDict routines,
        // so we have to copy this into a non-const string.
        char *name = new char[opts.GetName(j).length()+1];
        strcpy(name, opts.GetName(j).c_str());
        switch (opts.GetType(j))
        {
          case DBOptionsAttributes::Bool:
            PyDict_SetItemString(dict,name,PyInt_FromLong(opts.GetBool(name)));
            break;
          case DBOptionsAttributes::Int:
            PyDict_SetItemString(dict,name,PyInt_FromLong(opts.GetInt(name)));
            break;
          case DBOptionsAttributes::Float:
            PyDict_SetItemString(dict,name,PyFloat_FromDouble(opts.GetFloat(name)));
            break;
          case DBOptionsAttributes::Double:
            PyDict_SetItemString(dict,name,PyFloat_FromDouble(opts.GetDouble(name)));
            break;
          case DBOptionsAttributes::String:
            PyDict_SetItemString(dict,name,PyString_FromString(opts.GetString(name).c_str()));
            break;
          case DBOptionsAttributes::Enum:
            PyDict_SetItemString(dict,name,PyInt_FromLong(opts.GetEnum(name)));
            break;
        }
        delete[] name;
    }
    return dict;
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

PyObject *
visit_AddArgument(PyObject *self, PyObject *args)
{
    if(noViewer)
    {
        char *arg;
        if(!PyArg_ParseTuple(args, "s", &arg))
            return NULL;

        GetViewerProxy()->AddArgument(arg);
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
    NO_ARGUMENTS();
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
//   Brad Whitlock, Mon Jun 27 09:31:24 PDT 2005
//   Added code to handle client events that come up during launch after the
//   launch has happened. This prevents client methods from stalling the
//   launch process.
//
//   Brad Whitlock, Wed Nov 22 14:31:34 PST 2006
//   I added code to accept the name of the VisIt script to run.
//
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

PyObject *
visit_Launch(PyObject *self, PyObject *args)
{
    debug1 << "Launch: 0" << endl;

    //
    // Check to see if the viewer is already launched.
    //
    if(!noViewer)
    {
        VisItErrorFunc("VisIt's viewer is already launched!");
        return NULL;
    }

    //
    // Determine if the function was called with any arguments.
    //
    const char *visitProgram = 0;
    static const char *visitProgramDefault = "visit";
    if (!PyArg_ParseTuple(args, "s", &visitProgram))
    {
        visitProgram = visitProgramDefault;
        PyErr_Clear();
    }
    debug1 << "Launch: 1: " << visitProgram << endl;

    //
    // Launch the viewer.
    //
    LaunchViewer(visitProgram);

    debug1 << "Launch: 2" << endl;

    //
    // If the noViewer flag is false then the viewer could not launch.
    //
    if(noViewer)
    {
        VisItErrorFunc("VisIt's viewer could not launch!");
        return NULL;
    }

    debug1 << "Launch: 3" << endl;

    //
    // Wait for the viewer to tell us to load the plugins.
    //    
    DelayedLoadPlugins();

    debug1 << "Launch: 4" << endl;

    // Create the thread that listens for input from the viewer. Then
    // synchronize to flush out any initialization that came from the viewer
    // before we allow more commands to execute.
    CreateListenerThread();
    debug1 << "Launch: 5" << endl;
    int errorFlag = Synchronize();
    debug1 << "Launch: 6" << endl;

    //
    // Initialize the extensions.
    //
    InitializeExtensions();

    debug1 << "Launch: 7" << endl;

    //
    // Iterate over the plugins and add their methods to the VisIt module's
    // methods.
    //
    PlotPluginAddInterface();
    OperatorPluginAddInterface();

    debug1 << "Launch: 8" << endl;

    //
    // Execute any client methods that came in during the Synchronize.
    //
    debug1 << "Launch: 8, executing cached client methods." << endl;
    int size = 0;
    do
    {
        ClientMethod *m = 0;

        MUTEX_LOCK();
        if(cachedClientMethods.size() > 0)
        {
            m = cachedClientMethods[0];
            cachedClientMethods.erase(cachedClientMethods.begin());
        }
        size = cachedClientMethods.size();
        MUTEX_UNLOCK();

        if(m != 0)
        {
            ExecuteClientMethod(m, false);
            delete m;
        }
    }
    while(size > 0);

    clientMethodsAllowed = true;

    debug1 << "Launch, end" << endl;

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
        GetViewerProxy()->AddArgument("-nowin");
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

PyObject *
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

        GetViewerProxy()->AddArgument("-debug");
        char tmp[10];
        SNPRINTF(tmp, 10, "%d", moduleDebugLevel);
        GetViewerProxy()->AddArgument(tmp); 
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
    NO_ARGUMENTS();
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
    NO_ARGUMENTS();

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
visit_LocalNameSpace(PyObject *, PyObject *args)
{
    NO_ARGUMENTS();

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
    NO_ARGUMENTS();
    return PyString_FromString(GetViewerProxy()->GetLocalHostName().c_str());
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
    NO_ARGUMENTS();
    return PyString_FromString(GetViewerProxy()->GetLocalUserName().c_str());
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
    NO_ARGUMENTS();

    if(!noViewer)
    {
        CloseModule();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->AddWindow();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ShowAllWindows();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->CloneWindow();
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
    NO_ARGUMENTS();

    WindowInformation *wi = GetViewerState()->GetWindowInformation();
    DatabaseCorrelationList *correlations = GetViewerState()->GetDatabaseCorrelationList();

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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->TimeSliderNextState();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->TimeSliderPreviousState();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
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
    WindowInformation *wi = GetViewerState()->GetWindowInformation();
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
    if(GetViewerState()->GetKeyframeAttributes()->GetEnabled())
        nStates = GetViewerState()->GetKeyframeAttributes()->GetNFrames();
    else
    {
        DatabaseCorrelationList *correlations = GetViewerState()->GetDatabaseCorrelationList();
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
        GetViewerMethods()->SetTimeSliderState(state);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetTreatAllDBsAsTimeVarying
//
// Purpose: Tells the viewer to treat all databases as time varying 
//
// Programmer: Mark C. Miller
// Creation:   May 27, 2005 
//
// ****************************************************************************

STATIC PyObject *
visit_SetTreatAllDBsAsTimeVarying(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int flag;
    if (!PyArg_ParseTuple(args, "i", &flag))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->SetTreatAllDBsAsTimeVarying(flag);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: visit_SetTryHarderCyclesTimes
//
// Purpose: Tells the viewer to try harder to obtain accurate cycles/times
//
// Programmer: Mark C. Miller
// Creation:   May 27, 2005 
//
// ****************************************************************************

STATIC PyObject *
visit_SetTryHarderCyclesTimes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int flag;
    if (!PyArg_ParseTuple(args, "i", &flag))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->SetTryHarderCyclesTimes(flag);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetCreateMeshQualityExpressions
//
// Purpose: Tells the viewer to turn on/off automatic creation
//          of mesh quality expressions.
//
// Programmer: Kathleen Bonnell
// Creation:   October 9, 2007 
//
// ****************************************************************************

STATIC PyObject *
visit_SetCreateMeshQualityExpressions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int flag;
    if (!PyArg_ParseTuple(args, "i", &flag))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->SetCreateMeshQualityExpressions(flag);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetCreateTimeDerivativeExpressions
//
// Purpose: Tells the viewer to turn on/off automatic creation
//          of time derivative expressions.
//
// Programmer: Kathleen Bonnell
// Creation:   October 9, 2007 
//
// ****************************************************************************

STATIC PyObject *
visit_SetCreateTimeDerivativeExpressions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int flag;
    if (!PyArg_ParseTuple(args, "i", &flag))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->SetCreateTimeDerivativeExpressions(flag);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetCreateTimeDerivativeExpressions
//
// Purpose: Tells the viewer to turn on/off automatic creation of vector
//          magnitude expressions.
//
// Programmer: Cyrus Harrison
// Creation:   November 28, 2007
//
// ****************************************************************************

STATIC PyObject *
visit_SetCreateVectorMagnitudeExpressions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int flag;
    if (!PyArg_ParseTuple(args, "i", &flag))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->SetCreateVectorMagnitudeExpressions(flag);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
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
        GetViewerMethods()->SetActiveTimeSlider(tsName);
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
    NO_ARGUMENTS();

    const WindowInformation *wi = GetViewerState()->GetWindowInformation();
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
    NO_ARGUMENTS();

    const WindowInformation *wi = GetViewerState()->GetWindowInformation();
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
    NO_ARGUMENTS();

    //
    // Get the number of states for the active time slider.
    //
    WindowInformation *wi = GetViewerState()->GetWindowInformation();
    const std::string &ts = wi->GetTimeSliders()[wi->GetActiveTimeSlider()];
    int nStates = 1;
    if(GetViewerState()->GetKeyframeAttributes()->GetEnabled() && ts == "Keyframe animation")
        nStates = GetViewerState()->GetKeyframeAttributes()->GetNFrames();
    else
    {
        DatabaseCorrelationList *correlations = GetViewerState()->GetDatabaseCorrelationList();
        DatabaseCorrelation *c = correlations->FindCorrelation(ts);
        if(c != 0)
            nStates = c->GetNumStates();
    }

    // Return the success value.
    return PyLong_FromLong(long(nStates));
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
        GetViewerMethods()->AnimationSetNFrames(nFrames);
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
        GetViewerMethods()->SetWindowLayout(winLayout);
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
        GetViewerMethods()->SetActiveWindow(activewin);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->IconifyAllWindows();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->DeIconifyAllWindows();
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
//   Hank Childs, Thu Jun 30 14:30:39 PDT 2005
//   Load plugin info.  The best way to do this would be to have a blocking
//   RPC that can be called before exporting a database.  Since no facility
//   exists for that at this time, make sure we load the info well in advance
//   (ie in this function).
//
//   Jeremy Meredith, Wed Feb 14 16:10:50 EST 2007
//   Added support for specifying the plugin to use when opening files.
//   Also, made the argument parsing "better".
//
// ****************************************************************************

STATIC PyObject *
visit_OpenDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    int timeIndex = 0;
    char *format = NULL;
    if (!PyArg_ParseTuple(args, "s|is", &fileName, &timeIndex, &format))
        return NULL;

    // Open the database.
    MUTEX_LOCK();
        if (!format)
            GetViewerMethods()->OpenDatabase(fileName, timeIndex);
        else
            GetViewerMethods()->OpenDatabase(fileName, timeIndex, true, format);

        static bool loadedPluginInfo = false;
        if (!loadedPluginInfo)
        {
            GetViewerMethods()->UpdateDBPluginInfo("localhost");
            loadedPluginInfo = true;
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
        GetViewerMethods()->ReOpenDatabase(fileName);
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
//   Brad Whitlock, Thu Jul 24 09:35:32 PDT 2008
//   Added optional timeState argument.
//
// ****************************************************************************

STATIC PyObject *
visit_OverlayDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *fileName;
    int timeState = 0;
    if (!PyArg_ParseTuple(args, "s|i", &fileName, &timeState))
       return NULL;

    // Overlay the database.
    MUTEX_LOCK();
        GetViewerMethods()->OverlayDatabase(fileName, timeState);
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
        GetViewerMethods()->ReplaceDatabase(fileName, timeState);
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
        GetViewerMethods()->ActivateDatabase(fileName);
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
        GetViewerMethods()->CheckForNewStates(fileName);
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
        GetViewerMethods()->CloseDatabase(fileName);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetMetaData
//
// Purpose: 
//   Requests metadata for a specific file and returns a copy.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 18 14:02:38 PST 2004
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetMetaData(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int ts = -1;
    char *db = NULL;
    if(!PyArg_ParseTuple(args, "si", &db, &ts))
    {
        if(!PyArg_ParseTuple(args, "s", &db))
            return NULL;
        PyErr_Clear();
    }

    // Tell the viewer that we want the metadata for a specific file.
    MUTEX_LOCK();
        GetViewerMethods()->RequestMetaData(db, ts);
    MUTEX_UNLOCK();

    // Wait for the viewer to return the sync tag.
    Synchronize();

    // Copy the metadata.
    PyObject *mdObj = NULL;
    MUTEX_LOCK();
        mdObj = PyavtDatabaseMetaData_New();
        // Copy the metadata into the new object's metadata object.
        *PyavtDatabaseMetaData_FromPyObject(mdObj) = 
            *GetViewerState()->GetDatabaseMetaData();
    MUTEX_UNLOCK();

    // Return the new object.
    return mdObj;
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
        GetViewerMethods()->CreateDatabaseCorrelation(name, dbs, method);
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
        GetViewerMethods()->AlterDatabaseCorrelation(name, dbs, method);
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
        GetViewerMethods()->DeleteDatabaseCorrelation(name);
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
        DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
        cL->SetNeedPermission(false);
        cL->SetDefaultCorrelationMethod(defaultCorrelationMethod);
        cL->SetWhenToCorrelate((DatabaseCorrelationList::WhenToCorrelate)
            whenToCorrelate);
        cL->Notify();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetDatabaseCorrelation
//
// Purpose: 
//   Returns a read-only database correlation object that contains the
//   attributes for the specified database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 17 11:35:41 PDT 2005
//
// Modifications:
//   Brad Whitlock, Tue Mar 13 14:33:32 PST 2007
//   Made it use automatically generated code.
//
// ****************************************************************************

STATIC PyObject *
visit_GetDatabaseCorrelation(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    char *name = 0;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
    DatabaseCorrelation *C = cL->FindCorrelation(name);
    PyObject *retval = 0;
    if(C != 0)
    {
        // Create a new database correlation object.
        retval = PyDatabaseCorrelation_New();
        *PyDatabaseCorrelation_FromPyObject(retval) = *C;
    }
    else
        VisItErrorFunc("No such database correlation!");

    return retval;
}

// ****************************************************************************
// Function: visit_GetDatabaseCorrelationNames
//
// Purpose: 
//   Returns the names of all of the database correlations.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 17 11:35:41 PDT 2005
//
// Modifications:
//   Brad Whitlock, Tue Mar 13 11:01:38 PDT 2007
//   Updated due to code generation changes.
//
// ****************************************************************************

STATIC PyObject *
visit_GetDatabaseCorrelationNames(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    // Allocate a tuple the with enough entries to hold the database
    // correlation name list.
    DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();
    PyObject *retval = PyTuple_New(cL->GetNumCorrelations());

    for(int i = 0; i < cL->GetNumCorrelations(); ++i)
    {
        PyObject *name = PyString_FromString(
            cL->operator[](i).GetName().c_str());
        if(name == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, name);
    }

    return retval;
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
//    Brad Whitlock, Tue May 8 16:13:35 PST 2007
//    Added code to get the expression if it exists instead of always adding 
//    a new expression.
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

    // Access the expression list and add a new one, if necessary.
    MUTEX_LOCK();

        ExpressionList *list = GetViewerState()->GetExpressionList();
        // Get the existing expression if it exists or create a new one.
        Expression *e = list->operator[](exprName);
        bool expressionExists = e != 0;
        if(!expressionExists)
            e = new Expression();
        else
            debug4 << "Replacing definition for expression " << exprName << endl;

        // Set the expression properties.
        e->SetName(exprName);
        e->SetDefinition(exprDef);
        e->SetType(t);

        // Add the expression if it's not in the list.
        if(!expressionExists)
        {
            list->AddExpressions(*e);
            delete e;
        }

        // Send the new list to the viewer.
        list->Notify();
        GetViewerMethods()->ProcessExpressions();
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
//    Hank Childs, Thu Jun 30 10:57:47 PDT 2005
//    Added DefineTensorExpression
//
//    Hank Childs, Thu Jul 21 14:26:03 PDT 2005
//    Added DefineArrayExpression.
//
//    Kathleen Bonnell, Tue Aug  1 09:13:45 PDT 2006 
//    Added DefineCurveExpression.
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
visit_DefineTensorExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineTensorExpression", Expression::TensorMeshVar);
}

STATIC PyObject *
visit_DefineArrayExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineArrayExpression", Expression::ArrayMeshVar);
}

STATIC PyObject *
visit_DefineCurveExpression(PyObject *self, PyObject *args)
{
    return ExpressionDefinitionHelper(args, "DefineCurveExpression", Expression::CurveMeshVar);
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
//   Kathleen Bonnell, Wed Dec 22 12:50:20 PST 2004
//   Notify viewer that ExpressionList has been modified.
//   
//   Cyrus Harrison, Thu Nov 29 09:52:41 PST 2007
//   To maintain consistency with the GUI, made sure we guard against deletion
//   of database expressions and auto expressions. Also added descriptive error
//   messages if no expression is deleted. 
//
//   Jeremy Meredith, Tue Feb 19 14:37:10 EST 2008
//   Don't allow users to delete expressions created by an operator.
//
// ****************************************************************************

STATIC PyObject *
visit_DeleteExpression(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    bool found  = false;
    bool success =false;
    
    char *exprName;
    if (!PyArg_ParseTuple(args, "s", &exprName))
       return NULL;

    char buff[512];
    
    // Access the expression list and delete the proper expression
    MUTEX_LOCK();

        ExpressionList *list = GetViewerState()->GetExpressionList();
        for(int i=0;i<list->GetNumExpressions();i++)
        {
            const Expression &expr = list->GetExpressions(i);
            if( strcmp(expr.GetName().c_str(),exprName) == 0)
            {
                // make sure expr is not from db, or an auto expression
                if(expr.GetAutoExpression())
                {
                    SNPRINTF(buff,512,
                             "Cannot delete auto generated expression \"%s\".",
                             exprName);
                }
                else if(expr.GetFromDB())
                {
                    SNPRINTF(buff,512,
                             "Cannot delete database expression \"%s\".",
                             exprName);
                }
                else if(expr.GetFromOperator())
                {
                    SNPRINTF(buff,512,
                             "Cannot delete operator expression \"%s\".",
                             exprName);
                }
                else // delete the expression
                {
                    list->RemoveExpressions(i);
                    success = true;
                }
                found = true;
                break;
            }
        }

        if(success)
        {
            list->Notify();
            GetViewerMethods()->ProcessExpressions();
        }

    MUTEX_UNLOCK();
    
    if(!found)
        SNPRINTF(buff,512,"Cannot delete unknown expression \"%s\".",exprName);
         
    if(!success)
    {
        VisItErrorFunc(buff);
        return NULL;
    }
    
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
//   Hank Childs, Tue Feb 20 16:12:41 PST 2007
//   Improve the error message for the case where the user specifies the 
//   processor count as an integer, not a string.
//
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
OpenComponentHelper(PyObject *self, PyObject *args, bool openEngine)
{
    ENSURE_VIEWER_EXISTS();

    static const char *OCEError = "Arguments must be: hostName, arg | (args...)";
    static const char *OCEError2 = "Arguments must be: hostName, "
            "arg | (args...).\n\tNot all of the arguments were strings.\n"
            "\tDid you put in a number as an integer rather than as a string?\n"
            "\tE.g.: (\"-np\", \"2\"), not (\"-np\", 2)";

    const char  *hostName;
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
                    VisItErrorFunc(OCEError2);
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
        if(openEngine)
            GetViewerMethods()->OpenComputeEngine(hostName, argv);
        else
            GetViewerMethods()->OpenMDServer(hostName, argv);
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
// Function: OpenClientHelper
//
// Purpose: 
//   Helps us open a client.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 09:23:27 PDT 2005
//
// Modifications:
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//   
// ****************************************************************************

STATIC PyObject *
OpenClientHelper(PyObject *self, PyObject *args, int componentNumber)
{
    ENSURE_VIEWER_EXISTS();

    static const char *OCEError = "Arguments must be: clientName, program, arg | (args...)";
    const char  *clientName = 0;
    const char  *program = 0;
    const char  *arg1 = 0;
    stringVector argv;

    if(componentNumber == 1)
    {
        clientName = "GUI";
        program = "visit";
        argv.push_back("-gui");

        // Make sure it's a tuple.
        if(!GetStringVectorFromPyObject(args, argv))
        {
            VisItErrorFunc(OCEError);
            return NULL;
        }

        PyErr_Clear();
    }
    else if (!PyArg_ParseTuple(args, "ss", &clientName, &program))
    {
        if (!PyArg_ParseTuple(args, "sss", &clientName, &program, &arg1))
        {
            PyObject *tuple;
            if (PyArg_ParseTuple(args, "ssO", &clientName, &program, &tuple))
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
        // Open a client
        GetViewerMethods()->OpenClient(clientName, program, argv);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

STATIC PyObject *
visit_OpenClient(PyObject *self, PyObject *args)
{
    return OpenClientHelper(self, args, 0);
}

STATIC PyObject *
visit_OpenGUI(PyObject *self, PyObject *args)
{
    return OpenClientHelper(self, args, 1);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->InvertBackgroundColor();
        GetViewerMethods()->RedrawWindow();
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
//   Brad Whitlock, Wed Jan 23 09:27:52 PDT 2008
//   I made adding a plot set the new "plots inherit sil restriction"
//   setting to true if it is not provided since the viewer now does not apply
//   default SIL selections unless that flag is set. This change here ensures
//   script compatibility.
//
//   Gunther H. Weber, Tue Apr  1 16:40:25 PDT 2008
//   Added option to determine whether operators of existing plots will be
//   applied to the new plot. By default, do not apply operators of existing
//   plots to the new plot, even if the global option applyOperator is set.
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//
// ****************************************************************************

STATIC PyObject *
visit_AddPlot(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *plotName = 0;
    char *varName = 0;
    int inherit = 1;
    int applyExistingOperators = 0;
    if (!PyArg_ParseTuple(args, "ss", &plotName, &varName))
    {
        if(!PyArg_ParseTuple(args, "ssi", &plotName, &varName, &inherit))
        {
            if(!PyArg_ParseTuple(args, "ssii", &plotName, &varName, &inherit, &applyExistingOperators))
                return NULL;
            else PyErr_Clear();
        }
        else
            PyErr_Clear();    
    }

    // Find the plot index from the name. Throw a python exception if we are
    // allowing python exceptions and the plot index is -1.
    PlotPluginManager *pluginManager = GetViewerProxy()->GetPlotPluginManager();
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
        // Set the apply to all plots toggle.
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyExistingOperators != 0);

        bool inheritSILRestriction = inherit != 0;
        bool value = GetViewerState()->GetGlobalAttributes()->GetNewPlotsInheritSILRestriction();
        if(inheritSILRestriction != value)
        {
            GetViewerState()->GetGlobalAttributes()->SetNewPlotsInheritSILRestriction(inheritSILRestriction);
        }
        GetViewerState()->GetGlobalAttributes()->Notify();

        // Add the plot
        GetViewerMethods()->AddPlot(plotTypeIndex, varName);

        // Restore the old value
        if(inheritSILRestriction != value)
        {
            GetViewerState()->GetGlobalAttributes()->SetNewPlotsInheritSILRestriction(value);
        }

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
//   I made it return a success value.
//
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    OperatorPluginManager *pluginManager = GetViewerProxy()->GetOperatorPluginManager();
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
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        GetViewerState()->GetGlobalAttributes()->Notify();

        // Add the operator
        GetViewerMethods()->AddOperator(operTypeIndex);

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->DrawPlots();
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
             const stringVector &engines = GetViewerState()->GetEngineList()->GetEngines();
             const stringVector &sims = GetViewerState()->GetEngineList()->GetSimulationName();
             if(engines.size() > 0)
             {
                 engineName = engines[0].c_str();
                 simulationName = sims[0].c_str();
             }
         }
         else if (useFirstSimulation)
         {
             const stringVector &engines = GetViewerState()->GetEngineList()->GetEngines();
             const stringVector &sims = GetViewerState()->GetEngineList()->GetSimulationName();
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
             GetViewerMethods()->CloseComputeEngine(engineName, simulationName);
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
 
    GetViewerMethods()->SetPlotFrameRange(plotId, frame0, frame1);
 
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
 
    GetViewerMethods()->DeletePlotKeyframe(plotId, frame);
 
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
 
    GetViewerMethods()->MovePlotKeyframe(plotId, oldFrame, newFrame);
 
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
 
    GetViewerMethods()->SetPlotDatabaseState(plotId, frame, state);
 
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
 
    GetViewerMethods()->DeletePlotDatabaseKeyframe(plotId, frame);
 
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
 
    GetViewerMethods()->MovePlotDatabaseKeyframe(plotId, oldFrame, newFrame);
 
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
         GetViewerMethods()->ChangeActivePlotsVar(varName);
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
         GetViewerMethods()->CopyAnnotationsToWindow(from, to);
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
        GetViewerMethods()->CopyLightingToWindow(from, to);
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
        GetViewerMethods()->CopyViewToWindow(from, to);
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
        GetViewerMethods()->CopyPlotsToWindow(from, to);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->DeleteActivePlots();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        //
        // First set all plots active.
        //
        int nPlots = GetViewerState()->GetPlotList()->GetNumPlots();
        intVector plots;
        for(int i = 0; i < nPlots; ++i)
            plots.push_back(i);
        GetViewerMethods()->SetActivePlots(plots);

        GetViewerMethods()->DeleteActivePlots();
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
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
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
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        GetViewerState()->GetGlobalAttributes()->Notify();

        // Remove the last operator.
        GetViewerMethods()->RemoveLastOperator();

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
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
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        GetViewerState()->GetGlobalAttributes()->Notify();

        // Remove all operators.
        GetViewerMethods()->RemoveAllOperators();

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ClearWindow();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ClearAllWindows();
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
//    Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//    Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
visit_ClearCache(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const char *engineName = 0;
    const char *simulationName = 0;
    if(!PyArg_ParseTuple(args, "ss", &engineName, &simulationName))
    {
        if (!PyArg_ParseTuple(args, "s", &engineName))
            return NULL;
        simulationName = "";
        PyErr_Clear();
    }

    MUTEX_LOCK();
        GetViewerMethods()->ClearCache(engineName, simulationName);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ClearCacheForAllEngines();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ClearPickPoints();
        GetViewerMethods()->RedrawWindow();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ClearReferenceLines();
        GetViewerMethods()->RedrawWindow();
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
//    Mark C. Miller, Tue Mar  7 10:31:34 PST 2006
//    Made it return NULL only when it doesen't know SaveWindow exited
//    out the bottom.
// ****************************************************************************

STATIC PyObject *
visit_SaveWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->SaveWindow();
    MUTEX_UNLOCK();
    int errorFlag = Synchronize();

    std::string realname = GetViewerState()->GetSaveWindowAttributes()->GetLastRealFilename();
    PyObject *retval = NULL;

    if (errorFlag == 0)
    {
        retval = PyString_FromString(realname.c_str());
    }
    else
    {
        // test for bugus name indicating save failed, somehow
        if (realname == "/dev/null/SaveWindow_Error.txt")
            retval = PyString_FromString("");
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->DeleteWindow();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->DisableRedraw();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->RedrawWindow();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_MoveAndResizeWindow
//
// Purpose: 
//   Moves a window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 17:00:00 PST 2005
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ResizeWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int win, i0, i1;
    if(!PyArg_ParseTuple(args, "iii", &win, &i0, &i1))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->ResizeWindow(win, i0, i1);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_MoveWindow
//
// Purpose: 
//   Moves a window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 17:00:00 PST 2005
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_MoveWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int win, i0, i1;
    if(!PyArg_ParseTuple(args, "iii", &win, &i0, &i1))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->MoveWindow(win, i0, i1);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_MoveAndResizeWindow
//
// Purpose: 
//   Moves and resizes a window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 17:00:00 PST 2005
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_MoveAndResizeWindow(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int win, i0, i1, i2, i3;
    if(!PyArg_ParseTuple(args, "iiiii", &win, &i0, &i1, &i2, &i3))
        return NULL;

    MUTEX_LOCK();
        GetViewerMethods()->MoveAndResizeWindow(win, i0, i1, i2, i3);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->RecenterView();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ResetView();
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
        GetViewerMethods()->SetCenterOfRotation(c0, c1, c2);
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
            GetViewerMethods()->ChooseCenterOfRotation(sx, sy);
        }
        else
        {
            // Choose the point interactively
            GetViewerMethods()->ChooseCenterOfRotation();
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
        GetViewerMethods()->ImportEntireState(filename, sessionStoredInVisItDir!=0);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_RestoreSessionWithDifferentSources
//
// Purpose:
//   Tells the viewer to read in a session file but use a different list of
//   databases when restoring it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 10 11:16:04 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RestoreSessionWithDifferentSources(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *filename;    
    int sessionStoredInVisItDir = 1;
    PyObject *tuple = 0;
    if(!PyArg_ParseTuple(args, "siO", &filename, &sessionStoredInVisItDir,
        &tuple))
        return NULL;

    // Make sure it's a tuple.
    stringVector dbs;
    if(!GetStringVectorFromPyObject(tuple, dbs))
    {
        VisItErrorFunc("Arguments must be sessionFile, int, "
                       "tuple of database names");
        return NULL;
    }

    MUTEX_LOCK();
        GetViewerMethods()->ImportEntireStateWithDifferentSources(filename,
            sessionStoredInVisItDir!=0, dbs);
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
//    Cyrus Harrison, Wed Sep 12 15:22:13 PDT 2007
//    Automatically append ".session" to the session file name if not 
//    included by the user. 
//
// ****************************************************************************

STATIC PyObject *
visit_SaveSession(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
   
    char *filename;    
    if(!PyArg_ParseTuple(args, "s", &filename))
        return NULL;

    std::string session_file(filename);
    // check for ".session" extension
    int rpos = session_file.rfind(".session");
    if( rpos  == std::string::npos || rpos != session_file.size() - 8)
        session_file += ".session";
        
    MUTEX_LOCK();
        GetViewerMethods()->ExportEntireState(session_file.c_str());
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
    NO_ARGUMENTS();

    // Allocate a tuple the with enough entries to hold the engine list.
    const stringVector &engines = GetViewerState()->GetEngineList()->GetEngines();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->HideActivePlots();
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
        GetViewerMethods()->HideToolbars(hideForAllWindows != 0);
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
        GetViewerMethods()->ShowToolbars(showForAllWindows != 0);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetAnimationAttributes
//
// Purpose:
//   Tells the viewer to use the new animation attributes we're sending.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 12 15:16:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetAnimationAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *anim = NULL;
    // Try and get the Animation pointer.
    if(!PyArg_ParseTuple(args,"O",&anim))
    {
        VisItErrorFunc("SetAnimationAttributes: Canim parse object!");
        return NULL;
    }
    if(!PyAnimationAttributes_Check(anim))
    {
        VisItErrorFunc("Argument is not a AnimationAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        AnimationAttributes *va = PyAnimationAttributes_FromPyObject(anim);

        // Copy the object into the view attributes.
        *(GetViewerState()->GetAnimationAttributes()) = *va;
        GetViewerState()->GetAnimationAttributes()->Notify();
        GetViewerMethods()->SetAnimationAttributes();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetAnimationAttributes
//
// Purpose:
//   Returns the Animation attributes for the active window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 12 15:16:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetAnimationAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    PyObject *retval = PyAnimationAttributes_New();
    AnimationAttributes *aa = PyAnimationAttributes_FromPyObject(retval);

    // Copy the viewer proxy's Animation atts into the return data structure.
    *aa = *(GetViewerState()->GetAnimationAttributes());

    return retval;
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
        *(GetViewerState()->GetAnnotationAttributes()) = *va;
        GetViewerState()->GetAnnotationAttributes()->Notify();
        GetViewerMethods()->SetAnnotationAttributes();
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
        GetViewerState()->GetGlobalAttributes()->SetCloneWindowOnFirstRef(flag);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
        *(GetViewerState()->GetAnnotationAttributes()) = *va;
        GetViewerState()->GetAnnotationAttributes()->Notify();
        GetViewerMethods()->SetDefaultAnnotationAttributes();
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
    NO_ARGUMENTS();

    PyObject *retval = PyAnnotationAttributes_New();
    AnnotationAttributes *aa = PyAnnotationAttributes_FromPyObject(retval);

    // Copy the viewer proxy's annotation atts into the return data structure.
    *aa = *(GetViewerState()->GetAnnotationAttributes());

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
        *(GetViewerState()->GetKeyframeAttributes()) = *va;
        GetViewerState()->GetKeyframeAttributes()->Notify();
        GetViewerMethods()->SetKeyframeAttributes();
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
    NO_ARGUMENTS();

    PyObject *retval = PyKeyframeAttributes_New();
    KeyframeAttributes *aa = PyKeyframeAttributes_FromPyObject(retval);
 
    // Copy the viewer proxy's keyframe atts into the return data structure.
    *aa = *(GetViewerState()->GetKeyframeAttributes());
 
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
        *(GetViewerState()->GetMaterialAttributes()) = *va;
        GetViewerState()->GetMaterialAttributes()->Notify();
        GetViewerMethods()->SetMaterialAttributes();
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
        *(GetViewerState()->GetMaterialAttributes()) = *va;
        GetViewerState()->GetMaterialAttributes()->Notify();
        GetViewerMethods()->SetDefaultMaterialAttributes();
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

    NO_ARGUMENTS();

    PyObject *retval = PyMaterialAttributes_New();
    MaterialAttributes *aa = PyMaterialAttributes_FromPyObject(retval);

    // Copy the viewer proxy's material atts into the return data structure.
    *aa = *(GetViewerState()->GetMaterialAttributes());

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
        *(GetViewerState()->GetPrinterAttributes()) = *va;
        GetViewerState()->GetPrinterAttributes()->Notify();
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
        *(GetViewerState()->GetSaveWindowAttributes()) = *va;
        GetViewerState()->GetSaveWindowAttributes()->Notify();
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
    NO_ARGUMENTS();

    PyObject *retval = PySaveWindowAttributes_New();
    SaveWindowAttributes *aa = PySaveWindowAttributes_FromPyObject(retval);

    // Copy the viewer proxy's saveimage atts into the return data structure.
    *aa = *(GetViewerState()->GetSaveWindowAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_ExportDatabase
//
// Purpose:
//   Tells the viewer to export the database.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   June 30, 2005
//
// Modifications:
//   Jeremy Meredith, Thu Oct 11 14:54:13 EDT 2007
//   I added an optional third argument containing a dictionary of key-value
//   pairs for passing database export options to the file format writer.
//   Also, enhancements to attribute groups allowed separate vectors for the
//   read and write options, so I switched to this simpler organization.
//
//   Jeremy Meredith, Tue Apr 29 15:24:51 EDT 2008
//   Added better error message for when plugin wasn't found.
//
// ****************************************************************************

STATIC PyObject *
visit_ExportDatabase(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *exportatts = NULL;
    PyObject *optsdict = NULL;
    // Try and get the export attributes and database options.
    if(!PyArg_ParseTuple(args,"O|O",&exportatts,&optsdict))
    {
        VisItErrorFunc("ExportDatabase: Cannot parse object!");
        return NULL;
    }
    if(!PyExportDBAttributes_Check(exportatts))
    {
        VisItErrorFunc("Argument is not a ExportDBAttributes object");
        return NULL;
    }

    ExportDBAttributes *va = PyExportDBAttributes_FromPyObject(exportatts);
    const std::string &db_type = va->GetDb_type();

    MUTEX_LOCK();
        DBPluginInfoAttributes *dbplugininfo = 
                        GetViewerState()->GetDBPluginInfoAttributes();
    MUTEX_UNLOCK();

    const stringVector &types = dbplugininfo->GetTypes();
    bool foundMatch = false;
    bool hasWriter = false;
    for (int i = 0 ; i < types.size() ; i++)
    {
        if (types[i] == db_type)
        {
            foundMatch = true;
            va->SetDb_type_fullname(dbplugininfo->GetTypesFullNames()[i]);
            DBOptionsAttributes *opts = (DBOptionsAttributes *)
                                         dbplugininfo->GetDbWriteOptions()[i];
            if (optsdict)
            {
                DBOptionsAttributes newopts(*opts);
                bool ok = FillDBOptionsFromDictionary(optsdict, newopts);
                if (!ok)
                    return NULL;
                *opts = newopts;
            }
            va->SetOpts(*opts);
            if (dbplugininfo->GetHasWriter()[i] != 0)
                hasWriter = true;
            break;
        }
    }

    if (!foundMatch)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is not a valid plugin type.  Make sure the "
                "Metadata Server is running.", db_type.c_str());
        VisItErrorFunc(msg);
        return NULL;
    }
    if (!hasWriter)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is a valid plugin type.  But it does *not* have\n"
                     "a database writer, so the database cannot be exported",
                db_type.c_str());
        VisItErrorFunc(msg);
        return NULL;
    }

    MUTEX_LOCK();
        // Copy the object into the view attributes.
        *(GetViewerState()->GetExportDBAttributes()) = *va;
        GetViewerState()->GetExportDBAttributes()->Notify();
        GetViewerMethods()->ExportDatabase();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetExportOptions
//
// Purpose:
//   Gets the write options for a DB plugin as a Dictionary.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   October 11, 2007
//
//  Modifications:
//    Jeremy Meredith, Tue Apr 29 15:24:51 EDT 2008
//    Added better error message for when plugin wasn't found.
//
// ****************************************************************************
STATIC PyObject *
visit_GetExportOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *plugin = NULL;
    // Try and get the export attributes and database options.
    if(!PyArg_ParseTuple(args,"s",&plugin))
        return NULL;

    MUTEX_LOCK();
        DBPluginInfoAttributes *dbplugininfo = 
                        GetViewerState()->GetDBPluginInfoAttributes();
    MUTEX_UNLOCK();

    PyObject *dict = NULL;
    const stringVector &types = dbplugininfo->GetTypes();
    bool foundMatch = false;
    bool hasWriter = false;
    for (int i = 0 ; i < types.size() ; i++)
    {
        if (types[i] == plugin)
        {
            foundMatch = true;
            if (dbplugininfo->GetHasWriter()[i] == 0)
                break;
            hasWriter = true;
            DBOptionsAttributes *opts = (DBOptionsAttributes *)
                                         dbplugininfo->GetDbWriteOptions()[i];
            if (opts)
            {
                dict = CreateDictionaryFromDBOptions(*opts);
            }
            break;
        }
    }

    if (!foundMatch)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is not a valid plugin type.  Make sure the "
                "Metadata Server is running.", plugin);
        VisItErrorFunc(msg);
        return NULL;
    }
    if (!hasWriter)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is a valid plugin type.  But it does *not* have\n"
                "a database writer", plugin);
        VisItErrorFunc(msg);
        return NULL;
    }
    if (!dict)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is a valid plugin with export capability, but "
                "appears to have no options.", plugin);
        VisItErrorFunc(msg);
        return NULL;
    }

    return dict;
}

// ****************************************************************************
//  Method:  visit_GetDefaultFileOpenOptions
//
//  Purpose:
//    Gets the options for a single plugin from the global default
//    file open options, and returns them as a dictionary.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Apr 29 15:24:51 EDT 2008
//    Added better error message for when plugin wasn't found.
//
// ****************************************************************************
STATIC PyObject *
visit_GetDefaultFileOpenOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *plugin = NULL;
    // Try and get the export attributes and database options.
    if(!PyArg_ParseTuple(args,"s",&plugin))
        return NULL;

    MUTEX_LOCK();
    FileOpenOptions *foo = GetViewerState()->GetFileOpenOptions();
    MUTEX_UNLOCK();

    PyObject *dict = NULL;
    const stringVector &types = foo->GetTypeNames();
    bool foundMatch = false;
    for (int i = 0 ; i < types.size() ; i++)
    {
        if (types[i] == plugin)
        {
            foundMatch = true;
            DBOptionsAttributes *opts = (DBOptionsAttributes*)
                                        foo->GetOpenOptions()[i];
            if (opts)
            {
                dict = CreateDictionaryFromDBOptions(*opts);
            }
            break;
        }
    }

    if (!foundMatch)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is not a valid plugin type.  Make sure the "
                "Metadata Server is running.", plugin);
        VisItErrorFunc(msg);
        return NULL;
    }
    if (!dict)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is a valid plugin, but appear to have "
                "have no options for opening files.", plugin);
        VisItErrorFunc(msg);
        return NULL;
    }

    return dict;
}

// ****************************************************************************
//  Method:  visit_GetDefaultFileOpenOptions
//
//  Purpose:
//    Takes the name of a plugin and a set of options for it as a dictionary,
//    and sets those values in the global default file open options.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Apr 29 15:24:51 EDT 2008
//    Added better error message for when plugin wasn't found.
//
// ****************************************************************************
STATIC PyObject *
visit_SetDefaultFileOpenOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *plugin = NULL;
    PyObject *optsdict = NULL;
    // Try and get the export attributes and database options.
    if(!PyArg_ParseTuple(args,"sO",&plugin,&optsdict))
        return NULL;

    MUTEX_LOCK();
    FileOpenOptions *foo = GetViewerState()->GetFileOpenOptions();
    MUTEX_UNLOCK();

    const stringVector &types = foo->GetTypeNames();
    bool foundMatch = false;
    for (int i = 0 ; i < types.size() ; i++)
    {
        if (types[i] == plugin)
        {
            foundMatch = true;
            DBOptionsAttributes *opts = (DBOptionsAttributes*)
                                        foo->GetOpenOptions()[i];
            if (optsdict)
            {
                DBOptionsAttributes newopts(*opts);
                bool ok = FillDBOptionsFromDictionary(optsdict, newopts);
                if (!ok)
                    return NULL;
                *opts = newopts;
                MUTEX_LOCK();
                GetViewerState()->GetFileOpenOptions()->Notify();
                GetViewerMethods()->SetDefaultFileOpenOptions();
                MUTEX_UNLOCK();
            }
            break;
        }
    }

    if (!foundMatch)
    {
        char msg[1024];
        sprintf(msg, "\"%s\" is not a valid plugin type.  Make sure the "
                "Metadata Server is running.", plugin);
        VisItErrorFunc(msg);
        return NULL;
    }

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ConstructDDF
//
// Purpose:
//     Tells the viewer to construct a DDF.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   Mon Feb 13 21:18:22 PST 2006
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ConstructDDF(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *ddf_info = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&ddf_info))
    {
        VisItErrorFunc("ConstructDDF: Cannot parse object!");
        return NULL;
    }
    if(!PyConstructDDFAttributes_Check(ddf_info))
    {
        VisItErrorFunc("Argument is not a ConstructDDFAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        ConstructDDFAttributes *va = PyConstructDDFAttributes_FromPyObject(ddf_info);

        // Copy the object into the constructDDF attributes.
        *(GetViewerState()->GetConstructDDFAttributes()) = *va;
        GetViewerState()->GetConstructDDFAttributes()->Notify();
        GetViewerMethods()->ConstructDDF();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetViewAxisArray
//
// Purpose:
//   Tells the viewer to use the new axis array view we're giving it.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2008
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetViewAxisArray(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *view = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&view))
    {
        VisItErrorFunc("SetViewAxisArray: Cannot parse object!");
        return NULL;
    }
    if(!PyViewAxisArrayAttributes_Check(view))
    {
        VisItErrorFunc("Argument is not a ViewAxisArrayAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        ViewAxisArrayAttributes *va =
            PyViewAxisArrayAttributes_FromPyObject(view);

        // Copy the object into the view attributes.
        *(GetViewerState()->GetViewAxisArrayAttributes()) = *va;
        GetViewerState()->GetViewAxisArrayAttributes()->Notify();
        GetViewerMethods()->SetViewAxisArray();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetViewAxisArray
//
// Purpose:
//   Returns the axis array view.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2008
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetViewAxisArray(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    PyObject *retval = PyViewAxisArrayAttributes_New();
    ViewAxisArrayAttributes *va = PyViewAxisArrayAttributes_FromPyObject(retval);

    // Copy the viewer proxy's axis array view into the return data structure.
    *va = *(GetViewerState()->GetViewAxisArrayAttributes());

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
        *(GetViewerState()->GetViewCurveAttributes()) = *va;
        GetViewerState()->GetViewCurveAttributes()->Notify();
        GetViewerMethods()->SetViewCurve();
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
    NO_ARGUMENTS();

    PyObject *retval = PyViewCurveAttributes_New();
    ViewCurveAttributes *va = PyViewCurveAttributes_FromPyObject(retval);

    // Copy the viewer proxy's curve view into the return data structure.
    *va = *(GetViewerState()->GetViewCurveAttributes());

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
            *(GetViewerState()->GetView2DAttributes()) = *va;
            GetViewerState()->GetView2DAttributes()->Notify();
            GetViewerMethods()->SetView2D();
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
            View2DAttributes *v2da = GetViewerState()->GetView2DAttributes();

            // Copy the object into the view attributes.
            v2da->SetWindowCoords(va->GetWindowCoords());
            v2da->SetViewportCoords(va->GetViewportCoords());
            v2da->Notify();
            GetViewerMethods()->SetView2D();

            cerr << "Warning: Passing a ViewAttribute to SetView2D is"
                 << " deprecated.  Pass a" << endl
                 << " View2DAttribute instead." << endl;
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
    NO_ARGUMENTS();

    PyObject *retval = PyView2DAttributes_New();
    View2DAttributes *va = PyView2DAttributes_FromPyObject(retval);

    // Copy the viewer proxy's 2d view into the return data structure.
    *va = *(GetViewerState()->GetView2DAttributes());

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
            *(GetViewerState()->GetView3DAttributes()) = *va;
            GetViewerState()->GetView3DAttributes()->Notify();
            GetViewerMethods()->SetView3D();
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
            View3DAttributes *v3da = GetViewerState()->GetView3DAttributes();

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
            GetViewerMethods()->SetView3D();

            cerr << "Warning: Passing a ViewAttribute to SetView3D is"
                 << " deprecated.  Pass a" << endl
                 << " View3DAttribute instead." << endl;
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
    NO_ARGUMENTS();

    PyObject *retval = PyView3DAttributes_New();
    View3DAttributes *va = PyView3DAttributes_FromPyObject(retval);

    // Copy the viewer proxy's 3d view into the return data structure.
    *va = *(GetViewerState()->GetView3DAttributes());

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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ClearViewKeyframes();
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
        GetViewerMethods()->DeleteViewKeyframe(frame);
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
        GetViewerMethods()->MoveViewKeyframe(oldFrame, newFrame);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->SetViewKeyframe();
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
        GetViewerMethods()->SetViewExtentsType(extType);
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
    NO_ARGUMENTS();

    PyObject *retval = PyGlobalAttributes_New();
    GlobalAttributes *ga = PyGlobalAttributes_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *ga = *(GetViewerState()->GetGlobalAttributes());

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
    NO_ARGUMENTS();

    PyObject *retval = PyWindowInformation_New();
    WindowInformation *wi = PyWindowInformation_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *wi = *(GetViewerState()->GetWindowInformation());

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
    NO_ARGUMENTS();

    PyObject *retval = PyRenderingAttributes_New();
    RenderingAttributes *ra = PyRenderingAttributes_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *ra = *(GetViewerState()->GetRenderingAttributes());

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
        *(GetViewerState()->GetRenderingAttributes()) = *ra;
        GetViewerState()->GetRenderingAttributes()->Notify();
        GetViewerMethods()->SetRenderingAttributes();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetColorTexturingEnabled
//
// Purpose: 
//   This function sets the color texturing mode in the rendering atts.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 11:36:24 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SetColorTexturingEnabled(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int value = 0;
    if (!PyArg_ParseTuple(args, "i", &value))
        return NULL;

    MUTEX_LOCK();
        RenderingAttributes *ra = GetViewerState()->GetRenderingAttributes();
        ra->SetColorTexturingFlag(value != 0);
        ra->Notify();
        GetViewerMethods()->SetRenderingAttributes();
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
//   Gunther H. Weber, Wed Mar 19 18:52:36 PDT 2008
//   Added spreadsheet pick 
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
        else if (strcmp(cMode, "spreadsheet pick") == 0)
            mode = 5;
        else
            mode = 0;

        PyErr_Clear();
    }

    MUTEX_LOCK();
        GetViewerMethods()->SetWindowMode(mode);
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
        GetViewerMethods()->EnableTool(tool, enabled==1);
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
//   Mark C. Miller, Tue May 10 19:53:08 PDT 2005
//   Made it return its results as a python string that is easily parsable
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//
// ****************************************************************************

STATIC PyObject *
visit_ListPlots(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int stringOnly = 0;
    if (!PyArg_ParseTuple(args, "i", &stringOnly))
        stringOnly = 0;

    MUTEX_LOCK();
    //
    // Print out the plot list.
    //
    PlotList *pl = GetViewerState()->GetPlotList();
    PlotPluginManager *plugins = GetViewerProxy()->GetPlotPluginManager();
    std::string outStr;

    for(int i = 0; i < pl->GetNumPlots(); ++i)
    {
        const Plot &plot = pl->operator[](i);
        std::string id(plugins->GetEnabledID(int(plot.GetPlotType())));
        CommonPlotPluginInfo *info = plugins->GetCommonPluginInfo(id);

        if(info != 0)
        {
             char tmpStr[2048];
             int  strLen = 0;

             int j;
             SNPRINTF(tmpStr, sizeof(tmpStr),
                 "Plot[%d]|id=%d;type=\"%s\";database=\"%s\";var=%s;active=%d;"
                 "hidden=%d;framerange=(%d, %d);keyframes={", i,
                    plot.GetId(),
                    info->GetName(),
                    plot.GetDatabaseName().c_str(),
                    plot.GetPlotVar().c_str(),
                    plot.GetActiveFlag(),
                    plot.GetHiddenFlag(),
                    plot.GetBeginFrame(),
                    plot.GetEndFrame());
             strLen = strlen(tmpStr);
 
             // Print out the keyframes.
             const std::vector<int> &keyframes = plot.GetKeyframes();
             for(j = 0; j < keyframes.size(); ++j)
             {
                 SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, "%d", keyframes[j]);
                 strLen = strlen(tmpStr);
                 if(j < keyframes.size() - 1)
                     SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, ", ");
                 strLen = strlen(tmpStr);
             }
             SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, "};database keyframes={");
             strLen = strlen(tmpStr);

             // Print out the database keyframes.
             const std::vector<int> &databaseKeyframes =
                 plot.GetDatabaseKeyframes();
             for(j = 0; j < databaseKeyframes.size(); ++j)
             {
                 SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, "%d", databaseKeyframes[j]);
                 strLen = strlen(tmpStr);
                 if(j < databaseKeyframes.size() - 1)
                     SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, ", ");
                 strLen = strlen(tmpStr);
             }
             SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, "};operators={");
             strLen = strlen(tmpStr);

             // Print out the plot operators.
             for(j = 0; j < plot.GetNumOperators(); ++j)
             {
                 int op = plot.GetOperator(j);
                 SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, "\"%s\"",
                     GetViewerProxy()->GetOperatorPluginManager()->GetEnabledID(op).c_str());
                 strLen = strlen(tmpStr);
                 if(j < plot.GetNumOperators() - 1)
                     SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen, ", ");
                 strLen = strlen(tmpStr);
             }
             SNPRINTF(&tmpStr[strLen], sizeof(tmpStr)-strLen,
                 "};activeOperator=%d", plot.GetActiveOperator());
             strLen = strlen(tmpStr);

             if (stringOnly != 1)
                 printf("%s\n", tmpStr);

             outStr += std::string(tmpStr);
             outStr += "#";
        }
    }

    // Write the command to the file.
    MUTEX_UNLOCK();

    return PyString_FromString(outStr.c_str());
}

// ****************************************************************************
// Function: visit_GetPlotList
//
// Purpose: 
//   This is a Python callback that gets the PlotList object
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 13 11:12:15 PDT 2007
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_GetPlotList(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *ret = 0;
    MUTEX_LOCK();
        ret = PyPlotList_New();
        // Copy the plot list into the new object's PlotList object.
        *PyPlotList_FromPyObject(ret) = 
            *GetViewerState()->GetPlotList();

    MUTEX_UNLOCK();

    return ret;
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
    ExpressionList *list = GetViewerState()->GetExpressionList();

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
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    OperatorPluginManager *pluginManager = GetViewerProxy()->GetOperatorPluginManager();
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
            bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
            GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
            GetViewerState()->GetGlobalAttributes()->Notify();

            // Reset the operator options.
            GetViewerMethods()->ResetOperatorOptions(operatorTypeIndex);

            // Restore toggle
            GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
            GetViewerState()->GetGlobalAttributes()->Notify();
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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    PlotPluginManager *pluginManager = GetViewerProxy()->GetPlotPluginManager();
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
            GetViewerMethods()->ResetPlotOptions(plotTypeIndex);
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
        if(vec[j] < 0 || vec[j] >= GetViewerState()->GetPlotList()->GetNumPlots())
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
        GetViewerMethods()->SetActivePlots(activePlots);
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
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    OperatorPluginManager *pluginManager = GetViewerProxy()->GetOperatorPluginManager();
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
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        GetViewerState()->GetGlobalAttributes()->Notify();
 
        // If the active operator was set, change the plot selection so we can set
        // the active operator.
        intVector selectedPlots, newActiveOperators, newExpandedPlots;
        intVector oldActiveOperators, oldExpandedPlots;
        if(activeOperator != -1)
        {
            PlotList *plotList = GetViewerState()->GetPlotList();
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
                GetViewerMethods()->SetActivePlots(selectedPlots, newActiveOperators,
                                       newExpandedPlots);
            }
        }

        // Set the operator attributes.
        AttributeSubject *operAtts = GetViewerState()->GetOperatorAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonOperatorPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        general->CopyAttributes(operAtts, ((AttributesObject *)obj)->data);
        operAtts->Notify();
        GetViewerMethods()->SetOperatorOptions(objPluginIndex);

        // Restore the plot selection.
        if(activeOperator != -1 && selectedPlots.size() > 0)
        {
            GetViewerMethods()->SetActivePlots(selectedPlots, oldActiveOperators, oldExpandedPlots);
        }

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
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
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAllPlots != 0);
        GetViewerState()->GetGlobalAttributes()->Notify();

        // Do the operation.
        if(option == 0)
            GetViewerMethods()->PromoteOperator(operatorIndex);
        else if(option == 1)
            GetViewerMethods()->DemoteOperator(operatorIndex);
        else if(option == 2)
            GetViewerMethods()->RemoveOperator(operatorIndex);

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    OperatorPluginManager *pluginManager = GetViewerProxy()->GetOperatorPluginManager();
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
        AttributeSubject *operAtts = GetViewerState()->GetOperatorAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonOperatorPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        ScriptingOperatorPluginInfo *scripting = pluginManager->GetScriptingPluginInfo(id);
        general->CopyAttributes(operAtts, ((AttributesObject *)obj)->data);
        scripting->SetDefaults(operAtts);
        operAtts->Notify();
        GetViewerMethods()->SetDefaultOperatorOptions(objPluginIndex);
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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    PlotPluginManager *pluginManager = GetViewerProxy()->GetPlotPluginManager();
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
        AttributeSubject *plotAtts = GetViewerState()->GetPlotAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonPlotPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        ScriptingPlotPluginInfo *scripting = pluginManager->GetScriptingPluginInfo(id);
        general->CopyAttributes(plotAtts, ((AttributesObject *)obj)->data);
        scripting->SetDefaults(plotAtts);
        plotAtts->Notify();
        GetViewerMethods()->SetDefaultPlotOptions(objPluginIndex);
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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
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
    PlotPluginManager *pluginManager = GetViewerProxy()->GetPlotPluginManager();
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
        AttributeSubject *plotAtts = GetViewerState()->GetPlotAttributes(objPluginIndex);
        std::string id(pluginManager->GetEnabledID(objPluginIndex));
        CommonPlotPluginInfo *general = pluginManager->GetCommonPluginInfo(id);
        general->CopyAttributes(plotAtts, ((AttributesObject *)obj)->data);
        plotAtts->Notify();
        GetViewerMethods()->SetPlotOptions(objPluginIndex);
    }
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_GetPlotOptions
//
// Purpose: 
//   Returns a copy of the plot options for the first active plot or the first
//   plot if no plots are active.
//
// Arguments:
//
// Returns:    A PyObject copy of the plot attributes or None if there are 
//             no plots.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 15 10:09:18 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Jun 25 09:00:30 PDT 2008
//   Added an argument to GetPyObjectPluginAttributes.
//
// ****************************************************************************

STATIC PyObject *
visit_GetPlotOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *retval = 0;
    MUTEX_LOCK();
        if(GetViewerState()->GetPlotList()->GetNumPlots() > 0)
        {
            // Get the plot type of the first selected plot.
            int plotType = 0;
            for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
            {
                const Plot &p = GetViewerState()->GetPlotList()->operator[](i);
                if(p.GetActiveFlag())
                {
                    plotType = p.GetPlotType();
                    break;
                }
            }

            AttributeSubject *plotAtts = GetViewerState()->GetPlotAttributes(plotType);
            retval = GetPyObjectPluginAttributes(plotAtts, true, GetViewerProxy());
        }
        else
        {
            VisItErrorFunc("There are no plots.");
        }
    MUTEX_UNLOCK();

    return retval;
}

// ****************************************************************************
// Method: visit_GetOperatorOptions
//
// Purpose: 
//   Returns a copy of the operator options for i'th operator in the the first 
//   active plot or the first plot if no plots are active.
//
// Arguments:
//
// Returns:    A PyObject copy of the operator attributes or None if there are 
//             no plots.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 15 10:09:18 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Jun 25 09:00:51 PDT 2008
//   Added an argument to GetPyObjectPluginAttributes
//
// ****************************************************************************

STATIC PyObject *
visit_GetOperatorOptions(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int operatorIndex = 0;
    if(!PyArg_ParseTuple(args, "i", &operatorIndex))
        return NULL;

    int plotIndex = 0, plotType = -1, operatorType = 0;
    PyObject *retval = 0;
    MUTEX_LOCK();
        PlotList plCopy(*GetViewerState()->GetPlotList());

        if(plCopy.GetNumPlots() > 0)
        {
            // Get the plot type of the first selected plot.
            for(int i = 0; i < plCopy.GetNumPlots(); ++i)
            {
                const Plot &p = plCopy[i];
                if(p.GetActiveFlag())
                {
                    plotIndex = i;
                    plotType = p.GetPlotType();
                    break;
                }
            }

            // Let's check that the operator index that was passed is in range.
            if(operatorIndex < 0 || operatorIndex >= plCopy[plotIndex].GetOperators().size())
            {
                VisItErrorFunc("The supplied operator index was out of range.");
            }
            else
            {
                // Let's see if there are multiple types of the same operator since we'll
                // have to do a little more work in that case.
                const Plot &p = plCopy[plotIndex];
                int opCount = 0;
                operatorType = p.GetOperators()[operatorIndex];
                for(int op = 0; op < p.GetOperators().size(); ++op)
                {
                    if(p.GetOperators()[op] == operatorType)
                        ++opCount;
                }

                if(opCount == 1 || p.GetActiveOperator() == operatorIndex)
                {
                    AttributeSubject *opAtts = GetViewerState()->
                        GetOperatorAttributes(operatorType);
                    retval = GetPyObjectPluginAttributes(opAtts, true, GetViewerProxy());
                }
                else
                {
                    // We need to do more work.
                    debug3 << "GetOperatorOptions: Must set the active operator "
                              "prior to returning operator attributes."
                           << endl;

                    // Create the arguments to SetActivePlots.
                    intVector activePlots, activeOperatorIds, expandedPlots;
                    for(int i = 0; i < plCopy.GetNumPlots(); ++i)
                    {
                        const Plot &p = plCopy[i];
                        if(p.GetActiveFlag())
                            activePlots.push_back(i);
                        activeOperatorIds.push_back(p.GetActiveOperator());
                        expandedPlots.push_back(p.GetExpandedFlag()?1:0);
                    }
                    activeOperatorIds[plotIndex] = operatorIndex;
                    expandedPlots[plotIndex] = 1;
                    GetViewerMethods()->SetActivePlots(activePlots, 
                        activeOperatorIds, expandedPlots);

                    // Wait for the selected operator attributes to come back.
                    MUTEX_UNLOCK();
                    Synchronize();
                    MUTEX_LOCK();

                    // Create the current operator attributes since they've now 
                    // come back for the selected operator
                    AttributeSubject *opAtts = GetViewerState()->
                        GetOperatorAttributes(operatorType);
                    retval = GetPyObjectPluginAttributes(opAtts, true, GetViewerProxy());

                    // Restore the active operator and plot expansion for the 
                    // affected plot.
                    debug3 << "GetOperatorOptions: Restoring the active operator."
                           << endl;
                    activeOperatorIds[plotIndex] = plCopy[plotIndex].GetActiveOperator();
                    expandedPlots[plotIndex] = plCopy[plotIndex].GetExpandedFlag()?1:0;
                    GetViewerMethods()->SetActivePlots(activePlots, 
                        activeOperatorIds, expandedPlots);

                    // Wait for the old operator attributes to come back.
                    MUTEX_UNLOCK();
                    Synchronize();
                    MUTEX_LOCK();
                }
            }
        }
        else
        {
            VisItErrorFunc("There are no plots so there can't be any operators.");
        }
    MUTEX_UNLOCK();

    return retval;
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
//   Brad Whitlock, Wed Jan 23 10:05:43 PDT 2008
//   Changed how the applyToAllPlots works a little.
//
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplyOperator() since we do not affect operators 
//
//   Gunther H. Weber, Tue Apr  1 16:50:33 PDT 2008
//   Restore state of setApplySelection toggle
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
    int applyToAllPlots = 0;
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
        bool applySelectionSave = GetViewerState()->GetGlobalAttributes()->GetApplySelection();
        GetViewerState()->GetGlobalAttributes()->SetApplySelection(applyToAllPlots != 0);
        GetViewerState()->GetGlobalAttributes()->Notify();

        // Set the sil restriction.
        avtSILRestriction_p silr = PySILRestriction_FromPyObject(obj);
        GetViewerProxy()->SetPlotSILRestriction(silr);
        
        // Restore apply selection toggle
        GetViewerState()->GetGlobalAttributes()->SetApplySelection(applySelectionSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
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
    NO_ARGUMENTS();

    std::string pickOut;
    PickAttributes *pa = GetViewerState()->GetPickAttributes();
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
    NO_ARGUMENTS();

    std::string queryOut;
    QueryAttributes *qa = GetViewerState()->GetQueryAttributes();
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
    NO_ARGUMENTS();

    QueryAttributes *qa = GetViewerState()->GetQueryAttributes();
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
// Function: visit_GetQueryOutputXML
//
// Purpose:
//   Returns the xml string result set by a query.
//
//
// Programmer: Cyrus Harrison
// Creation:   December 17, 2007
//
// ****************************************************************************

STATIC PyObject *
visit_GetQueryOutputXML(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    
    QueryAttributes *qa = GetViewerState()->GetQueryAttributes();
    std::string xml_string = qa->GetXmlResult();
    return PyString_FromString(xml_string.c_str());
}


// ****************************************************************************
// Function: visit_GetQueryOutputObject
//
// Purpose:
//   Returns a python dictonary created from an xml query result.
//   Assumes the xml query result is a serialized MapNode.
//
//
// Programmer: Cyrus Harrison
// Creation:   December 17, 2007 
//
// ****************************************************************************

STATIC PyObject *
visit_GetQueryOutputObject(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    QueryAttributes *qa = GetViewerState()->GetQueryAttributes();
    string xml_string = qa->GetXmlResult();
    XMLNode xml_node(xml_string);
    MapNode node(xml_node);
    return PyMapNode_Wrap(node);
}



// ****************************************************************************
// Function: visit_GetOutputArray
//
// Purpose:
//   Returns the output array for the active plot.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   June 20, 2006 
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_GetOutputArray(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int winId = -1;
    int plotId = -1;
    if (!PyArg_ParseTuple(args, "i", &plotId))
    {
        if (!PyArg_ParseTuple(args, "ii", &plotId, &winId))
        {
        }
        PyErr_Clear();
    }
    PyObject *retval;
    MUTEX_LOCK();
        GetViewerMethods()->UpdatePlotInfoAtts(plotId, winId);
    MUTEX_UNLOCK();
    // Wait until viewer has finished updating the plot Info atts
    int error = Synchronize();
    // Retrieve the update plot info atts.
    PlotInfoAttributes *pia = GetViewerState()->GetPlotInfoAttributes();
    if (pia == NULL)
    {
        retval = PyString_FromString("Plot did not define an output array."); 
    }
    else
    {
        doubleVector vals = pia->GetOutputArray();
        if (vals.size() == 0)
          retval = PyString_FromString("Plot did not define an output array." );
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
//   Kathleen Bonnell, Thu Jul 21 13:11:49 PDT 2005 
//   Removed test for number of maps, so that domains can still be listed
//   even with the presense of groups in the data. 
//
// ****************************************************************************

void
ListCategoryHelper(SILCategoryRole role)
{
    MUTEX_LOCK();
    if(viewer)
    {
        avtSILRestriction_p silr = GetViewerProxy()->GetPlotSILRestriction();

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
                    if(collection->GetSupersetIndex() == silr->GetTopSet()) 
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
        avtSILRestriction_p silr = GetViewerProxy()->GetPlotSILRestriction();

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
//   Kathleen Bonnell, Thu Jul 21 13:11:49 PDT 2005 
//   Removed test for number of maps, so that domains can still be turned
//   on/off even with the presense of groups in the data. 
//
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplyOperator() since we do not affect operators 
//
//   Gunther H. Weber, Tue Apr  1 16:50:33 PDT 2008
//   Restore state of setApplySelection toggle
//
// ****************************************************************************

bool
TurnOnOffHelper(SILCategoryRole role, bool val, const stringVector &names)
{
    bool retval = true;
    avtSILRestriction_p silr = GetViewerProxy()->GetPlotSILRestriction();

    // Print the material collections for each whole set in the SIL.
    for(int i = 0; i < silr->GetNumCollections(); ++i)
    {
        avtSILCollection_p collection = silr->GetSILCollection(i);
        if(collection->GetRole() == role)
        {
            const std::vector<int> &sets = collection->GetSubsetList();
            if(sets.size() > 0)
            {
                if(collection->GetSupersetIndex() == silr->GetTopSet()) 
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
    bool applySelectionSave = GetViewerState()->GetGlobalAttributes()->GetApplySelection();
    GetViewerState()->GetGlobalAttributes()->SetApplySelection(false);
    GetViewerState()->GetGlobalAttributes()->Notify();

    // Send the modified SIL restriction to the viewer.
    GetViewerProxy()->SetPlotSILRestriction(silr);
        
    // Restore apply selection toggle
    GetViewerState()->GetGlobalAttributes()->SetApplySelection(applySelectionSave);
    GetViewerState()->GetGlobalAttributes()->Notify();

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
    NO_ARGUMENTS();

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
    NO_ARGUMENTS();

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
    NO_ARGUMENTS();

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
    NO_ARGUMENTS();

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
    NO_ARGUMENTS();

    MUTEX_LOCK();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    const stringVector &ctNames = GetViewerState()->GetColorTableAttributes()->GetNames();
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
    NO_ARGUMENTS();

    const stringVector &ctNames = GetViewerState()->GetColorTableAttributes()->GetNames();
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
        GetViewerMethods()->SetActiveContinuousColorTable(ctName);
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
        GetViewerMethods()->SetActiveDiscreteColorTable(ctName);
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

    const std::string &ct = GetViewerState()->GetColorTableAttributes()->GetActiveContinuous();
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
    NO_ARGUMENTS();

    const std::string &ct = GetViewerState()->GetColorTableAttributes()->GetActiveDiscrete();
    PyObject *retval = PyString_FromString(ct.c_str());

    return retval;
}

// ****************************************************************************
// Method: visit_AddColorTable
//
// Purpose: 
//   Adds a color control point list as a new color table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 13 16:15:25 PST 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_AddColorTable(PyObject *self, PyObject *args)
{
    char *ctName = 0;
    PyObject *ccpl = 0;
    if(!PyArg_ParseTuple(args, "sO", &ctName, &ccpl))
    {
        VisItErrorFunc("The arguments must be: color table name, followed by a color control point list.");
        return NULL;
    }

    if(PyColorControlPointList_Check(ccpl))
    {
        MUTEX_LOCK();
            // Remove the color table in case it already exists.
            GetViewerState()->GetColorTableAttributes()->RemoveColorTable(ctName);

            // Add the color table.
            GetViewerState()->GetColorTableAttributes()->AddColorTable(ctName,
                *PyColorControlPointList_FromPyObject(ccpl));

            // Tell the viewer.
            GetViewerState()->GetColorTableAttributes()->Notify();
            GetViewerMethods()->UpdateColorTable(ctName);
        MUTEX_UNLOCK();
    }
    else
    {
        VisItErrorFunc("The arguments must be: color table name, followed by a color control point list.");
        return NULL;
    }

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_RemoveColorTable
//
// Purpose: 
//   Removes a color table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 13 16:17:33 PST 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RemoveColorTable(PyObject *self, PyObject *args)
{
    char *ctName = 0;
    if(!PyArg_ParseTuple(args, "s", &ctName))
    {
        VisItErrorFunc("The arguments must be a color table name.");
        return NULL;
    }

    MUTEX_LOCK();
        // Remove the color table in case it already exists.
        GetViewerState()->GetColorTableAttributes()->RemoveColorTable(ctName);
        // Tell the viewer.
        GetViewerState()->GetColorTableAttributes()->Notify();
        GetViewerMethods()->UpdateColorTable(ctName);
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Method: visit_ColorTable
//
// Purpose: 
//   Gets the named color table definition.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 3 16:26:42 PST 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetColorTable(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *ctName;
    if(!PyArg_ParseTuple(args, "s", &ctName))
    {
        VisItErrorFunc("The argument must be a color table name.");
        return NULL;
    }
    PyObject *ct = NULL;
    MUTEX_LOCK();
        const ColorControlPointList *ctptr = GetViewerState()->GetColorTableAttributes()->GetColorControlPoints(ctName);
        if(ctptr != 0)
        {
            ct = PyColorControlPointList_New();
            ColorControlPointList *ccpl = PyColorControlPointList_FromPyObject(ct);
            *ccpl = *ctptr;
        }
    MUTEX_UNLOCK();
    if(ctptr == 0)
    {
        VisItErrorFunc("The argument must be a color table name.");
        return NULL;
    }

    return ct;
}

STATIC PyObject *
visit_SetColorTable(PyObject *self, PyObject *args)
{
     return visit_AddColorTable(self, args);
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
    NO_ARGUMENTS();

    return PyInt_FromLong(long(GetViewerState()->GetPlotList()->GetNumPlots()));
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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//   
// ****************************************************************************

STATIC PyObject *
visit_PlotPlugins(PyObject *self, PyObject *args)
{
    NO_ARGUMENTS();

    PlotPluginManager *plugins = GetViewerProxy()->GetPlotPluginManager();

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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//   
// ****************************************************************************

STATIC PyObject *
visit_NumPlotPlugins(PyObject *self, PyObject *args)
{
    NO_ARGUMENTS();

    PlotPluginManager *plugins = GetViewerProxy()->GetPlotPluginManager();

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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//
// ****************************************************************************

STATIC PyObject *
visit_OperatorPlugins(PyObject *self, PyObject *args)
{
    NO_ARGUMENTS();

    OperatorPluginManager *plugins = GetViewerProxy()->GetOperatorPluginManager();

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
    NO_ARGUMENTS();

    stringVector queries = GetViewerState()->GetQueryList()->GetNames();

    // We only want to include Database queries, so count them.
    intVector types = GetViewerState()->GetQueryList()->GetTypes();
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
//   Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//   Reflect changes to QueryList: QueryTime is now QueryMode, and
//   has three values, QueryOnly, QueryAndTime, and TimeOnly.
//   
// ****************************************************************************

STATIC PyObject *
visit_QueriesOverTime(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    stringVector queries = GetViewerState()->GetQueryList()->GetNames();

    // We only want to include Database time queries, so count them.
    intVector types = GetViewerState()->GetQueryList()->GetTypes();
    intVector mode = GetViewerState()->GetQueryList()->GetQueryMode();
    int nQueries = 0; 
    for(int i = 0; i < types.size(); ++i)
    {
        if (types[i] == QueryList::DatabaseQuery && 
            mode[i] != QueryList::QueryOnly )
            nQueries++;
    }

    // Allocate a tuple with enough entries to hold the queries name list.
    PyObject *retval = PyTuple_New(nQueries);

    for(int j = 0, k = 0; j < queries.size(); ++j)
    {
        if (types[j] == QueryList::DatabaseQuery && 
            mode[j] != QueryList::QueryOnly)
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
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//   
// ****************************************************************************

STATIC PyObject *
visit_NumOperatorPlugins(PyObject *self, PyObject *args)
{
    NO_ARGUMENTS();
    OperatorPluginManager *plugins = GetViewerProxy()->GetOperatorPluginManager();

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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->PrintWindow();
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
        GetViewerMethods()->SetWindowArea(x, y, w, h);
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
        AnimationAttributes *atts = GetViewerState()->GetAnimationAttributes();
        atts->SetTimeout(milliSeconds);
        atts->Notify();
        GetViewerMethods()->SetAnimationAttributes();
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
    NO_ARGUMENTS();
    return PyLong_FromLong(long(GetViewerState()->GetAnimationAttributes()->GetTimeout()));
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
        AnimationAttributes *atts = GetViewerState()->GetAnimationAttributes();
        atts->SetPipelineCachingMode(val != 0);
        atts->Notify();
        GetViewerMethods()->SetAnimationAttributes();
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
    NO_ARGUMENTS();
    return PyLong_FromLong(long(GetViewerState()->GetAnimationAttributes()->GetPipelineCachingMode()?1:0));
}

// ****************************************************************************
//  Method:  visit_LoadAttribute
//
//  Purpose:
//    Loads a single attribute from an XML file.
//
//  Note:  this is not exactly type-safe!  We make some attempt to check
//         that the name of its type ends in "Attributes", but if that's
//         not a sufficiently strict check, it can crash.  That check is
//         also not quite right, because not all AttributeSubjects end
//         in "Attributes", but since all the plot and operator ones do,
//         it's not too bad.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  5, 2009
//
// ****************************************************************************

// We don't have a python abstraction of a generic attribute
// subject, but they all follow this pattern, so we'll fake it.
struct GenericAttributeSubjectObject
{
    PyObject_HEAD
    AttributeSubject *data;
    bool  owns;
};

STATIC PyObject *
visit_LoadAttribute(PyObject *self, PyObject *args)
{
    char *filename;
    PyObject *attobj;
    if (!PyArg_ParseTuple(args, "sO", &filename, &attobj))
        return NULL;

    const char *objtypename = attobj->ob_type->tp_name;
    if (strlen(objtypename) <= 10 ||
        strcmp(objtypename+(strlen(objtypename)-10), "Attributes") != 0)
    {
        VisItErrorFunc("Unceremoniously refusing to load into an object "
                       "whose type name does not end in 'Attributes'.  "
                       "If this check is incorrect, please contact a "
                       "developer.");
        return NULL;
        
    }

    AttributeSubject *as =
        reinterpret_cast<GenericAttributeSubjectObject*>(attobj)->data;

    if (!as || !filename)
        return NULL;
        
    SingleAttributeConfigManager mgr(as);
    mgr.Import(filename);
    as->SelectAll();
    as->Notify();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
//  Method:  visit_LoadAttribute
//
//  Purpose:
//    Loads a single attribute from an XML file.
//
//  Note:  this is not exactly type-safe!  We make some attempt to check
//         that the name of its type ends in "Attributes", but if that's
//         not a sufficiently strict check, it can crash.  That check is
//         also not quite right, because not all AttributeSubjects end
//         in "Attributes", but since all the plot and operator ones do,
//         it's not too bad.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  5, 2009
//
// ****************************************************************************

STATIC PyObject *
visit_SaveAttribute(PyObject *self, PyObject *args)
{
    char *filename;
    PyObject *attobj;
    if (!PyArg_ParseTuple(args, "sO", &filename, &attobj))
        return NULL;

    const char *objtypename = attobj->ob_type->tp_name;
    if (strlen(objtypename) <= 10 ||
        strcmp(objtypename+(strlen(objtypename)-10), "Attributes") != 0)
    {
        VisItErrorFunc("Unceremoniously refusing to load into an object "
                       "whose type name does not end in 'Attributes'.  "
                       "If this check is incorrect, please contact a "
                       "developer.");
        return NULL;
        
    }

    AttributeSubject *as =
        reinterpret_cast<GenericAttributeSubjectObject*>(attobj)->data;

    if (!as || !filename)
        return NULL;
        
    SingleAttributeConfigManager mgr(as);
    mgr.Export(filename);

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetLight
//
// Purpose: 
//   Sets a light by index (0..7)
//
// Programmer: Jeremy Meredith
// Creation:   October 28, 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Feb  6 16:23:30 PST 2006
//   Don't allow light0 to be turned off.
//
// ****************************************************************************

STATIC PyObject *
visit_SetLight(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int index;
    PyObject *pylight;
    if (!PyArg_ParseTuple(args, "iO", &index, &pylight))
        return NULL;

    LightAttributes *light = PyLightAttributes_FromPyObject(pylight);
    if (index == 0 && !light->GetEnabledFlag())
    {
        light->SetEnabledFlag(true);
        cerr << "Warning:  Cannot un-enable light 0.  To turn off lighting " 
             << "for all plots, change light 0 type to Ambient." << endl; 
    }
    MUTEX_LOCK();
    LightList *lightlist = GetViewerState()->GetLightList();
    lightlist->SetLight(index, *light);
    lightlist->Notify();
    GetViewerMethods()->SetLightList();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_GetLight
//
// Purpose: 
//   Gets a light by index (0..7)
//
// Programmer: Jeremy Meredith
// Creation:   October 28, 2004
//
// Modifications:
//   Kathleen Bonnell, Fri Feb 24 10:36:00 PST 2006
//   Ensure light 0 is always enabled.
//
// ****************************************************************************

STATIC PyObject *
visit_GetLight(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    int index;
    if (!PyArg_ParseTuple(args, "i", &index))
        return NULL;

    PyObject *pylight = PyLightAttributes_New();
    LightAttributes *light = PyLightAttributes_FromPyObject(pylight);
    if (index == 0)
    {
        light->SetEnabledFlag(true);
    }
    *light = GetViewerState()->GetLightList()->GetLight(index);

    return pylight;
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
//   Brad Whitlock, Fri Jun 24 10:50:12 PDT 2005
//   Made the message include the name of the file that could not be sourced.
//
//   Brad Whitlock, Tue Jan 10 12:04:10 PDT 2006
//   Changed how logging works.
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
    char buf[1024];
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        //
        // Add a ".py" extension and try to open the file.
        //
        SNPRINTF(buf, 1024, "%s.py", fileName);
        fp = fopen(buf, "rb");
        if(fp == NULL)
        {
            SNPRINTF(buf, 1024, "Could not find file %s for sourcing.", fileName);
            VisItErrorFunc(buf);
            return NULL;
        }
    }

    //
    // Turn logging off.
    //
    SNPRINTF(buf, 1024, "Source(\"%s\")\n", fileName);
    LogFile_Write(buf);
    LogFile_IncreaseLevel();

    //
    // Execute the commands in the file.
    //
    PyRun_SimpleFile(fp, fileName);
    fclose(fp);

    //
    // Turn logging back on.
    //
    LogFile_DecreaseLevel();

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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleMaintainViewMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleMaintainDataMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleLockTime();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_ToggleLockTools
//
// Purpose: 
//   Tells the viewer to toggle tool locking for the active vis window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 17 10:15:55 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_ToggleLockTools(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleLockTools();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleBoundingBoxMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleLockViewMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleSpinMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleCameraViewMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ToggleFullFrameMode();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->UndoView();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_RedoView
//
// Purpose:
//   Tells the viewer to redo the last view operation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 16:39:03 PST 2006
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_RedoView(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->RedoView();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->WriteConfigFile();
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
//   Kathleen Bonnell, Tue Dec 28 16:23:43 PST 2004
//   Support 'Global' in query name, to designate use of global id.
//
//   Brad Whitlock, Tue Jan 4 16:19:15 PST 2005
//   strcasecmp does not exist on Windows so I made it use _strnicmp.
//
//   Hank Childs, Tue Aug  1 12:20:56 PDT 2006
//   Add support for line distributions.
//
//   Dave Bremer, Wed Jan 17 19:02:51 PST 2007
//   Added parsing appropriate for the Hohlraum flux query.
//
//   Dave Bremer, Fri Jan 19 16:46:19 PST 2007
//   Patched to call PyErr_Clear() after each failed attempt to parse
//   the tuple.  
//
//   Cyrus Harrison, Tue Dec 18 21:06:34 PST 2007
//   Added support for Shapelet Decomposition, removed nested parsing
//   because it was getting real hairy.
//
//   Cyrus Harrison, Thu Dec 20 17:15:58 PST 2007
//   Fixed parsing conflict.
//
//   Cyrus Harrison, Thu Jan  3 11:42:16 PST 2008
//   Another stab at fixing the parsing conflict. 
//
// ****************************************************************************

STATIC PyObject *
visit_Query(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    char *queryName = NULL;
    char *output_name = NULL;
    int arg1 = 0, arg2 = 0;
    doubleVector darg1(3), darg2(3);
    PyObject *tuple = NULL;
    
    bool parse_success = false;
    
    parse_success = PyArg_ParseTuple(args, "sidddddd|O", &queryName, &arg1,
                                     &(darg1[0]), &(darg1[1]), &(darg1[2]),
                                     &(darg2[0]), &(darg2[1]), &(darg2[2]),
                                     &tuple);
    if(!parse_success)
    {
        PyErr_Clear();
        darg1.resize(1);
        darg2.resize(1);
        parse_success = PyArg_ParseTuple(args, "siidd|O", &queryName, &arg1,
                                         &arg2, &(darg1[0]), &(darg2[0]), 
                                         &tuple);
    }
    
    // shapelets
    if(!parse_success)
    {
        PyErr_Clear();
        darg1.resize(1);
        darg2.resize(0);
        parse_success = PyArg_ParseTuple(args, "sdis|O", &queryName, 
                                         &(darg1[0]), &arg1, &output_name,
                                         &tuple);
    }
    
    // shapelets (with output)
    if(!parse_success)
    {
        PyErr_Clear();
        parse_success = PyArg_ParseTuple(args, "sdi|O", &queryName, 
                                         &(darg1[0]), &arg1, &tuple);
    }

    if(parse_success)
    {
        // args for Zone Center and Node Coords need a special fix here.
        std::string qname(queryName);
        if(qname == "Zone Center" || qname == "Node Coords" )
        {
            arg2 = arg1;
            arg1 = (int)darg1[0];
            darg1.resize(0);
        }
    }

    if(!parse_success)
    {
        PyErr_Clear();
        darg1.resize(0);
        parse_success = PyArg_ParseTuple(args, "sii|O", &queryName,
                                         &arg1, &arg2, &tuple);
    }
    
    
    if(!parse_success)
    {
        PyErr_Clear();
        parse_success = PyArg_ParseTuple(args, "si|O", &queryName, &arg1,
                                         &tuple);
    }
    
    if(!parse_success)
    {
        PyErr_Clear();
        parse_success = PyArg_ParseTuple(args, "s|O", &queryName, &tuple);
    }
    
    // we could not parse the args!
    if(!parse_success)
        return NULL;
    
    // Check for global flag.
    std::string qname(queryName);
    bool doGlobal = false;
#if defined(_WIN32)
    if (_strnicmp(queryName, "Global ", 7) == 0)
#else
    if (strncasecmp(queryName, "Global ", 7) == 0)
#endif
    {
        std::string::size_type pos1 = 0;
        pos1 = qname.find_first_of(' ', pos1);
        qname = qname.substr(pos1+1);
        doGlobal = true;
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);
    
    // magic for Shapelet Decomposition call
    if(output_name != NULL)
    {
        vars.push_back("default");
        vars.push_back(std::string(output_name));
    }

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
        GetViewerMethods()->DatabaseQuery(qname, vars, false, arg1, arg2, doGlobal,
                              darg1, darg2);
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}


// ****************************************************************************
// Function: visit_SuppressQueryOutputOn
//
// Purpose:
//   Turns on the suppression of query output (the automatic printing
//   of the QueryOutput string.)
//
// Programmer: Kathleen Bonnell
// Creation:   July 27, 2005
//
// Modifications:
//
// ****************************************************************************

STATIC PyObject *
visit_SuppressQueryOutputOn(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();
    MUTEX_LOCK();
        GetViewerMethods()->SuppressQueryOutput(true);
    MUTEX_UNLOCK();
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SuppressQueryOutputOff
//
// Purpose: 
//   Turns off the suppression of query output (the automatic printing
//   of the QueryOutput string.) 
//
// Programmer: Kathleen Bonnell 
// Creation:   July 27, 2005 
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_SuppressQueryOutputOff(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();
    MUTEX_LOCK();
        GetViewerMethods()->SuppressQueryOutput(false);
    MUTEX_UNLOCK();
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetQueryFloatFormat()
//
// Purpose:
//   Sets the floating point format string used to generate query output.
//
// Programmer: Cyrus Harrison
// Creation:   September 19, 2007
//
// Modifications:
//    Cyrus Harrison, Mon Sep 24 15:25:18 PDT 2007
//    Added check for valid format string
//
// ****************************************************************************

STATIC PyObject *
visit_SetQueryFloatFormat(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    char *format_string;
    if(!PyArg_ParseTuple(args, "s", &format_string))
        return NULL;
    if(! StringHelpers::ValidatePrintfFormatString(format_string,
                                                    "float","EOA"))
    {
        VisItErrorFunc("Invalid floating point format string.");
        return NULL;
    }
        
    MUTEX_LOCK();
        GetViewerMethods()->SetQueryFloatFormat(format_string);
    MUTEX_UNLOCK();
    
    
    Py_INCREF(Py_None);
    return Py_None;
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
//   Kathleen Bonnell, Wed Feb 23 11:23:23 PST 2005 
//   Changed arg1 default to 0 (is used to specify 'original' data). 
//
//   Brad Whitlock, Tue Jan 10 14:04:35 PST 2006
//   Changed logging.
//
// ****************************************************************************

STATIC PyObject *
visit_QueryOverTime(PyObject *self, PyObject *args)
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
        GetViewerMethods()->DatabaseQuery(queryName, vars, true, arg1, arg2);

        char tmp[1024];
        SNPRINTF(tmp, 1024, "QueryOverTime(\"%s\", %d, %d, %s)\n", queryName,
                 arg1, arg2,
                 StringVectorToTupleString(vars).c_str());
        LogFile_Write(tmp);
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
//   Brad Whitlock, Tue Jan 10 14:05:47 PST 2006
//   Changed logging.
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
            VisItErrorFunc("The first argument to Pick must be a point "
                           "specified as a tuple of coordinates.");
            return NULL;
        }
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    MUTEX_LOCK();
        if (!wp)
            GetViewerMethods()->Pick(x, y, vars);
        else 
            GetViewerMethods()->Pick(pt,  vars);

        char tmp[1024];
        SNPRINTF(tmp, 1024, "Pick(%d, %d, %s)\n", x, y,
                 StringVectorToTupleString(vars).c_str());
        LogFile_Write(tmp);
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
//   Brad Whitlock, Tue Jan 10 14:07:43 PST 2006
//   Changed logging.
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
            VisItErrorFunc("The first argument to NodePick must be a "
                           "point specified as a tuple of coordinates.");
            return NULL;
        }
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    MUTEX_LOCK();
        if (!wp)
            GetViewerMethods()->NodePick(x, y, vars);
        else 
            GetViewerMethods()->NodePick(pt, vars);

        char tmp[1024];
        SNPRINTF(tmp, 1024, "Pick(%d, %d, %s)\n", x, y,
                 StringVectorToTupleString(vars).c_str());
        LogFile_Write(tmp);
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ResetPickLetter();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ResetLineoutColor();
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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ResetPickAttributes();
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
        *(GetViewerState()->GetPickAttributes()) = *pa;
        GetViewerState()->GetPickAttributes()->Notify();
        GetViewerMethods()->SetPickAttributes();
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
        *(GetViewerState()->GetPickAttributes()) = *pa;
        GetViewerState()->GetPickAttributes()->Notify();
        GetViewerMethods()->SetDefaultPickAttributes();
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
    NO_ARGUMENTS();

    PyObject *retval = PyPickAttributes_New();
    PickAttributes *pa = PyPickAttributes_FromPyObject(retval);

    // Copy the viewer proxy's pick atts into the return data structure.
    *pa = *(GetViewerState()->GetPickAttributes());

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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ResetInteractorAttributes();
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
        *(GetViewerState()->GetInteractorAttributes()) = *ia;
        GetViewerState()->GetInteractorAttributes()->Notify();
        GetViewerMethods()->SetInteractorAttributes();
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
        *(GetViewerState()->GetInteractorAttributes()) = *ia;
        GetViewerState()->GetInteractorAttributes()->Notify();
        GetViewerMethods()->SetDefaultInteractorAttributes();
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
    NO_ARGUMENTS();

    PyObject *retval = PyInteractorAttributes_New();
    InteractorAttributes *ia = PyInteractorAttributes_FromPyObject(retval);

    // Copy the viewer proxy's pick atts into the return data structure.
    *ia = *(GetViewerState()->GetInteractorAttributes());

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
    NO_ARGUMENTS();

    MUTEX_LOCK();
        GetViewerMethods()->ResetQueryOverTimeAttributes();
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
        *(GetViewerState()->GetQueryOverTimeAttributes()) = *tqa;
        GetViewerState()->GetQueryOverTimeAttributes()->Notify();
        GetViewerMethods()->SetQueryOverTimeAttributes();
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
        *(GetViewerState()->GetQueryOverTimeAttributes()) = *tqa;
        GetViewerState()->GetQueryOverTimeAttributes()->Notify();
        GetViewerMethods()->SetDefaultQueryOverTimeAttributes();
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
    NO_ARGUMENTS();

    PyObject *retval = PyQueryOverTimeAttributes_New();
    QueryOverTimeAttributes *tqa = PyQueryOverTimeAttributes_FromPyObject(retval);

    // Copy the viewer proxy's pick atts into the return data structure.
    *tqa = *(GetViewerState()->GetQueryOverTimeAttributes());

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
        *(GetViewerState()->GetGlobalLineoutAttributes()) = *gla;
        GetViewerState()->GetGlobalLineoutAttributes()->Notify();
        GetViewerMethods()->SetGlobalLineoutAttributes();
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
    NO_ARGUMENTS();

    PyObject *retval = PyGlobalLineoutAttributes_New();
    GlobalLineoutAttributes *gla = PyGlobalLineoutAttributes_FromPyObject(retval);

    // Copy the viewer proxy's global lineout atts into the return data structure.
    *gla = *(GetViewerState()->GetGlobalLineoutAttributes());

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
//   Kathleen Bonnell, Thu Dec 16 17:31:10 PST 2004 
//   Added bool arg, indicating of node/zone is global or not. 
//
// ****************************************************************************

STATIC PyObject *
visit_DomainPick(const char *type, int el, int dom, stringVector vars, 
                 bool doGlobal)
{
    double pt[3] = {0., 0., 0};

    MUTEX_LOCK();
       GetViewerMethods()->PointQuery(type, pt, vars, false, el, dom, doGlobal);
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
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
visit_PickByZone(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const char *type = "PickByZone";
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
    return visit_DomainPick(type, zone, dom, vars, false);
}

// ****************************************************************************
// Function: visit_PickByGlobalZone
//
// Purpose:
//   Tells the viewer to perform Pick via global zone id. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 16, 2004
//
// Modifications:
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
visit_PickByGlobalZone(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const char *type = "PickByZone";
    int dom = 0, zone = 0;
    PyObject *tuple = NULL;
    if (!PyArg_ParseTuple(args, "i|O", &zone, &tuple))
    {
        return NULL;
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    // Return the success value.
    return visit_DomainPick(type, zone, dom, vars, true);
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
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************
STATIC PyObject *
visit_PickByNode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const char *type = "PickByNode";
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
    return visit_DomainPick(type, node, dom, vars, false);
}


// ****************************************************************************
// Function: visit_PickByGlobalNode
//
// Purpose:
//   Tells the viewer to perform Pick via global node id. 
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   December 16, 2004
//
// Modifications:
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
visit_PickByGlobalNode(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    const char *type = "PickByNode";
    int dom = 0, node = 0;
    PyObject *tuple = NULL;
    if (!PyArg_ParseTuple(args, "i|O", &node, &tuple))
    {
        return NULL;
    }

    // Check the tuple argument.
    stringVector vars;
    GetStringVectorFromPyObject(tuple, vars);

    // Return the success value.
    return visit_DomainPick(type, node, dom, vars, true);
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
//    Kathleen Bonnell, Tue May 15 10:36:47 PDT 2007 
//    Added haveSamples. 
//
//   Gunther H. Weber, Tue Apr  1 15:46:53 PDT 2008
//   Removed SetApplySelection() since we do not affect SIL selection 
//
//   Gunther H. Weber, Tue Apr  1 16:42:15 PDT 2008
//   Save state of "apply operator toggle"
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
    bool haveSamples = false;
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
        else
        {
            haveSamples = true;
        }
        PyErr_Clear();
    }
    else
    {
        haveSamples = true;
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
        bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(false);
        GetViewerState()->GetGlobalAttributes()->Notify();
        GetViewerMethods()->Lineout(p0, p1, vars, samples, haveSamples);

        // Write the output to the log
        char tmp[1024];
        SNPRINTF(tmp, 1024, "Lineout((%g, %g, %g), (%g, %g, %g), %s)\n",
                 p0[0], p0[1], p0[2], p1[0], p1[1], p1[2],
                 StringVectorToTupleString(vars).c_str());
        LogFile_Write(tmp);

        // Restore toggle
        GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
        GetViewerState()->GetGlobalAttributes()->Notify();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
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
//   Brad Whitlock, Tue Mar 13 11:01:38 PDT 2007
//   Updated due to code generation changes.
//
//   Brad Whitlock, Thu Mar 22 03:27:21 PDT 2007
//   Changed bookkeeping.
//
// ****************************************************************************

void
UpdateAnnotationHelper(AnnotationObject *annot)
{
    const char *mName = "UpdateAnnotationHelper: ";

    // Make sure that the annotation object annot is in the localObjectMap.
    debug1 << "Searching for " << annot->GetObjectName()
           << " in the local object map... ";
    std::map<std::string, AnnotationObjectRef>::iterator pos = 
        localObjectMap.find(annot->GetObjectName());

    if(pos == localObjectMap.end())
    {
        debug1 << "not found!" << endl;

        // The annotation was not in the local object list so it must have
        // been deleted.
        VisItErrorFunc("Setting the annotation object attributes for this "
                       "object has no effect because the annotation to which "
                       "it corresponds has been deleted.");
    }
    else
    {
        debug1 << "found!" << endl;

        // Synchronize so we don't send more operations than we can handle.
        debug1 << mName << "Synchonizing..." << endl;
        Synchronize();

        MUTEX_LOCK();

        // The annotation was in the local object list.
        AnnotationObjectList *aol = GetViewerState()->GetAnnotationObjectList();
        bool found = false;
        for(int i = 0; i < aol->GetNumAnnotations(); ++i)
        {
            AnnotationObject &viewerAnnot = aol->GetAnnotation(i);

            if(viewerAnnot.GetObjectName() == annot->GetObjectName())
            {
                found = true;                

                debug1 << mName << "Sending annotation object "
                       << annot->GetObjectName() << "'s attributes to the viewer."
                       << endl;

                // Copy the annotation object into the annotation object list.
                viewerAnnot = *annot;

                // Send the options to the viewer.
                aol->SelectAll();
                aol->Notify();

                // Make the viewer use the options.
                GetViewerMethods()->SetAnnotationObjectOptions();
                break;
            }
        }

        if(!found)
        {
            debug1 << mName << "Annotation object " << annot->GetObjectName()
                   << " was not found in the viewer's annotation object list."
                   << endl;
        }

        MUTEX_UNLOCK();
        Synchronize();
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
//   Brad Whitlock, Tue Mar 13 11:01:38 PDT 2007
//   Updated due to code generation changes.
//   
// ****************************************************************************

bool
DeleteAnnotationObjectHelper(AnnotationObject *annot)
{
    const char *mName = "DeleteAnnotationObjectHelper: ";
    bool transferOwnership = false;

    debug1 << mName << "Synchronizing..." << endl;
    Synchronize();

    MUTEX_LOCK();

    // Reduce the reference count.
    debug1 << mName << "Looking for annotation object "
           << annot->GetObjectName() << " in the local object map...";
    std::map<std::string, AnnotationObjectRef>::iterator pos = 
        localObjectMap.find(annot->GetObjectName());
    if(pos != localObjectMap.end())
    {
         debug1 << "found; decremented refCount" << endl;

         --pos->second.refCount;
         if(pos->second.refCount < 2)  // Why 2?
         {
             debug1 << mName << "Remove " << annot->GetObjectName()
                    << " from the local object map." << endl;
             int deletedIndex = pos->second.index;
             localObjectMap.erase(pos);
             transferOwnership = true;

             // Decrement the indices that are larger than deletedIndex.
             for(pos = localObjectMap.begin(); pos != localObjectMap.end(); ++pos)
                 if(pos->second.index > deletedIndex)
                     --pos->second.index;

             // Erase it from the viewer too.
             AnnotationObjectList *aol = GetViewerState()->GetAnnotationObjectList();
             bool needToDelete = false;
             debug1 << "Setting annotation objects' active flag:" << endl;
             for(int i = 0; i < aol->GetNumAnnotations(); ++i)
             {
                 bool namesEqual = aol->GetAnnotation(i).GetObjectName() == annot->GetObjectName();
                 needToDelete |= namesEqual;
                 aol->GetAnnotation(i).SetActive(namesEqual);
                 debug1 << "\t" << aol->GetAnnotation(i).GetObjectName()
                        << " = " << (aol->GetAnnotation(i).GetActive()?"true":"false")
                        << endl;
             }

             if(needToDelete)
             {
                 debug1 << mName << "Deletion required." << endl;

                 // Now that we've modified the annotation object list, we have to send
                 // it to the viewer.
                 aol->SelectAll();
                 aol->Notify();
                 GetViewerMethods()->SetAnnotationObjectOptions();

                 // Now that the viewer has the new selected list of annotation objects,
                 // delete the active ones.
                 GetViewerMethods()->DeleteActiveAnnotationObjects();
             }
         }
    }
    else
    {
        debug1 << "not found! The Python object must refer to an object "
               << "that no longer exists." << endl;
    }

    MUTEX_UNLOCK();
    Synchronize();

    return transferOwnership;
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
//   Brad Whitlock, Tue Jun 28 11:48:50 PDT 2005
//   Added John Anderson's objects.
//
//   Brad Whitlock, Fri Mar 23 17:24:54 PST 2007
//   Added PyLegendAttributesObject and added an error message.
//
//   Brad Whitlock, Mon Nov 12 16:00:45 PST 2007
//   Added PyText3DObject.
//
// ****************************************************************************

PyObject *
CreateAnnotationWrapper(AnnotationObject *annot)
{
    PyObject *retval = NULL;
    if(annot->GetObjectType() == AnnotationObject::Text2D)
    {
        // Create a text2d wrapper for the new annotation object.
        retval = PyText2DObject_WrapPyObject(annot);
    }
    else if(annot->GetObjectType() == AnnotationObject::Text3D)
    {
        // Create a text3d wrapper for the new annotation object.
        retval = PyText3DObject_WrapPyObject(annot);
    }
    else if(annot->GetObjectType() == AnnotationObject::TimeSlider)
    {
        // Create a time slider wrapper for the new annotation object.
        retval = PyTimeSliderObject_WrapPyObject(annot);
    }
    else if(annot->GetObjectType() == AnnotationObject::Line2D)
    {
        // Create a Line2D wrapper for the new annotation object.
        retval = PyLineObject_WrapPyObject(annot);
    }
    else if(annot->GetObjectType() == AnnotationObject::Image)
    {
        // Create a Image wrapper for the new annotation object.
        retval = PyImageObject_WrapPyObject(annot);
    }
    else if(annot->GetObjectType() == AnnotationObject::LegendAttributes)
    {
        // Create a Image wrapper for the new annotation object.
        retval = PyLegendAttributesObject_WrapPyObject(annot);
    }
    
    // Add more cases here later...

    else
    {
        debug1 << "CreateAnnotationWrapper was asked to create a " << annot->GetObjectType()
               << " object and could not because that is an unsupported type." << endl;
    }

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
//   Brad Whitlock, Tue Jun 28 11:48:50 PDT 2005
//   Added John Anderson's objects.
//
//   Brad Whitlock, Tue Mar 13 11:01:38 PDT 2007
//   Updated due to code generation changes.
//
//   Brad Whitlock, Tue Mar 20 09:36:05 PDT 2007
//   Added ability to name an object when it is created. Added a message for
//   how to get at the legend attributes.
//
//   Brad Whitlock, Mon Nov 12 16:00:45 PST 2007
//   Added Text3D.
//
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
visit_CreateAnnotationObject(PyObject *self, PyObject *args)
{
    const char *mName = "visit_CreateAnnotationObject: ";
    ENSURE_VIEWER_EXISTS();

    const char *annotType = 0, *annotName = 0;
    if (!PyArg_ParseTuple(args, "ss", &annotType, &annotName))
    {
        if (!PyArg_ParseTuple(args, "s", &annotType))
            return NULL;

        annotName = "";
        PyErr_Clear();
    }

    // See if it is an annotation type that we know about
    int annotTypeIndex;
    if(strcmp(annotType, "TimeSlider") == 0)
        annotTypeIndex = 2;
    else if(strcmp(annotType, "Text2D") == 0)
        annotTypeIndex = 0;
    else if(strcmp(annotType, "Text3D") == 0)
        annotTypeIndex = 1;
    else if(strcmp(annotType, "Line2D") == 0)
        annotTypeIndex = 3;
    else if(strcmp(annotType, "Image") == 0)
        annotTypeIndex = 7;
    else if(strcmp(annotType, "LegendAttributes") == 0)
    {
        VisItErrorFunc("Legends are created by plots and the legend attributes "
            "annotation objects associated with plot legends are created when "
            "plots are created. You may access the legend attributes by "
            "passing the name of a plot instance to GetAnnotationObject like "
            "this: \n\n"
            "    index = 0 # The index of the plot that you want in the plot list.\n"
            "    legend = GetAnnotationObject(GetPlotList().GetPlots(index).plotName)\n");
        return NULL;
    }
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
        debug1 << mName << "Telling the viewer to create a new " << annotType
               << " annotation object called \"" << annotName << "\"\n";
        GetViewerMethods()->AddAnnotationObject(annotTypeIndex, annotName);
    MUTEX_UNLOCK();
    int errorFlag = Synchronize();

    // If it was a success, then get the last annotation object in the
    // annotation object list and copy it into a Python wrapper class.
    PyObject *retval = 0;
    if(errorFlag == 0)
    {
        MUTEX_LOCK();
            AnnotationObjectList *aol = GetViewerState()->GetAnnotationObjectList();
            const AnnotationObject &newObject = aol->operator[](aol->GetNumAnnotations() - 1);

            debug1 << mName << "The viewer created a new " << annotType
                   << " annotation object called \""
                   << newObject.GetObjectName()
                   << "\". Wrapping that object for Python\n";

            //
            // Create a copy of the new annotation object that we'll keep in the
            // module's own annotation object list.
            //
            AnnotationObject *localCopy = new AnnotationObject(newObject);

            // Cache references based on the object name.
            AnnotationObjectRef ref;
            ref.object = localCopy;
            ref.refCount = 1;
            ref.index = localObjectMap.size();
          
            localObjectMap[newObject.GetObjectName()] = ref;

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
//   Brad Whitlock, Thu Mar 22 03:24:48 PDT 2007
//   Rewrote for new annotation object scheme.
//  
// ****************************************************************************

STATIC PyObject *
visit_GetAnnotationObject(PyObject *self, PyObject *args)
{
    const char *mName = "visit_GetAnnotationObject: ";
    ENSURE_VIEWER_EXISTS();

    bool useIndex = true;
    int annotIndex;
    char *annotName = NULL;
    if (!PyArg_ParseTuple(args, "i", &annotIndex))
    {
        if(!PyArg_ParseTuple(args, "s", &annotName))
            return NULL;
        PyErr_Clear();
        useIndex = false;
    }

    // Make sure the annotation object list is up to date.
    Synchronize();

    MUTEX_LOCK();

    PyObject *retval = NULL;

    if(useIndex)
    {
        if(annotIndex >= 0 && 
           annotIndex < GetViewerState()->GetAnnotationObjectList()->GetNumAnnotations())
        {
            debug1 << mName << "Look in the map for an object that has index equal to "
                   << annotIndex << endl;
            for(std::map<std::string, AnnotationObjectRef>::iterator pos = localObjectMap.begin();
                pos != localObjectMap.end(); ++pos)
            {
                if(pos->second.index == annotIndex)
                {
                    debug1 << mName << "Found object called "
                           << pos->second.object->GetObjectName()
                           << " with an index of " << annotIndex
                           << endl;

                    AnnotationObject *annot = pos->second.object;
                    retval = CreateAnnotationWrapper(annot);
    
                    // Increase the object's reference count.
                    if(retval != 0)
                        ++pos->second.refCount;
                    break;
                }
            }
        }
        else
        {
            const char *errMsg = "An invalid annotation object index was given!";
            debug1 << mName << errMsg << endl;
            VisItErrorFunc(errMsg);
        }
    }
    else
    {
        debug1 << mName << "Look in the map for an object called "
               << annotName << endl;
        std::map<std::string, AnnotationObjectRef>::iterator pos = 
            localObjectMap.find(annotName);
        if(pos != localObjectMap.end())
        {
            AnnotationObject *annot = pos->second.object;
            retval = CreateAnnotationWrapper(annot);
    
            // Increase the object's reference count.
            if(retval != 0)
                ++pos->second.refCount;
        }
        else
        {
            // The object was not in the map but see if we can add it.
            int index = GetViewerState()->GetAnnotationObjectList()->
                IndexForName(annotName);
            if(index != -1)
            {
                AnnotationObject &newObject = GetViewerState()->
                    GetAnnotationObjectList()->GetAnnotation(index);

                //
                // Create a copy of the new annotation object that we'll keep in the
                // module's own annotation object list.
                //
                AnnotationObject *localCopy = new AnnotationObject(newObject);

                retval = CreateAnnotationWrapper(localCopy);

                if(retval != 0)
                {
                    // Cache references based on the object name.
                    AnnotationObjectRef ref;
                    ref.object = localCopy;
                    ref.refCount = 1;
                    ref.index = localObjectMap.size();
         
                    localObjectMap[newObject.GetObjectName()] = ref;
                }
                else
                {
                    delete localCopy;
                    debug1 << mName << "CreateAnnotationWrapper returned 0!" << endl;
                }
            }
            else
            {
                char msg[400];
                SNPRINTF(msg, 400, "An unrecognized object name \"%s\" was requested.", annotName);
                VisItErrorFunc(msg);
            }
        }
    }

    MUTEX_UNLOCK();

    return retval;
}

// ****************************************************************************
// Function: visit_GetAnnotationObjectNames
//
// Purpose:
//   Returns a tuple containing the annotation object names.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 18:24:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_GetAnnotationObjectNames(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    Synchronize();
    MUTEX_LOCK();

    // Allocate a tuple the with enough entries to hold the plugin name list.
    PyObject *retval = PyTuple_New(GetViewerState()->GetAnnotationObjectList()->
        GetNumAnnotations());

    for(int i = 0;
        i < GetViewerState()->GetAnnotationObjectList()->GetNumAnnotations();
        ++i)
    {
        const AnnotationObject &annot = GetViewerState()->
            GetAnnotationObjectList()->GetAnnotation(i);
        PyObject *sval = PyString_FromString(annot.GetObjectName().c_str());
        if(sval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, sval);
    }

    MUTEX_UNLOCK();

    return retval;
}

// ****************************************************************************
// Function: visit_GetProcessAttributes
//
// Purpose: Gets Unix process attributes for a visit component by name 
//
// Programmer: Mark C. Miller 
// Creation:   Tuesday, January 18, 2005 
//   
// Modifications:
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change string literals to const char*'s.
//
// ****************************************************************************

STATIC PyObject *
visit_GetProcessAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    const char *componentName;
    const char *engineHostName;
    const char *engineDbName;
    if (!PyArg_ParseTuple(args, "sss", &componentName, &engineHostName,
                                       &engineDbName))
    {
        if (!PyArg_ParseTuple(args, "ss", &componentName, &engineHostName))
        {
            if (!PyArg_ParseTuple(args, "s", &componentName))
                return NULL;
            engineHostName = "localhost";
            PyErr_Clear();
        }
        engineDbName = "";
        PyErr_Clear();
    }

    PyObject *retval = PyProcessAttributes_New();
    ProcessAttributes *pa = PyProcessAttributes_FromPyObject(retval);

    if (strncmp(componentName, "cli", 3) != 0)
    {
        int id = VisItInit::ComponentNameToID(componentName);
        GetViewerMethods()->QueryProcessAttributes(id, engineHostName, engineDbName);

        Synchronize();

        // Copy the viewer proxy's window information into the return data structure.
        *pa = *(GetViewerState()->GetProcessAttributes());

    }
    else
    {
        doubleVector pids, ppids;
        stringVector hosts;

#if defined(_WIN32)
        int myPid = _getpid();
        int myPpid = -1;
#else
        int myPid = getpid();
        int myPpid = getppid();
#endif

        pids.push_back(myPid);
        ppids.push_back(myPpid);

        char myHost[256];
        gethostname(myHost, sizeof(myHost));
        hosts.push_back(myHost);

        pa->SetPids(pids);
        pa->SetPpids(ppids);
        pa->SetHosts(hosts);
        pa->SetIsParallel(false);
    }

    return retval;
}

// ****************************************************************************
// Function: visit_GetMeshManagementAttributes
//
// Purpose: Returns a mesh management attributes object with the current state
//          of the active window.
//
// Programmer: Mark C. Miller
// Creation:   November 6, 2005 
//   
// ****************************************************************************

STATIC PyObject *
visit_GetMeshManagementAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    NO_ARGUMENTS();

    PyObject *retval = PyMeshManagementAttributes_New();
    MeshManagementAttributes *ra = PyMeshManagementAttributes_FromPyObject(retval);

    // Copy the viewer proxy's window information into the return data structure.
    *ra = *(GetViewerState()->GetMeshManagementAttributes());

    return retval;
}

// ****************************************************************************
// Function: visit_SetMeshManagementAttributes
//
// Purpose: Tells the viewer to use the mesh management attributes we're
//          giving it.
//
// Programmer: Mark C. Miller 
// Creation:   November 5, 2005 
//
// ****************************************************************************

STATIC PyObject *
visit_SetMeshManagementAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *mmAtts = NULL;
    // Try and get the view pointer.
    if(!PyArg_ParseTuple(args,"O",&mmAtts))
    {
        VisItErrorFunc("SetMeshManagementAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyMeshManagementAttributes_Check(mmAtts))
    {
        VisItErrorFunc("Argument is not a MeshManagementAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        MeshManagementAttributes *ra = PyMeshManagementAttributes_FromPyObject(mmAtts);

        // Copy the object into the view attributes.
        *(GetViewerState()->GetMeshManagementAttributes()) = *ra;
        GetViewerState()->GetMeshManagementAttributes()->Notify();
        GetViewerMethods()->SetMeshManagementAttributes();
    MUTEX_UNLOCK();

    // Return the success value.
    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_SetDefaultMeshManagementAttributes
//
// Purpose: Tells the viewer to save the default mesh management attributes.
//
// Programmer: Mark C. Miller 
// Creation:   November 15, 2005
//
// ****************************************************************************

STATIC PyObject *
visit_SetDefaultMeshManagementAttributes(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    PyObject *mma = NULL;
    // Try and get the mma pointer.
    if(!PyArg_ParseTuple(args,"O",&mma))
    {
        VisItErrorFunc("SetDefaultMeshManagementAttributes: Cannot parse object!");
        return NULL;
    }
    if(!PyMeshManagementAttributes_Check(mma))
    {
        VisItErrorFunc("Argument is not a MeshManagementAttributes object");
        return NULL;
    }

    MUTEX_LOCK();
        MeshManagementAttributes *va = PyMeshManagementAttributes_FromPyObject(mma);

        // Copy the object into the view attributes.
        *(GetViewerState()->GetMeshManagementAttributes()) = *va;
        GetViewerState()->GetMeshManagementAttributes()->Notify();
        GetViewerMethods()->SetDefaultMeshManagementAttributes();
    MUTEX_UNLOCK();

    return IntReturnValue(Synchronize());
}

// ****************************************************************************
// Function: visit_Argv
//
// Purpose: 
//   Returns all arguments after the -s script.py argument.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 8 10:43:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

STATIC PyObject *
visit_Argv(PyObject *self, PyObject *args)
{
    NO_ARGUMENTS();
          
    // Allocate a tuple the with enough entries to hold the argv list.
    PyObject *retval = PyTuple_New(cli_argc_after_s);
    for(int i = 0; i < cli_argc_after_s; ++i)
    {
        PyObject *dval = PyString_FromString(cli_argv_after_s[i]);
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Function: PopulateMethodArgs
//
// Purpose: 
//   Recurses and populates the client method arguments from the python
//   object provided.
//
// Arguments:
//   m   : The client method object to populate.
//   obj : The python object to use to populate m.
//
// Returns:    false if there was an error.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 17:38:04 PST 2005
//
// Modifications:
//   
// ****************************************************************************

static bool
PopulateMethodArgs(ClientMethod *m, PyObject *obj)
{
    bool noErrors = true;

    if(PyInt_Check(obj))
    {
        m->AddArgument((int)PyInt_AS_LONG(obj));
    }
    else if(PyLong_Check(obj))
    {
        m->AddArgument((int)PyLong_AsLong(obj));
    }
    else if(PyFloat_Check(obj))
    {
        m->AddArgument((double)PyFloat_AS_DOUBLE(obj));
    }
    else if(PyString_Check(obj))
    {
        m->AddArgument(std::string(PyString_AS_STRING(obj)));
    }
    else if(PyTuple_Check(obj))
    {
        // Extract arguments from the tuple.
        for(int i = 0; i < PyTuple_Size(obj) && noErrors; ++i)
            noErrors = PopulateMethodArgs(m, PyTuple_GET_ITEM(obj, i));
    }
    else if(PyList_Check(obj))
    {
        // Extract arguments from the tuple.
        for(int i = 0; i < PyList_Size(obj) && noErrors; ++i)
            noErrors = PopulateMethodArgs(m, PyList_GET_ITEM(obj, i));
    }
    else
        noErrors = false;

    return noErrors;
}

// ****************************************************************************
// Function: visit_ClientMethod
//
// Purpose: 
//   Tells VisIt to execute a client method.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 17:06:44 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Apr 7 11:43:40 PDT 2008
//   Use a mutex to access ClientMethod so it can't be overwritten by thread 2.
//
// ****************************************************************************

STATIC PyObject *
visit_ClientMethod(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *name = 0;
    char *arg1 = 0;
    const char *CMError = "The tuple passed as the arguments to the"
                          "client method must contain only int, long, "
                          " float, tuples, or lists.";
    MUTEX_LOCK();
    ClientMethod *m = GetViewerState()->GetClientMethod();
    m->ClearArgs();

    if (!PyArg_ParseTuple(args, "s", &name))
    {
        PyObject *obj = 0;
        if(!PyArg_ParseTuple(args, "sO", &name, &obj))
        {
            MUTEX_UNLOCK();
            VisItErrorFunc(CMError);
            return NULL;
        }

        if(!PopulateMethodArgs(m, obj))
        {
            MUTEX_UNLOCK();
            VisItErrorFunc(CMError);
            return NULL;
        }

        PyErr_Clear();
    }

    // Send the client method to the viewer where it will be sent to
    // the other clients.
    m->SetMethodName(name);
    clientMethodObserver->SetUpdate(false);
    m->Notify();
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_RegisterMacro
//
// Purpose: 
//   Gives a function a name that can be called from the VisIt GUI.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:29:57 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Apr 7 11:43:40 PDT 2008
//   Use a mutex to access ClientMethod so it can't be overwritten by thread 2.
//   
// ****************************************************************************

STATIC PyObject *
visit_RegisterMacro(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *name = 0;
    PyObject *callback = 0;

    if (!PyArg_ParseTuple(args, "sO", &name, &callback))
        return NULL;

    if(callback == 0 || !PyCallable_Check(callback))
    {
        VisItErrorFunc("The object passed to RegisterMacro is not callable.");
        return NULL;
    }

    MUTEX_LOCK();
    std::string sname(name);
    bool found = macroFunctions.find(sname) != macroFunctions.end();

    // Now, send a client method to the GUI defining the macro button.
    if(found)
    {
        // We found a previous copy of an object. Let's decrement its
        // reference since we don't need it anymore.
        Py_DECREF(macroFunctions[sname]);
    }
    else
    {
        // We're caching the object for the first time. We need to increment
        // its reference count since we're keeping a pointer to the object.
        // This ensures that it won't go away behind our backs.
        Py_INCREF(callback);

        ClientMethod *m = GetViewerState()->GetClientMethod();
        m->SetMethodName("AddMacroButton");
        m->ClearArgs();
        m->AddArgument(name);
        clientMethodObserver->SetUpdate(false);
        m->Notify();
    }

    // Save the macro name to function mapping so we know how to call
    // the macro when the GUI wants to call it via a client method.
    macroFunctions[sname] = callback;
    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Function: visit_ExecuteMacro
//
// Purpose: 
//   Calls a function registered with RegisterMacro.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:29:57 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Apr 7 11:43:40 PDT 2008
//   Use a mutex to access macroFunctions. Guard against the Python callback
//   returning NULL somehow.
//   
// ****************************************************************************

STATIC PyObject *
visit_ExecuteMacro(PyObject *self, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();

    char *name = 0;
    PyObject *obj = 0;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    MUTEX_LOCK();
    PyObject *macro = NULL;
    std::string sname(name);
    if(macroFunctions.find(sname) != macroFunctions.end())
        macro = macroFunctions[sname];
    MUTEX_UNLOCK();

    if(macro == NULL)
    {
        VisItErrorFunc("An unrecognized macro name was provided.\n");
        return NULL;
    }

    // Call the function that the user named with the RegisterMacro function.
    PyObject *ret = PyObject_CallFunction(macro, NULL);

    if(ret == NULL)
    {
        debug1 << "The macro function " << name << " returned NULL" << endl;
        ret = Py_None;
        Py_INCREF(Py_None);
    }
 
    return ret;
}

// ****************************************************************************
// Function: visit_ClearMacro
//
// Purpose: 
//   Gives a function a name that can be called from the VisIt GUI.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:29:57 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Apr 7 11:50:24 PDT 2008
//   Access macroFunctions and ClientMethod with a mutex to protect against 
//   thread 2.
//
// ****************************************************************************

STATIC PyObject *
visit_ClearMacros(PyObject *self, PyObject *args)
{
    MUTEX_LOCK();

    // Decrement the reference counts for objects that we've stored.
    for(std::map<std::string,PyObject*>::iterator pos = macroFunctions.begin();
        pos != macroFunctions.end(); ++pos)
    {
        Py_DECREF(pos->second);
    }
    macroFunctions.clear();

    // Send a client method to the GUI telling it to clear its macro buttons.
    // Now, send a client method to the GUI defining the macro button.
    ClientMethod *m = GetViewerState()->GetClientMethod();
    m->SetMethodName("ClearMacroButtons");
    m->ClearArgs();
    clientMethodObserver->SetUpdate(false);
    m->Notify();

    MUTEX_UNLOCK();

    Py_INCREF(Py_None);
    return Py_None;
}

// ****************************************************************************
// Method: visit_SuppressMessages
//
// Purpose: set suppression level for status messages generated by viewer
//
// Programmer: Mark C. Miller 
// Creation:   Tue Nov 27 10:01:03 PST 2007 
//
// ****************************************************************************

STATIC PyObject *
visit_SuppressMessages(PyObject *self, PyObject *args)
{
    int suppressLevel = 4;
    if (!PyArg_ParseTuple(args, "i", &suppressLevel))
        return NULL;

    if(messageObserver)
        return PyInt_FromLong(long(messageObserver->SetSuppressLevel(suppressLevel)));

    return PyInt_FromLong(0);
}

// ****************************************************************************
// Method: ENSURE_CALLBACK_MANAGER_EXISTS
//
// Purpose: 
//   Creates the callback manager if it does not already exist.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 16:23:56 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 14:21:42 PDT 2008
//   Pass the viewer proxy into the callback manager.
//
// ****************************************************************************

static void
ENSURE_CALLBACK_MANAGER_EXISTS()
{
    //
    // Hook up the callback manager.
    //
    if(callbackMgr == 0)
    {
        MUTEX_LOCK();
        callbackMgr = new CallbackManager(GetViewerProxy());
        rpcCallbacks = new ViewerRPCCallbacks;
        RegisterCallbackHandlers(callbackMgr, GetViewerProxy(), rpcCallbacks);
        MUTEX_UNLOCK();
    }
}

// ****************************************************************************
// Method: visit_GetCallbackNames
//
// Purpose: 
//   Get the names of the available objects on which the user can set callback
//   functions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  1 16:53:02 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
visit_GetCallbackNames(PyObject *, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    ENSURE_CALLBACK_MANAGER_EXISTS();

    // Get the names of the callbacks that we can set.
    stringVector names;
    callbackMgr->GetCallbackNames(names);
    rpcCallbacks->GetCallbackNames(names);

    // Allocate a tuple the with enough entries to hold the plugin name list.
    PyObject *retval = PyTuple_New(names.size());
    for(int i = 0; i < names.size(); ++i)
    {
        PyObject *dval = PyString_FromString(names[i].c_str());
        if(dval == NULL)
            continue;
        PyTuple_SET_ITEM(retval, i, dval);
    }

    return retval;
}

// ****************************************************************************
// Method: visit_GetCallbackArgumentCount
//
// Purpose: 
//   Returns the number of arguments for a callback.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 15:31:32 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
visit_GetCallbackArgumentCount(PyObject *, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    ENSURE_CALLBACK_MANAGER_EXISTS();

    char *name = 0;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    // Check among the callback manager names.
    stringVector names;
    callbackMgr->GetCallbackNames(names);
    bool failed = true;
    int retval = 0;
    for(int i = 0; i < names.size(); ++i)
    {
        if(names[i] == name)
        {
            retval = 1;
            failed = false;
            break;
        }
    }
    // Check among the viewer rpc callback names.
    if(failed)
    {
        ViewerRPC::ViewerRPCType r;
        if(ViewerRPC::ViewerRPCType_FromString(name, r))
        {
            extern PyObject *args_ViewerRPC(ViewerRPC *rpc);

            // Get the number of argments.
            ViewerRPC rpc;
            rpc.SetRPCType(r);
            PyObject *args = args_ViewerRPC(&rpc);
            if(args == 0)
                retval = 0;
            else if(PyTuple_Check(args))
                retval = PyTuple_Size(args);
            else if(PyViewerRPC_Check(args))
                retval = 1;
            else
                retval = 0;

            // Delete the args.
            if(args != 0)
                Py_DECREF(args);

            failed = false;
        }
    }

    if(failed)
    {
        VisItErrorFunc("An invalid callback name was provided.");
        return NULL;
    }

    return PyLong_FromLong((long)retval);
}

// ****************************************************************************
// Method: visit_RegisterCallback
//
// Purpose: 
//   Registers a user-defined callback with a named state object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  1 16:53:02 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Feb  6 10:17:24 PST 2008
//   Added optional callback data and made it possible to unregister a 
//   callback by passing just the name.
//
// ****************************************************************************

PyObject *
visit_RegisterCallback(PyObject *, PyObject *args)
{
    ENSURE_VIEWER_EXISTS();
    ENSURE_CALLBACK_MANAGER_EXISTS();

    char *name = 0;
    PyObject *callback = 0, *callback_data = 0;
    if (!PyArg_ParseTuple(args, "sOO", &name, &callback, &callback_data))
    {
        if (PyArg_ParseTuple(args, "sO", &name, &callback))
            callback_data = 0;
        else
        {
            if(PyArg_ParseTuple(args, "s", &name))
                callback = callback_data = 0;
            else
                return NULL;
        }
        PyErr_Clear();
    }
    if(callback != 0 && !PyCallable_Check(callback))
    {
        VisItErrorFunc("The object passed to RegisterCallback is not callable.");
        return NULL;
    }

    // Try and register a ViewerRPC callback.
    bool failed = !rpcCallbacks->RegisterCallback(name, callback, callback_data);

    // Try and register a state callback instead.
    if(failed)
    {
        // Get the names of the callbacks that we can set.
        stringVector names;
        callbackMgr->GetCallbackNames(names);
        failed = true;
        for(int i = 0; i < names.size(); ++i)
        {
            if(names[i] == name)
            {
                callbackMgr->RegisterCallback(name, callback, callback_data);
                failed = false;
                break;
            }
        }
    }
    if(failed)
    {
        VisItErrorFunc("An invalid callback name was provided.");
        return NULL;
    }
    else
    {
        // Tell the callback manager that it can process callbacks now.
        callbackMgr->WorkAllowed();
    }

    return PyLong_FromLong(1L);
}

// ****************************************************************************
// Function: visit_exec_client_method
//
// Purpose:
//   This method is a thread callback function whose sole purpose is to
//   execute the method stored in the ClientMethod object that is passed in.
//
// Notes:      If the method being asked for is implemented here, try and 
//             execute it.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 16:58:15 PST 2005
//
// Modifications:
//    Jeremy Meredith, Thu Jul  7 10:05:26 PDT 2005
//    On 64-bit AIX, one cannot convert a void* to an int.  I instead compare
//    the second void* pointer with NULL to construct the boolean result.
//
//    Brad Whitlock, Fri Feb  1 16:58:23 PST 2008
//    Moved interpreter locking to functions that we can call elsewhere.
//
// ****************************************************************************

#if defined(_WIN32)
static DWORD WINAPI
visit_exec_client_method(LPVOID data)
#else
static void *
visit_exec_client_method(void *data)
#endif
{
    void **cbData = (void **)data;
    ClientMethod *m = (ClientMethod *)cbData[0];
    bool acquireLock = cbData[1] ? true : false;

    PyThreadState *myThreadState = 0;

    if(acquireLock)
        myThreadState = VisItLockPythonInterpreter();

    // Execute the method
    if(m->GetMethodName() == "Quit")
    {
        // Tell the 2nd thread to quit.
        keepGoing = false;
        // Make the interpreter quit.
        viewerInitiatedQuit = true;
        PyRun_SimpleString("import sys; sys.exit(0)");
    }
    else if(m->GetMethodName() == "Interpret")
    {
        // Interpret all of the strings stored in the method arguments.
        const stringVector &code = m->GetStringArgs();
        for(int i = 0; i < code.size(); ++i)
        {
            int len = code[i].size() + 1;
            char *buf = new char[len];
            strcpy(buf, code[i].c_str());
            PyRun_SimpleString(buf);
            delete [] buf;
        }
    }

    if(acquireLock)
        VisItUnlockPythonInterpreter(myThreadState);

    delete m;
    delete [] cbData;

    return NULL;
}

// ****************************************************************************
// Function: ExecuteClientMethodHelper
//
// Purpose:
//   This method is called when the clientMethodObserver gets client method
//   data from the viewer.
//
// Notes:      We don't ever need MUTEX_LOCK here when accessing the
//             cachedClientMethods vector because the 2nd thread always has
//             MUTEX_LOCK locked when processing its input from the viewer,
//             which is how we got here.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 16:58:15 PST 2005
//
// Modifications:
//   
// ****************************************************************************

static void
ExecuteClientMethodHelper(Subject *subj, void *)
{
    ClientMethod *method = (ClientMethod *)subj;
    debug1 << "Received a " << method->GetMethodName().c_str()
           << " client method. ";

    if(!clientMethodsAllowed)
    {
        debug1 << "Save it for later." << endl;
        cachedClientMethods.push_back(new ClientMethod(*method));
    }
    else
    {
        debug1 << "Execute it now." << endl;
        ExecuteClientMethod(method, true);
    }
}

// ****************************************************************************
// Function: ExecuteClientMethod
//
// Purpose:
//   This method executes client methods and optionally spawns a new thread 
//   to execute them.
//
// Notes:      If the method being asked for is implemented here, try and 
//             execute it. We may execute the method on a new thread because
//             this method is usually called by the 2nd thread, which is the
//             messaging thread and it must return so it can listen for 
//             synchronizes and other data from the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 28 11:01:02 PDT 2005
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 11:04:38 PDT 2005
//   Removed some code to disable updates on Xfer when sending back the
//   client information to the viewer. This was a holdover from an intermediate
//   implementation and it was causing synchronization failures when we
//   process a client method during the Launch method. I also made the
//   Quit method be processed on the listener thread so we don't have problems
//   with the 2nd thread coring when Python gets shut down by a worker thread.
//
//   Brad Whitlock, Wed Jun 29 15:02:01 PST 2005
//   Put back the code to enable/disable updates on Xfer but made sure that it
//   only takes effect when this routine is run from the listener thread. The
//   absence of that code prevented client info from making it to the viewer
//   when we execute this routine on the listener thread. We only do that
//   code on the listener thread so we don't mess up synchronizes on the main
//   thread during startup.
//
//   Brad Whitlock, Fri Jan 6 12:02:03 PDT 2006
//   Added support for recording macros.
//
//   Hank Childs, Thu Oct 25 08:52:27 PDT 2007
//   Add preprocessor directives for the case when THREADS is not defined.
//
// ****************************************************************************

static void
ExecuteClientMethod(ClientMethod *method, bool onNewThread)
{
    debug1 << "ExecuteClientMethod: " << method->GetMethodName().c_str()
           << endl;

    if(method->GetMethodName() == "_QueryClientInformation")
    {
        // The viewer uses this method to discover information about the GUI.
        ClientInformation *info = GetViewerState()->GetClientInformation();
        info->SetClientName("cli");
        info->ClearMethods();

        // Populate the method names and prototypes that the CLI supports
        // but don't advertise _QueryClientInformation.
        info->DeclareMethod("Quit", "");
        info->DeclareMethod("Interpret",  "s");
        info->DeclareMethod("Interrupt",  "");
        info->DeclareMethod("MacroStart", "");
        info->DeclareMethod("MacroPause", "");
        info->DeclareMethod("MacroEnd",   "");
        info->SelectAll();

        // If onNewThread is true then we got into this method on the 2nd
        // thread, which means that xfer's update will be set to false. That
        // means that calling Notify on the client information would not make
        // xfer send it to the viewer. To combat this problem, we set xfer's
        // update to true temporarily so we can send the object to the viewer.
        // We only do it on the 2nd thread because if this method is called 
        // from the first thread, we did not arrive here from xfer and
        // turning off its updates messes up Synchronize.
        if(onNewThread)
           GetViewerProxy()->SetXferUpdate(true);
        info->Notify();
        if(onNewThread)
           GetViewerProxy()->SetXferUpdate(false);
    }
    else if(method->GetMethodName() == "Interrupt")
    {
        interruptScript = true;
    }
    else if(method->GetMethodName() == "Quit")
    {
        // Execute the Quit method here on the 2nd thread. Make it get
        // the interpreter lock by calling it using visit_exec_client_method.
        void **cbData = new void *[2];
        ClientMethod *m = new ClientMethod(*method);
        cbData[0] = (void *)m;
        cbData[1] = (void *)1;
        visit_exec_client_method(cbData);
    }
    else if(method->GetMethodName() == "MacroStart")
    {
        Macro_SetString("");
        Macro_SetRecord(true);
    }
    else if(method->GetMethodName() == "MacroPause")
    {
        Macro_SetRecord(!Macro_GetRecord());
    }
    else if(method->GetMethodName() == "MacroEnd")
    {
        // Send the macro to the clients.
        if(Macro_GetString().size() > 0)
        {
            // If onNewThread is true then we got into this method on the 2nd
            // thread, which means that xfer's update will be set to false. That
            // means that calling Notify on the client information would not make
            // xfer send it to the viewer. To combat this problem, we set xfer's
            // update to true temporarily so we can send the object to the viewer.
            // We only do it on the 2nd thread because if this method is called 
            // from the first thread, we did not arrive here from xfer and
            // turning off its updates messes up Synchronize.
            if(onNewThread)
               GetViewerProxy()->SetXferUpdate(true);

            // We don't want to get here re-entrantly so disable the client method
            // observer temporarily.
            clientMethodObserver->SetUpdate(false);

            stringVector args;
            args.push_back(Macro_GetString());
            ClientMethod *newM = GetViewerState()->GetClientMethod();
            newM->ClearArgs();
            newM->SetMethodName("AcceptRecordedMacro");
            newM->SetStringArgs(args);
            newM->Notify();

            if(onNewThread)
               GetViewerProxy()->SetXferUpdate(false);
        }

        Macro_SetString("");
        Macro_SetRecord(false);
    }
    else
    {
        // Determine whether the method is supported by this client.
        int okay = GetViewerProxy()->MethodRequestHasRequiredInformation();
     
        if(okay == 0)
        {
            debug1 << "Client method " << method->GetMethodName().c_str()
                   << " is not supported by the CLI." << endl;
        }
        else if(okay == 1)
        {
            debug1 << "Client method " << method->GetMethodName().c_str()
                   << " is supported by the CLI but not enough information "
                      "was passed in the method request." << endl;
        }
        else
        {
            //
            // We're going to interpret the Python code. We need another 
            // thread so this thread can get back to reading output from the
            // viewer.
            //
            void **cbData = new void *[2];
            ClientMethod *m = new ClientMethod(*method);
            cbData[0] = (void *)m;
            cbData[1] = (void *)(onNewThread?1l:0);
            if(onNewThread)
            {
#ifdef THREADS
#if defined(_WIN32)
                // Create the thread with the WIN32 API.
                DWORD Id;
                if(CreateThread(0, 0, visit_exec_client_method, (LPVOID)cbData, 0, &Id) == INVALID_HANDLE_VALUE)
                {
                    delete m;
                    delete [] cbData;
                    fprintf(stderr, "VisIt: Error - Could not create work thread to "
                            "execute %s client method.\n", m->GetMethodName().c_str());
                }
#else
                // Create the thread using PThreads.
                pthread_t tid;
                if(pthread_create(&tid, &thread_atts, visit_exec_client_method, (void*)cbData) == -1)
                {
                    delete m;
                    delete [] cbData;
                    fprintf(stderr, "VisIt: Error - Could not create work thread to "
                            "execute %s client method.\n", m->GetMethodName().c_str());
                }
#endif
#endif
            }
            else
            {
                // Execute the method on the current thread.
                visit_exec_client_method(cbData);
            }
        }
    }
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
//   Kathleen Bonnell, Wed Sep  8 10:33:24 PDT 2004 
//   Removed WorldPick and WorldNodePick, as they are unnecessary and handled
//   by Pick and NodePick. 
//
//   Jeremy Meredith, Fri Oct 29 16:47:57 PDT 2004
//   Added methods to support lighting.
//
//   Kathleen Bonnell, Thu Dec 16 17:31:10 PST 2004
//   Added PickByGlobalZone and PickByGlobalNode. 
//
//   Kathleen Bonnell, Thu Jan  6 11:06:29 PST 2005 
//   Added GetViewCurve. 
//
//   Brad Whitlock, Tue Mar 8 16:43:51 PST 2005
//   I removed some deprecated functions and added TimeSliderSetState, which
//   is the same thing as SetTimeSliderState but might be easier to remember.
//
//   Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//   Added GetProcessAttributes
//
//   Brad Whitlock, Thu Mar 17 10:17:32 PDT 2005
//   Added ToggleLockTools, GetDatabaseCorrelation, GetDatabaseCorrelationNames.
//
//   Brad Whitlock, Wed May 4 10:45:53 PDT 2005
//   Added OpenClient and OpenGUI.
//
//   Hank Childs, Mon Jun 13 11:23:41 PDT 2005
//   Added docstrings.
//
//   Hank Childs, Thu Jul 21 16:28:48 PDT 2005
//   Added DefineArrayExpression.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Brad Whitlock, Thu Nov 17 17:03:13 PST 2005
//   Added ResizeWindow, MoveWindow, MoveAndResizeWindow.
//
//   Brad Whitlock, Tue Mar 7 16:39:38 PST 2006
//   Added RedoView.
//
//   Kathleen Bonnell, Tue May  9 15:45:04 PDT 2006 
//   Added 'PointPick', an alias for 'NodePick'.
//
//   Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006 
//   Added GetOutputArray.
//
//   Kathleen Bonnell, Tue Aug  1 09:13:45 PDT 2006 
//   Added DefineCurveExpression.
//
//   Brad Whitlock, Mon Sep 18 11:38:05 PDT 2006
//   Added SetColorTexturingEnabled.
//
//   Brad Whitlock, Fri Nov 10 11:20:08 PDT 2006
//   Added RestoreSessionWithDifferentSources.
//
//   Brad Whitlock, Fri Mar 9 17:23:33 PST 2007
//   Added GetMetaData.
//
//   Brad Whitlock, Thu Jun 14 16:43:26 PST 2007
//   Added ClearMacros, RegisterMacro, ExecuteMacros
//
//   Brad Whitlock, Tue Jul 3 16:28:39 PST 2007
//   Added ColorTable function.
//
//   Cyrus Harrison, Wed Sep 19 08:43:42 PDT 2007
//   Added SetQueryFloatFormat function.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added methods to control automatic creation of MeshQuality and
//   TimeDerivative expressions. 
//
//   Jeremy Meredith, Fri Oct 12 10:32:05 EDT 2007
//   Added GetExportOptions.
//
//   Mark C. Miller, Tue Nov 27 10:44:29 PST 2007
//   Added SuppressMessages
//
//   Cyrus Harrison, Wed Nov 28 11:50:54 PST 2007
//   Added SetCreateVectorMagnitudeExpressions
//
//   Brad Whitlock, Wed Dec 12 15:18:39 PST 2007
//   Added Set/GetAnimationAttributes, which I thought I added a long time ago.
//
//   Cyrus Harrison, Mon Dec 17 14:49:25 PST 2007
//   Added GetQueryOutputXML() and GetQueryOutputObject() 
//
//   Jeremy Meredith, Wed Jan 23 15:27:20 EST 2008
//   Added Get/SetDefaultFileOpenOptions.
//
//   Brad Whitlock, Fri Feb  1 16:54:17 PST 2008
//   Added GetCallbackNames, RegisterCallback, GetCallbackArgumentCount.
//
//   Jeremy Meredith, Mon Feb  4 13:41:43 EST 2008
//   Added Get/SetViewAxisArray.
//
//   Brad Whitlock, Fri Feb 15 11:21:24 PST 2008
//   Added GetPlotOptions, GetOperatorOptions.
//
//   Jeremy Meredith, Mon Jan  5 10:21:05 EST 2009
//   Added LoadAttribute, SaveAttribute
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
    AddMethod("AddArgument", visit_AddArgument, visit_AddArgument_doc);
    AddMethod("Close",  visit_Close, visit_Close_doc);
    AddMethod("Launch", visit_Launch, visit_Launch_doc);
    AddMethod("LaunchNowin", visit_LaunchNowin, visit_Launch_doc);
    AddMethod("LocalNameSpace", visit_LocalNameSpace,visit_LocalNamespace_doc);
    AddMethod("GetDebugLevel", visit_GetDebugLevel, visit_DebugLevel_doc);
    AddMethod("GetLastError", visit_GetLastError, visit_GetLastError_doc);
    AddMethod("SetDebugLevel", visit_SetDebugLevel, visit_DebugLevel_doc);
    AddMethod("Version", visit_Version, visit_Version_doc);
    AddMethod("LongFileName", visit_LongFileName, visit_LongFileName_doc);

    //
    // Viewer proxy methods.
    //
    AddMethod("ActivateDatabase", visit_ActivateDatabase, 
                                                   visit_ActivateDatabase_doc);
    AddMethod("AddOperator", visit_AddOperator, visit_AddOperator_doc);
    AddMethod("AddPlot", visit_AddPlot, visit_AddPlot_doc);
    AddMethod("AddWindow",  visit_AddWindow, visit_AddWindow_doc);
    AddMethod("AlterDatabaseCorrelation", visit_AlterDatabaseCorrelation, 
                                           visit_AlterDatabaseCorrelation_doc);
    AddMethod("AnimationSetNFrames", visit_AnimationSetNFrames, NULL);
    AddMethod("ChangeActivePlotsVar", visit_ChangeActivePlotsVar, 
                                               visit_ChangeActivePlotsVar_doc);
    AddMethod("CheckForNewStates", visit_CheckForNewStates, 
                                                  visit_CheckForNewStates_doc);
    AddMethod("ChooseCenterOfRotation",  visit_ChooseCenterOfRotation, 
                                             visit_ChooseCenterOfRotation_doc);
    AddMethod("ClearAllWindows", visit_ClearAllWindows, visit_Clear_doc);
    AddMethod("ClearCache", visit_ClearCache, visit_ClearCache_doc);
    AddMethod("ClearCacheForAllEngines", visit_ClearCacheForAllEngines,
                                                         visit_ClearCache_doc);
    AddMethod("ClearPickPoints", visit_ClearPickPoints,
                                                    visit_ClearPickPoints_doc);
    AddMethod("ClearReferenceLines", visit_ClearReferenceLines,
                                                visit_ClearReferenceLines_doc);
    AddMethod("ClearViewKeyframes", visit_ClearViewKeyframes,
                                                 visit_ClearViewKeyframes_doc);
    AddMethod("ClearWindow", visit_ClearWindow, visit_Clear_doc);
    AddMethod("ClientMethod", visit_ClientMethod);
    AddMethod("CloneWindow",  visit_CloneWindow, visit_CloneWindow_doc);
    AddMethod("CloseComputeEngine", visit_CloseComputeEngine, 
                                                 visit_CloseComputeEngine_doc);
    AddMethod("CloseDatabase", visit_CloseDatabase, visit_CloseDatabase_doc);
    AddMethod("ConstructDDF", visit_ConstructDDF, visit_ConstructDDF_doc);
    AddMethod("CopyAnnotationsToWindow", visit_CopyAnnotationsToWindow,
                                                               visit_Copy_doc);
    AddMethod("CopyLightingToWindow", visit_CopyLightingToWindow,
                                                               visit_Copy_doc);
    AddMethod("CopyPlotsToWindow", visit_CopyPlotsToWindow, visit_Copy_doc);
    AddMethod("CopyViewToWindow", visit_CopyViewToWindow, visit_Copy_doc);
    AddMethod("CreateAnnotationObject", visit_CreateAnnotationObject,
                                             visit_CreateAnnotationObject_doc);
    AddMethod("CreateDatabaseCorrelation", visit_CreateDatabaseCorrelation,
                                          visit_CreateDatabaseCorrelation_doc);
    AddMethod("DefineArrayExpression", visit_DefineArrayExpression,
                                               visit_DefineExpression_doc);
    AddMethod("DefineCurveExpression", visit_DefineCurveExpression,
                                               visit_DefineExpression_doc);
    AddMethod("DefineMeshExpression", visit_DefineMeshExpression,
                                               visit_DefineExpression_doc);
    AddMethod("DefineMaterialExpression", visit_DefineMaterialExpression,
                                           visit_DefineExpression_doc);
    AddMethod("DefineScalarExpression", visit_DefineScalarExpression,
                                             visit_DefineExpression_doc);
    AddMethod("DefineSpeciesExpression", visit_DefineSpeciesExpression,
                                            visit_DefineExpression_doc);
    AddMethod("DefineTensorExpression", visit_DefineTensorExpression,
                                             visit_DefineExpression_doc);
    AddMethod("DefineVectorExpression", visit_DefineVectorExpression,
                                             visit_DefineExpression_doc);
    AddMethod("DeleteDatabaseCorrelation", visit_DeleteDatabaseCorrelation,
                                          visit_DeleteDatabaseCorrelation_doc);
    AddMethod("DeleteExpression", visit_DeleteExpression,
                                                   visit_DeleteExpression_doc);
    AddMethod("DeIconifyAllWindows", visit_DeIconifyAllWindows,
                                                visit_DeIconifyAllWindows_doc);
    AddMethod("DeleteActivePlots", visit_DeleteActivePlots,
                                                        visit_DeletePlots_doc);
    AddMethod("DeleteAllPlots", visit_DeleteAllPlots,visit_DeletePlots_doc);
    AddMethod("DeletePlotDatabaseKeyframe", visit_DeletePlotDatabaseKeyframe,
                                         visit_DeletePlotDatabaseKeyframe_doc);
    AddMethod("DeletePlotKeyframe", visit_DeletePlotKeyframe,
                                                 visit_DeletePlotKeyframe_doc);
    AddMethod("DeleteViewKeyframe", visit_DeleteViewKeyframe,
                                                 visit_DeleteViewKeyframe_doc);
    AddMethod("DeleteWindow", visit_DeleteWindow, visit_DeleteWindow_doc);
    AddMethod("DemoteOperator", visit_DemoteOperator,visit_DemoteOperator_doc);
    AddMethod("DisableRedraw", visit_DisableRedraw, visit_DisableRedraw_doc);
    AddMethod("DrawPlots", visit_DrawPlots, visit_DrawPlots_doc);
    AddMethod("EnableTool", visit_EnableTool, visit_EnableTool_doc);
    AddMethod("ExportDatabase", visit_ExportDatabase, visit_ExportDatabase_doc);
    AddMethod("GetAnimationAttributes", visit_GetAnimationAttributes,
                                                NULL /* DOCUMENT ME*/);
    AddMethod("GetAnimationTimeout", visit_GetAnimationTimeout,
                                                visit_GetAnimationTimeout_doc);
    AddMethod("GetAnnotationObject", visit_GetAnnotationObject,
                                                visit_GetAnnotationObject_doc);
    AddMethod("GetAnnotationObjectNames", visit_GetAnnotationObjectNames,
                                                NULL);

    AddMethod("GetLocalHostName", visit_GetLocalHostName,
                                                       visit_GetLocalName_doc);
    AddMethod("GetLocalUserName", visit_GetLocalUserName,
                                                       visit_GetLocalName_doc);
    AddMethod("GetSaveWindowAttributes", visit_GetSaveWindowAttributes,
                                            visit_GetSaveWindowAttributes_doc);
    AddMethod("GetViewAxisArray", visit_GetViewAxisArray, visit_GetView_doc);
    AddMethod("GetViewCurve", visit_GetViewCurve, visit_GetView_doc);
    AddMethod("GetView2D", visit_GetView2D, visit_GetView_doc);
    AddMethod("GetView3D", visit_GetView3D, visit_GetView_doc);
    AddMethod("GetAnnotationAttributes", visit_GetAnnotationAttributes,
                                            visit_GetAnnotationAttributes_doc);
    AddMethod("GetDatabaseCorrelation", visit_GetDatabaseCorrelation, NULL);
    AddMethod("GetDatabaseCorrelationNames", visit_GetDatabaseCorrelationNames,
                                                                         NULL);
    AddMethod("GetDatabaseNStates", visit_GetDatabaseNStates,
                                                 visit_GetDatabaseNStates_doc);
    AddMethod("GetDefaultFileOpenOptions", visit_GetDefaultFileOpenOptions,
                                          visit_GetDefaultFileOpenOptions_doc);
    AddMethod("GetEngineList", visit_GetEngineList, visit_GetEngineList_doc);
    AddMethod("GetExportOptions", visit_GetExportOptions, NULL);
    AddMethod("GetGlobalAttributes", visit_GetGlobalAttributes,
                                                visit_GetGlobalAttributes_doc);
    AddMethod("GetGlobalLineoutAttributes", visit_GetGlobalLineoutAttributes,
                                         visit_GetGlobalLineoutAttributes_doc);
    AddMethod("GetInteractorAttributes", visit_GetInteractorAttributes,
                                            visit_GetInteractorAttributes_doc);
    AddMethod("GetKeyframeAttributes", visit_GetKeyframeAttributes,
                                              visit_GetKeyframeAttributes_doc);
    AddMethod("GetMaterialAttributes", visit_GetMaterialAttributes,
                                              visit_GetMaterialAttributes_doc);
    AddMethod("GetMeshManagementAttributes", visit_GetMeshManagementAttributes,
                                              visit_GetMeshManagementAttributes_doc);
    AddMethod("GetPickAttributes", visit_GetPickAttributes,
                                                  visit_GetPickAttributes_doc);
    AddMethod("GetPickOutput", visit_GetPickOutput, visit_GetPickOutput_doc);
    AddMethod("GetPipelineCachingMode", visit_GetPipelineCachingMode,
                                             visit_GetPipelineCachingMode_doc);
    AddMethod("GetProcessAttributes", visit_GetProcessAttributes, NULL);
    AddMethod("GetQueryOutputString", visit_GetQueryOutputString,
                                                     visit_GetQueryOutput_doc);
    AddMethod("GetQueryOutputValue", visit_GetQueryOutputValue,
                                                     visit_GetQueryOutput_doc);
    AddMethod("GetQueryOutputXML", visit_GetQueryOutputXML,
                                                     visit_GetQueryOutput_doc);
    AddMethod("GetQueryOutputObject", visit_GetQueryOutputObject,
                                                     visit_GetQueryOutput_doc);
    
    AddMethod("GetOutputArray", visit_GetOutputArray,
                                                     visit_GetOutputArray_doc);
    AddMethod("GetRenderingAttributes", visit_GetRenderingAttributes,
                                             visit_GetRenderingAttributes_doc);
    AddMethod("GetQueryOverTimeAttributes", visit_GetQueryOverTimeAttributes,
                                         visit_GetQueryOverTimeAttributes_doc);
    AddMethod("GetWindowInformation", visit_GetWindowInformation,
                                               visit_GetWindowInformation_doc);
    AddMethod("HideActivePlots", visit_HideActivePlots,
                                                    visit_HideActivePlots_doc);
    AddMethod("HideToolbars", visit_HideToolbars, visit_HideToolbars_doc);
    AddMethod("IconifyAllWindows", visit_IconifyAllWindows,
                                                  visit_IconifyAllWindows_doc);
    AddMethod("InvertBackgroundColor", visit_InvertBackgroundColor,
                                              visit_InvertBackgroundColor_doc);
    AddMethod("Lineout", visit_Lineout, visit_Lineout_doc);
    AddMethod("MovePlotDatabaseKeyframe", visit_MovePlotDatabaseKeyframe,
                                           visit_MovePlotDatabaseKeyframe_doc);
    AddMethod("MovePlotKeyframe", visit_MovePlotKeyframe,
                                                   visit_MovePlotKeyframe_doc);
    AddMethod("MoveViewKeyframe", visit_MoveViewKeyframe,
                                                   visit_MoveViewKeyframe_doc);
    AddMethod("MoveWindow", visit_MoveWindow, visit_MoveWindow_doc);
    AddMethod("MoveAndResizeWindow", visit_MoveAndResizeWindow, visit_MoveAndResizeWindow_doc);
    AddMethod("NodePick", visit_NodePick, visit_NodePick_doc);
    AddMethod("OpenDatabase", visit_OpenDatabase, visit_OpenDatabase_doc);
    AddMethod("OpenClient", visit_OpenClient);
    AddMethod("OpenComputeEngine", visit_OpenComputeEngine,
                                                  visit_OpenComputeEngine_doc);
    AddMethod("OpenGUI", visit_OpenGUI);
    AddMethod("OpenMDServer", visit_OpenMDServer, visit_OpenMDServer_doc);
    AddMethod("OverlayDatabase", visit_OverlayDatabase, 
                                                    visit_OverlayDatabase_doc);
    AddMethod("Pick", visit_Pick, visit_Pick_doc);
    AddMethod("PickByNode", visit_PickByNode, visit_PickByNode_doc);
    AddMethod("PickByZone", visit_PickByZone, visit_PickByZone_doc);
    AddMethod("PickByGlobalNode", visit_PickByGlobalNode, 
                                                   visit_PickByGlobalNode_doc);
    AddMethod("PickByGlobalZone", visit_PickByGlobalZone,
                                                   visit_PickByGlobalZone_doc);
    AddMethod("PointPick", visit_NodePick, visit_NodePick_doc);
    AddMethod("PrintWindow", visit_PrintWindow, visit_PrintWindow_doc);
    AddMethod("PromoteOperator", visit_PromoteOperator,
                                                    visit_PromoteOperator_doc);
    AddMethod("Query", visit_Query, visit_Query_doc);
    AddMethod("QueryOverTime", visit_QueryOverTime, visit_QueryOverTime_doc);
    AddMethod("RecenterView", visit_RecenterView, visit_RecenterView_doc);
    AddMethod("RedrawWindow", visit_RedrawWindow, visit_RedrawWindow_doc);
    AddMethod("RemoveAllOperators", visit_RemoveAllOperators,
                                                     visit_RemoveOperator_doc);
    AddMethod("RemoveLastOperator", visit_RemoveLastOperator,
                                                     visit_RemoveOperator_doc);
    AddMethod("RemoveOperator", visit_RemoveOperator,visit_RemoveOperator_doc);
    AddMethod("ReOpenDatabase", visit_ReOpenDatabase,visit_ReOpenDatabase_doc);
    AddMethod("ReplaceDatabase", visit_ReplaceDatabase,
                                                    visit_ReplaceDatabase_doc);
    AddMethod("ResetLineoutColor", visit_ResetLineoutColor,
                                                  visit_ResetLineoutColor_doc);
    AddMethod("ResetOperatorOptions", visit_ResetOperatorOptions,
                                               visit_ResetOperatorOptions_doc);
    AddMethod("ResetPickAttributes", visit_ResetPickAttributes);
    AddMethod("ResetPickLetter", visit_ResetPickLetter, 
                                                    visit_ResetPickLetter_doc);
    AddMethod("ResetPlotOptions", visit_ResetPlotOptions, 
                                                   visit_ResetPlotOptions_doc);
    AddMethod("ResetQueryOverTimeAttributes", visit_ResetQueryOverTimeAttributes);
    AddMethod("ResetView", visit_ResetView, visit_ResetView_doc);
    AddMethod("ResizeWindow", visit_ResizeWindow, visit_ResizeWindow_doc);
    AddMethod("RestoreSession", visit_RestoreSession,visit_RestoreSession_doc);
    AddMethod("RestoreSessionWithDifferentSources", 
              visit_RestoreSessionWithDifferentSources,
              visit_RestoreSession_doc);
    AddMethod("SaveSession", visit_SaveSession, visit_SaveSession_doc);
    AddMethod("SaveWindow", visit_SaveWindow, visit_SaveWindow_doc);
    AddMethod("SetActivePlots", visit_SetActivePlots,visit_SetActivePlots_doc);
    AddMethod("SetActiveTimeSlider", visit_SetActiveTimeSlider, 
                                                visit_SetActiveTimeSlider_doc);
    AddMethod("SetActiveWindow", visit_SetActiveWindow, 
                                                    visit_SetActiveWindow_doc);
    AddMethod("SetAnimationAttributes", visit_SetAnimationAttributes,
                                                NULL/* DOCUMENT ME*/);
    AddMethod("SetAnimationTimeout", visit_SetAnimationTimeout,
                                                visit_SetAnimationTimeout_doc);
    AddMethod("SetAnnotationAttributes", visit_SetAnnotationAttributes,
                                            visit_SetAnnotationAttributes_doc);
    AddMethod("SetCreateMeshQualityExpressions", 
               visit_SetCreateMeshQualityExpressions,
               visit_SetCreateMeshQualityExpressions_doc);
    AddMethod("SetCreateTimeDerivativeExpressions", 
               visit_SetCreateTimeDerivativeExpressions,
               visit_SetCreateTimeDerivativeExpressions_doc);
    AddMethod("SetCreateVectorMagnitudeExpressions", 
               visit_SetCreateVectorMagnitudeExpressions,
               visit_SetCreateVectorMagnitudeExpressions_doc);
    AddMethod("SetCenterOfRotation", visit_SetCenterOfRotation,
                                                visit_SetCenterOfRotation_doc);
    AddMethod("SetCloneWindowOnFirstRef", visit_SetCloneWindowOnFirstRef);
    AddMethod("SetDefaultAnnotationAttributes", visit_SetDefaultAnnotationAttributes,
                                            visit_SetAnnotationAttributes_doc);
    AddMethod("SetDefaultFileOpenOptions", visit_SetDefaultFileOpenOptions,
                                          visit_SetDefaultFileOpenOptions_doc);
    AddMethod("SetDefaultInteractorAttributes", visit_SetDefaultInteractorAttributes,
                                            visit_SetInteractorAttributes_doc);
    AddMethod("SetDefaultMaterialAttributes", visit_SetDefaultMaterialAttributes,
                                            visit_SetMaterialAttributes_doc);
    AddMethod("SetDefaultMeshManagementAttributes", visit_SetDefaultMeshManagementAttributes,
                                            visit_SetMeshManagementAttributes_doc);
    AddMethod("SetDefaultOperatorOptions", visit_SetDefaultOperatorOptions,
                                                 visit_SetOperatorOptions_doc);
    AddMethod("SetDefaultPickAttributes", visit_SetDefaultPickAttributes,
                                                  visit_SetPickAttributes_doc);
    AddMethod("SetDefaultPlotOptions", visit_SetDefaultPlotOptions,
                                                visit_SetPlotOptions_doc);
    AddMethod("SetDefaultQueryOverTimeAttributes", visit_SetDefaultQueryOverTimeAttributes);
    AddMethod("SetGlobalLineoutAttributes", visit_SetGlobalLineoutAttributes,
                                         visit_SetGlobalLineoutAttributes_doc);
    AddMethod("SetInteractorAttributes", visit_SetInteractorAttributes,
                                            visit_SetInteractorAttributes_doc);
    AddMethod("SetKeyframeAttributes", visit_SetKeyframeAttributes,
                                              visit_SetKeyframeAttributes_doc);
    AddMethod("SetMaterialAttributes", visit_SetMaterialAttributes,
                                            visit_SetMaterialAttributes_doc);
    AddMethod("SetMeshManagementAttributes", visit_SetMeshManagementAttributes,
                                        visit_SetMeshManagementAttributes_doc);
    AddMethod("SetOperatorOptions", visit_SetOperatorOptions,
                                                 visit_SetOperatorOptions_doc);
    AddMethod("SetPickAttributes", visit_SetPickAttributes,
                                                  visit_SetPickAttributes_doc);
    AddMethod("SetPipelineCachingMode", visit_SetPipelineCachingMode,
                                             visit_SetPipelineCachingMode_doc);
    AddMethod("SetPlotDatabaseState", visit_SetPlotDatabaseState,
                                               visit_SetPlotDatabaseState_doc);
    AddMethod("SetPlotFrameRange", visit_SetPlotFrameRange,
                                                  visit_SetPlotFrameRange_doc);
    AddMethod("SetPlotOptions", visit_SetPlotOptions,visit_SetPlotOptions_doc);
    AddMethod("SetPlotSILRestriction", visit_SetPlotSILRestriction,
                                              visit_SetPlotSILRestriction_doc);
    AddMethod("SetPrinterAttributes", visit_SetPrinterAttributes,
                                               visit_SetPrinterAttributes_doc);
    AddMethod("SetRenderingAttributes", visit_SetRenderingAttributes,
                                             visit_SetRenderingAttributes_doc);
    AddMethod("SetSaveWindowAttributes", visit_SetSaveWindowAttributes,
                                            visit_SetSaveWindowAttributes_doc);
    AddMethod("SetQueryOverTimeAttributes", visit_SetQueryOverTimeAttributes,
                                         visit_SetQueryOverTimeAttributes_doc);
    AddMethod("SetTimeSliderState", visit_SetTimeSliderState,
                                                 visit_SetTimeSliderState_doc);
    AddMethod("SetTreatAllDBsAsTimeVarying", visit_SetTreatAllDBsAsTimeVarying,
                                        visit_SetTreatAllDBsAsTimeVarying_doc);
    AddMethod("SetTryHarderCyclesTimes", visit_SetTryHarderCyclesTimes,
                                         visit_SetTryHarderCyclesTimes_doc);
    AddMethod("SetViewExtentsType", visit_SetViewExtentsType,
                                                 visit_SetViewExtentsType_doc);
    AddMethod("SetViewAxisArray", visit_SetViewAxisArray, visit_SetView_doc);
    AddMethod("SetViewCurve", visit_SetViewCurve, visit_SetView_doc);
    AddMethod("SetView2D", visit_SetView2D, visit_SetView_doc);
    AddMethod("SetView3D", visit_SetView3D, visit_SetView_doc);
    AddMethod("SetViewKeyframe", visit_SetViewKeyframe,
                                                    visit_SetViewKeyframe_doc);
          
    AddMethod("SetWindowArea", visit_SetWindowArea, visit_SetWindowArea_doc);
    AddMethod("SetWindowLayout", visit_SetWindowLayout,
                                                    visit_SetWindowLayout_doc);
    AddMethod("SetWindowMode", visit_SetWindowMode, visit_SetWindowMode_doc);
    AddMethod("ShowAllWindows", visit_ShowAllWindows,visit_ShowAllWindows_doc);
    AddMethod("ShowToolbars", visit_ShowToolbars, visit_ShowToolbars_doc);
    AddMethod("SuppressQueryOutputOn", visit_SuppressQueryOutputOn, visit_SuppressQueryOutput_doc);
    AddMethod("SuppressQueryOutputOff", visit_SuppressQueryOutputOff, visit_SuppressQueryOutput_doc);
    AddMethod("SetQueryFloatFormat", visit_SetQueryFloatFormat, visit_SetQueryFloatFormat_doc);
    AddMethod("TimeSliderGetNStates", visit_TimeSliderGetNStates,
                                               visit_TimeSliderGetNStates_doc);
    AddMethod("TimeSliderNextState", visit_TimeSliderNextState,
                                                visit_TimeSliderNextState_doc);
    AddMethod("TimeSliderPreviousState", visit_TimeSliderPreviousState,
                                                visit_TimeSliderPrevState_doc);
    AddMethod("TimeSliderSetState", visit_SetTimeSliderState,
                                                 visit_SetTimeSliderState_doc);
    AddMethod("ToggleBoundingBoxMode", visit_ToggleBoundingBoxMode,
                                                         visit_ToggleMode_doc);
    AddMethod("ToggleCameraViewMode", visit_ToggleCameraViewMode,
                                                         visit_ToggleMode_doc);
    AddMethod("ToggleFullFrameMode", visit_ToggleFullFrameMode,
                                                         visit_ToggleMode_doc);
    AddMethod("ToggleLockTime", visit_ToggleLockTime, visit_ToggleMode_doc);
    AddMethod("ToggleLockTools", visit_ToggleLockTools, visit_ToggleMode_doc);
    AddMethod("ToggleLockViewMode", visit_ToggleLockViewMode,
                                                         visit_ToggleMode_doc);
    AddMethod("ToggleMaintainViewMode", visit_ToggleMaintainViewMode,
                                                         visit_ToggleMode_doc);
    AddMethod("ToggleMaintainDataMode", visit_ToggleMaintainDataMode,
                                                         visit_ToggleMode_doc);
    AddMethod("ToggleSpinMode", visit_ToggleSpinMode, visit_ToggleMode_doc);
    AddMethod("UndoView",  visit_UndoView, visit_UndoView_doc);
    AddMethod("RedoView",  visit_RedoView, visit_RedoView_doc);
    AddMethod("WriteConfigFile",  visit_WriteConfigFile, 
                                                    visit_WriteConfigFile_doc);
    AddMethod("ZonePick", visit_Pick, visit_Pick_doc);

    //
    // Extra methods that are not part of the ViewerProxy but allow the
    // script writer to do interesting things.
    //
    AddMethod("Source", visit_Source, visit_Source_doc);
    AddMethod("ListPlots", visit_ListPlots, visit_ListPlots_doc);
    AddMethod("Expressions", visit_Expressions, visit_Expressions_doc);
    AddMethod("GetActiveTimeSlider", visit_GetActiveTimeSlider,
                                                visit_GetActiveTimeSlider_doc);
    AddMethod("GetDomains", visit_GetDomains, visit_GetDomains_doc);
    AddMethod("GetMaterials", visit_GetMaterials, visit_GetMaterials_doc);
    AddMethod("GetOperatorOptions", visit_GetOperatorOptions, NULL/*DOCUMENT ME*/);
    AddMethod("GetPlotOptions", visit_GetPlotOptions, NULL/*DOCUMENT ME*/);
    AddMethod("GetTimeSliders", visit_GetTimeSliders,visit_GetTimeSliders_doc);
    AddMethod("ListDomains", visit_ListDomains, visit_List_doc);
    AddMethod("ListMaterials", visit_ListMaterials, visit_List_doc);
    AddMethod("NumOperatorPlugins", visit_NumOperatorPlugins,
                                                 visit_NumOperatorPlugins_doc);
    AddMethod("NumPlotPlugins", visit_NumPlotPlugins,
                                                     visit_NumPlotPlugins_doc);
    AddMethod("OperatorPlugins", visit_OperatorPlugins,
                                                    visit_OperatorPlugins_doc);
    AddMethod("PlotPlugins", visit_PlotPlugins, visit_PlotPlugins_doc);
    AddMethod("Queries",  visit_Queries, visit_Queries_doc);
    AddMethod("SetDatabaseCorrelationOptions", visit_SetDatabaseCorrelationOptions,
                                      visit_SetDatabaseCorrelationOptions_doc);
    AddMethod("TurnDomainsOff", visit_TurnDomainsOff, visit_Turn_doc);
    AddMethod("TurnDomainsOn", visit_TurnDomainsOn, visit_Turn_doc);
    AddMethod("TurnMaterialsOff", visit_TurnMaterialsOff, visit_Turn_doc);
    AddMethod("TurnMaterialsOn", visit_TurnMaterialsOn, visit_Turn_doc);
    AddMethod("QueriesOverTime",  visit_QueriesOverTime,
                                                    visit_QueriesOverTime_doc);
    AddMethod("SetColorTexturingEnabled", visit_SetColorTexturingEnabled, NULL/*DOCUMENT ME*/);
    AddMethod("GetMetaData", visit_GetMetaData, NULL/*DOCUMENT ME*/);
    AddMethod("GetPlotList", visit_GetPlotList, NULL/*DOCUMENT ME*/);

    AddMethod("ClearMacros", visit_ClearMacros, NULL/*DOCUMENT ME*/);
    AddMethod("ExecuteMacro", visit_ExecuteMacro, NULL/*DOCUMENT ME*/);
    AddMethod("RegisterMacro", visit_RegisterMacro, NULL/*DOCUMENT ME*/);
    AddMethod("SuppressMessages", visit_SuppressMessages, visit_SuppressMessages_doc);

    AddMethod("GetCallbackNames", visit_GetCallbackNames, visit_GetCallbackNames_doc);
    AddMethod("RegisterCallback", visit_RegisterCallback, visit_RegisterCallback_doc);
    AddMethod("GetCallbackArgumentCount", visit_GetCallbackArgumentCount, NULL/*DOCUMENT ME*/);

    AddMethod("LoadAttribute", visit_LoadAttribute, visit_LoadSaveAttribute_doc);
    AddMethod("SaveAttribute", visit_SaveAttribute, visit_LoadSaveAttribute_doc);

    //
    // Lighting
    //
    AddMethod("GetLight", visit_GetLight, visit_GetLight_doc);
    AddMethod("SetLight", visit_SetLight, visit_SetLight_doc);

    // Temporary methods
    AddMethod("AddColorTable", visit_AddColorTable, NULL /*DOCUMENT ME*/);
    AddMethod("RemoveColorTable", visit_RemoveColorTable, NULL /*DOCUMENT ME*/);
    AddMethod("GetColorTable", visit_GetColorTable, NULL /*DOCUMENT ME*/);
    AddMethod("SetColorTable", visit_SetColorTable, NULL /*DOCUMENT ME*/);
    AddMethod("ColorTableNames", visit_ColorTableNames, 
                                                    visit_ColorTableNames_doc);
    AddMethod("NumColorTableNames", visit_NumColorTables,
                                                 visit_NumColorTableNames_doc);
    AddMethod("SetActiveContinuousColorTable", visit_SetActiveContinuousColorTable,
                                                visit_SetActiveColorTable_doc);
    AddMethod("SetActiveDiscreteColorTable", visit_SetActiveDiscreteColorTable,
                                                visit_SetActiveColorTable_doc);
    AddMethod("GetActiveContinuousColorTable", visit_GetActiveContinuousColorTable,
                                                visit_GetActiveColorTable_doc);
    AddMethod("GetActiveDiscreteColorTable", visit_GetActiveDiscreteColorTable,
                                                visit_GetActiveColorTable_doc);
    AddMethod("GetNumPlots", visit_GetNumPlots, visit_GetNumPlots_doc);
    AddMethod("Argv", visit_Argv, NULL);
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
//   Jeremy Meredith, Fri Oct 29 16:47:57 PDT 2004
//   Added constructor to support lighting (LightAttributes).
//
//   Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//   Added PyProcessAttributes
//
//   Hank Childs, Thu Jun 30 11:18:11 PDT 2005
//   Added PyExportDBAttributes.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Hank Childs, Mon Feb 13 21:22:43 PST 2006
//   Added ConstructDDFAttributes.
//
//   Brad Whitlock, Tue Mar 13 16:05:56 PST 2007
//   Added ColorControlPoint, ColorControlPointList, ColorAttribute, 
//   ColorAttributeList, GaussianControlPoint, GaussianControlPointList.
//
//   Brad Whitlock, Wed Dec 12 15:09:20 PST 2007
//   Added AnimationAttributes.
//
//   Brad Whitlock, Fri Jan 25 16:58:12 PST 2008
//   Added Axes2D Axes3D AxisAttributes AxisLabels AxisTickMarks AxisTitles 
//   FontAttributes. All are part of AnnotationAttributes.
//
//   Jeremy Meredith, Mon Feb  4 13:42:08 EST 2008
//   Added ViewAxisArrayAttributes.
//
// ****************************************************************************

static void
AddExtensions()
{
    int          i, nMethods;
    PyMethodDef *methods;

    ADD_EXTENSION(PyAnimationAttributes_GetMethodTable);
    ADD_EXTENSION(PyAnnotationAttributes_GetMethodTable);
    ADD_EXTENSION(PyColorAttribute_GetMethodTable);
    ADD_EXTENSION(PyColorAttributeList_GetMethodTable);
    ADD_EXTENSION(PyColorControlPoint_GetMethodTable);
    ADD_EXTENSION(PyColorControlPointList_GetMethodTable);
    ADD_EXTENSION(PyConstructDDFAttributes_GetMethodTable);
    ADD_EXTENSION(PyExportDBAttributes_GetMethodTable);
    ADD_EXTENSION(PyGaussianControlPoint_GetMethodTable);
    ADD_EXTENSION(PyGaussianControlPointList_GetMethodTable);
    ADD_EXTENSION(PyGlobalAttributes_GetMethodTable);
    ADD_EXTENSION(PyHostProfile_GetMethodTable);
    ADD_EXTENSION(PyMeshManagementAttributes_GetMethodTable);
    ADD_EXTENSION(PyMaterialAttributes_GetMethodTable);
    ADD_EXTENSION(PyPrinterAttributes_GetMethodTable);
    ADD_EXTENSION(PyProcessAttributes_GetMethodTable);
    ADD_EXTENSION(PyRenderingAttributes_GetMethodTable);
    ADD_EXTENSION(PySaveWindowAttributes_GetMethodTable);
    ADD_EXTENSION(PySILRestriction_GetMethodTable);
    ADD_EXTENSION(PyViewAttributes_GetMethodTable);
    ADD_EXTENSION(PyViewAxisArrayAttributes_GetMethodTable);
    ADD_EXTENSION(PyViewCurveAttributes_GetMethodTable);
    ADD_EXTENSION(PyView2DAttributes_GetMethodTable);
    ADD_EXTENSION(PyView3DAttributes_GetMethodTable);
    ADD_EXTENSION(PyWindowInformation_GetMethodTable);
    ADD_EXTENSION(PyLightAttributes_GetMethodTable);

    ADD_EXTENSION(PyAxes2D_GetMethodTable);
    ADD_EXTENSION(PyAxes3D_GetMethodTable);
    ADD_EXTENSION(PyAxisAttributes_GetMethodTable);
    ADD_EXTENSION(PyAxisLabels_GetMethodTable);
    ADD_EXTENSION(PyAxisTickMarks_GetMethodTable);
    ADD_EXTENSION(PyAxisTitles_GetMethodTable);
    ADD_EXTENSION(PyFontAttributes_GetMethodTable);

    // Note that we don't add PyDatabaseCorrelation's method table
    // because we don't want to be able to create them.
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
//   Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//   Added ProcessAttributes
//
//   Hank Childs, Thu Jun 30 11:18:11 PDT 2005
//   Added PyExportDBAttributes.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Brad Whitlock, Fri Jan 6 11:00:23 PDT 2006
//   I changed the 2nd argument in the StartUp function calls so it passes
//   a logging callback function instead of a pointer to a log file.
//
//   Hank Childs, Mon Feb 13 21:22:43 PST 2006
//   Added PyConstructDDFAttributes.
//
//   Brad Whitlock, Wed Dec 12 15:10:10 PST 2007
//   Added PyAnimationAttributes.
//
//   Jeremy Meredith, Mon Feb  4 13:42:08 EST 2008
//   Added ViewAxisArrayAttributes.
//
// ****************************************************************************

static void
InitializeExtensions()
{
    PyAnimationAttributes_StartUp(GetViewerState()->GetAnimationAttributes(), 0);
    PyAnnotationAttributes_StartUp(GetViewerState()->GetAnnotationAttributes(), 0);
    PyConstructDDFAttributes_StartUp(GetViewerState()->GetConstructDDFAttributes(), 0);
    PyExportDBAttributes_StartUp(GetViewerState()->GetExportDBAttributes(), 0);
    PyGlobalAttributes_StartUp(GetViewerState()->GetGlobalAttributes(), 0);
    PyHostProfile_StartUp(0, 0);
    PyMaterialAttributes_StartUp(GetViewerState()->GetMaterialAttributes(), 0);
    PyMeshManagementAttributes_StartUp(GetViewerState()->GetMeshManagementAttributes(), 0);
    PyPickAttributes_StartUp(GetViewerState()->GetPickAttributes(), 0);
    PyPrinterAttributes_StartUp(GetViewerState()->GetPrinterAttributes(), 0);
    PyProcessAttributes_StartUp(GetViewerState()->GetProcessAttributes(), 0);
    PyRenderingAttributes_StartUp(GetViewerState()->GetRenderingAttributes(), 0);
    PySaveWindowAttributes_StartUp(GetViewerState()->GetSaveWindowAttributes(), 0);
    PyWindowInformation_StartUp(GetViewerState()->GetWindowInformation(), 0);

    PyViewAxisArrayAttributes_StartUp(GetViewerState()->GetViewAxisArrayAttributes(), (void *)SS_log_ViewAxisArray);
    PyViewCurveAttributes_StartUp(GetViewerState()->GetViewCurveAttributes(), (void *)SS_log_ViewCurve);
    PyView2DAttributes_StartUp(GetViewerState()->GetView2DAttributes(), (void *)SS_log_View2D);
    PyView3DAttributes_StartUp(GetViewerState()->GetView3DAttributes(), (void *)SS_log_View3D);
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
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Brad Whitlock, Wed Dec 12 15:14:22 PST 2007
//   Added AnimationAttributes.
//
//   Jeremy Meredith, Mon Feb  4 13:42:08 EST 2008
//   Added ViewAxisArrayAttributes.
//
// ****************************************************************************

static void
CloseExtensions()
{
    PyAnimationAttributes_CloseDown();
    PyAnnotationAttributes_CloseDown();
    PyGlobalAttributes_CloseDown();
    PyMaterialAttributes_CloseDown();
    PyMeshManagementAttributes_CloseDown();
    PyPickAttributes_CloseDown();
    PyPrinterAttributes_CloseDown();
    PySaveWindowAttributes_CloseDown();
    PyViewAxisArrayAttributes_CloseDown();
    PyViewCurveAttributes_CloseDown();
    PyView2DAttributes_CloseDown();
    PyView3DAttributes_CloseDown();
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
//   Brad Whitlock, Thu Jan 5 15:14:48 PST 2006
//   The 2nd argument to the InitializePlugin method has changed from a file
//   pointer to a void *. In the plot plugins, the value is now used to
//   identify a callback function that gets called with a string representation
//   of the current attributes when they update from the viewer. We pass 0 for
//   the callback since we don't want to log plot attribute changes because
//   they are queried when writing log for SetPlotOptions, which ensures that
//   we only write them out when needed.
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//
// ****************************************************************************

static void
PlotPluginAddInterface()
{
    MUTEX_LOCK();

    // Get a pointer to the plot plugin manager.
    PlotPluginManager *pluginManager = GetViewerProxy()->GetPlotPluginManager();

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
        info->InitializePlugin(GetViewerState()->GetPlotAttributes(i), 0);

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
//   Brad Whitlock, Fri Jan 6 18:00:07 PST 2006
//   I changed the 2nd argument to InitializePlugin.
//
//   Brad Whitlock, Tue Jun 24 12:20:37 PDT 2008
//   Get the plugin manager via the viewer proxy.
//
// ****************************************************************************

static void
OperatorPluginAddInterface()
{
    MUTEX_LOCK();

    // Get a pointer to the plot plugin manager.
    OperatorPluginManager *pluginManager = GetViewerProxy()->GetOperatorPluginManager();

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
        info->InitializePlugin(GetViewerState()->GetOperatorAttributes(i), 0);

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
    debug1 << "DelayedLoadPlugins: start" << endl;

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
    GetViewerProxy()->LoadPlugins();

    debug1 << "DelayedLoadPlugins: end" << endl;
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
//   the call to VisItInit::Initialize because I wanted to add some debug stream
//   code into the termination function.
//
//   Brad Whitlock, Thu Dec 18 16:00:04 PST 2003
//   I added an observer that helps us do thread synchronization that does
//   not involve polling so we don't waste the CPU.
//
//   Brad Whitlock, Wed May 4 08:35:00 PDT 2005
//   I changed the for-loop to add arguments to the viewer proxy since the
//   the cli_argv array now has argv[0] in it so the ParentProcess inside of
//   the ViewerProxy can work.
//
//   Brad Whitlock, Tue Jan 10 11:57:31 PDT 2006
//   I made it use LogFile_open.
//
//   Brad Whitlock, Wed Nov 22 14:00:22 PST 2006
//   I changed the name of the log file so it does not interfere when we
//   use the module with "import visit". I also moved the code to read
//   the plugin directory until later.
//
//   Brad Whitlock, Tue May 8 13:31:20 PST 2007
//   I added code to honor -pid.
//
//   Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//   Moved setting of component name to before Initialize
//
//   Brad Whitlock, Tue Jun 24 13:51:41 PDT 2008
//   Pass the viewer proxy to the log callback function.
//
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Change some string literals to const char*'s.  For others,
//   assume VisItInit::Initialize won't modify argv, and simply convert one
//   some string literals to a char*.
//
//   Mark C. Miller, Thu Aug 21 11:32:08 PDT 2008
//   Added passing of '-clobber_vlogs' to Init
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
        char *argv[6];
        argv[0] = (char*)"cli";

        if(moduleDebugLevel > 0)
        {
            static const char *nums[] = {"1", "2", "3", "4", "5"};
            argv[argc++] = (char*)"-debug";
            argv[argc++] = (char*)nums[moduleDebugLevel - 1];
        }
        for(int i = 1; i < cli_argc; ++i)
        {
           if(strcmp(cli_argv[i], "-pid") == 0)
           {
               argv[argc++] = (char*)"-pid";
               break;
           }
           if(strcmp(cli_argv[i], "-clobber_vlogs") == 0)
           {
               argv[argc++] = (char*)"-clobber_vlogs";
               break;
           }
        }

        VisItInit::SetComponentName("cli");
        VisItInit::Initialize(argc, argv, 0, 1, false);
    }
    CATCH(VisItException)
    {
        // Return that we could not initialize VisIt.
        ret = true;
    }
    ENDTRY

    //
    // Create the viewer proxy and add some default arguments.
    //
    viewer = new ViewerProxy;

    //
    // Ensure that the viewer will be run in a mode that does not check for
    // interruption when reading back from the engine.
    //
    GetViewerProxy()->AddArgument("-noint");

    //
    // Add the debuglevel argument to the viewer proxy.
    //
    if(moduleDebugLevel > 0)
    {
        GetViewerProxy()->AddArgument("-debug");
        char tmp[10];
        SNPRINTF(tmp, 10, "%d", moduleDebugLevel);
        GetViewerProxy()->AddArgument(tmp);        
    }

    //
    // Add the optional command line arguments coming from cli_argv.
    //
    for(int i = 1; i < cli_argc; ++i)
        GetViewerProxy()->AddArgument(cli_argv[i]);

    //
    // Hook up observers
    //
    messageObserver = new VisItMessageObserver(GetViewerState()->GetMessageAttributes());
    statusObserver = new VisItStatusObserver(GetViewerState()->GetStatusAttributes());
    statusObserver->SetVerbose(moduleVerbose);
    pluginLoader = new ObserverToCallback(GetViewerState()->GetPluginManagerAttributes(),
                                          NeedToLoadPlugins);
    clientMethodObserver = new ObserverToCallback(GetViewerState()->GetClientMethod(),
                                          ExecuteClientMethodHelper);
    stateLoggingObserver = new ObserverToCallback(GetViewerState()->GetLogRPC(),
                                                  LogRPCs);
#ifndef POLLING_SYNCHRONIZE
    synchronizeCallback = new ObserverToCallback(GetViewerState()->GetSyncAttributes(),
                                                 WakeMainThread);
#endif

    // Set the macro string to empty.
    Macro_SetString("");

    //
    // Open the log file
    //
    const char *logName = "visitlog.py";
    if(!LogFile_Open(logName))
        fprintf(stderr, "Could not open %s log file.\n", logName);

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
// Function: ReadVisItPluginDir
//
// Purpose: 
//   Reads VISITPLUGINDIR that would be set up by the VisIt script that
//   is passed into this function.
//
// Arguments:
//   visitProgram : The fully qualified filename to the visit script that
//                  we want to run.
//
// Returns:    A new string containing the contents of VISITPLUGINDIR or
//             NULL if there was a problem.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 22 15:17:56 PST 2006
//
// Modifications:
//   
// ****************************************************************************

char *
ReadVisItPluginDir(const char *visitProgram)
{
    FILE *p = NULL;
    char *VISITPLUGINDIR = NULL;
    char line[2000];
    char *command =  NULL;
    int vpdLen = 0;
    const char *vpd = "VISITPLUGINDIR=";
    vpdLen = strlen(vpd);

    command = (char*)malloc(strlen(visitProgram) + 1 + strlen(" -env"));
    if(command == NULL)
        return NULL;
    sprintf(command, "%s -env", visitProgram);
#ifndef _WIN32
    p = popen(command, "r");
#else
    p = _popen(command, "r");
#endif
    if(p == NULL)
    {
        free(command);
        return NULL;
    }

    while(!feof(p))
    {
        fgets(line, 2000, p);
        if(strncmp(line, vpd, vpdLen) == 0)
        {
            char *value = NULL, *end = NULL;
            int len;
            value = line + vpdLen + 1;
            len = strlen(value);
            /* Trim off the newlines at the end.*/
            end = value + len;
            while(*end == '\0' || *end == '\n')
                *end-- = '\0';
            /* Copy the string. */
            VISITPLUGINDIR=(char *)malloc(len + 1);
            strcpy(VISITPLUGINDIR, value);
            break;
        }
    }
#ifndef _WIN32
    pclose(p);
#else
    _pclose(p);
#endif
    free(command);

    return VISITPLUGINDIR;
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
//   Brad Whitlock, Tue May 3 16:53:29 PST 2005
//   Added cli_argc, cli_argv to the Create call so we can do reverse
//   launching (viewer launching cli).
//
//   Brad Whitlock, Wed Nov 22 15:22:08 PST 2006
//   Added visitProgram argument and code to try and set VISITPLUGINDIR if
//   it has not been set already.
//
//   Brad Whitlock, Tue Jun 24 12:19:57 PDT 2008
//   Initialize the plugin managers via the viewer proxy.
//
// ****************************************************************************

static void
LaunchViewer(const char *visitProgram)
{
    // If noViewer is false at this stage then we have tried to launch
    // the viewer already and we should not try again.
    if(!noViewer)
        return;

    //
    // If we've not set up VISITPLUGINDIR then we're probably running
    // from "import visit" in a regular Python shell. Let's do our best
    // to set up VISITPLUGINDIR using the provided visitProgram.
    //
    bool freeVPD = false;
    char *VISITPLUGINDIR = getenv("VISITPLUGINDIR");
    if(VISITPLUGINDIR == NULL)
    {
        VISITPLUGINDIR = ReadVisItPluginDir(visitProgram);
        freeVPD = (VISITPLUGINDIR != NULL);
    }

    TRY
    {
        // Read the plugin info
        GetViewerProxy()->InitializePlugins(PlotPluginManager::Scripting, 
                                            VISITPLUGINDIR);
    }
    CATCH(VisItException)
    {
        if(freeVPD) 
            free(VISITPLUGINDIR);
        // Return since we could not initialize VisIt.
        CATCH_RETURN(1);
    }
    ENDTRY

    // Free the VISITPLUGINDIR array if we need to.
    if(freeVPD) 
        free(VISITPLUGINDIR);

    TRY
    {
        //
        // Try and connect to the viewer.
        //
        GetViewerProxy()->Create(visitProgram, &cli_argc, &cli_argv);

        //
        // Tell the windows to show themselves
        //
        GetViewerMethods()->ShowAllWindows();

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
//   Brad Whitlock, Thu Feb 2 13:20:48 PST 2006
//   I made the Windows implementation keep track of the thread handle so
//   we can later terminate it, if necessary.
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
    if((threadHandle = CreateThread(0, 0, visit_eventloop, (LPVOID)0, 0, &Id)) == INVALID_HANDLE_VALUE)
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
//   Brad Whitlock, Wed May 4 16:56:32 PST 2005
//   Added code to delete clientMethodObserver. I also made the CLI just
//   detach from the viewer instead of telling it to close.
//
//   Brad Whitlock, Thu Feb 2 13:23:51 PST 2006
//   Added code to wait for the listening thread to be finished or to terminate
//   it if we know that it's just waiting for viewer input
//   (viewerBlockingRead == true).
//
//   Brad Whitlock, Fri Feb  1 16:51:42 PST 2008
//   Close the callback manager.
//
// ****************************************************************************

static void
CloseModule()
{
#if defined(_WIN32)
    //
    // Wait for the reading thread to be done.
    //
    if(!viewerInitiatedQuit)
    {
        MUTEX_LOCK();
        keepGoing = false;
        MUTEX_UNLOCK();
    }
    else
        keepGoing = false;

    //
    // Sometimes it's not possible to wait for the reading thread to be
    // done - such as when it enters a blocking read for input from
    // the viewer at just the right moment. In that case, let's poll
    // until the thread is done or still active and we know it's
    // waiting for viewer input. If we know that it's waiting
    // for viewer input, terminate it so we can safely disconnect from
    // the viewer, without the 2nd thread wreaking havoc.
    //
    if(threadHandle != INVALID_HANDLE_VALUE && !viewerInitiatedQuit)
    {
        DWORD exitCode = STILL_ACTIVE;
        do
        {
            if(GetExitCodeThread(threadHandle, &exitCode) != 0)
            {
                if(exitCode == STILL_ACTIVE && viewerBlockingRead)
                {
                    TerminateThread(threadHandle, 0);
                    exitCode = 0;
                    viewerBlockingRead = false;
                }
            }
            else
                exitCode = 0;
        } while(exitCode == STILL_ACTIVE);
    }
#else
    keepGoing = false;
#endif

    // Delete the callback manager.
    if(callbackMgr != 0)
    {
        delete callbackMgr;
        callbackMgr = 0;
    }
    if(rpcCallbacks != 0)
    {
        delete rpcCallbacks;
        rpcCallbacks = 0;
    }

    // Delete the observers
    delete messageObserver;
    delete pluginLoader;
    delete clientMethodObserver;

    // Make each extension delete its observer
    debug1 << "Closing the extensions." << endl;
    CloseExtensions();

    // Close the viewer.
    if(viewer)
    {
        debug1 << "Telling the viewer to close." << endl;
        if(!viewerInitiatedQuit)
            GetViewerMethods()->Detach();
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
//   Brad Whitlock, Fri Jun 8 11:00:02 PDT 2007
//   Added argc,v_after_s so we can query which arguments to the cli came
//   after -s file.py.
//
// ****************************************************************************

void
cli_initvisit(int debugLevel, bool verbose, int argc, char **argv,
    int argc_after_s, char **argv_after_s)
{
    moduleDebugLevel = debugLevel;
    moduleVerbose = verbose;
    localNameSpace = true;
    cli_argc = argc;
    cli_argv = argv;
    cli_argc_after_s = argc_after_s;
    cli_argv_after_s = argv_after_s;
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
// Function: GetViewerProxy
//
// Purpose:
//   Gets the viewer proxy.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 11:57:53 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerProxy *
GetViewerProxy()
{
    return viewer;
}

// ****************************************************************************
// Function: GetViewerState
//
// Purpose: 
//   Returns a pointer to the viewer's state.
//
// Returns:    A pointer to the viewer's state
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 11:48:45 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
GetViewerState()
{
    return viewer->GetViewerState();
}

// ****************************************************************************
// Function: GetViewerMethods
//
// Purpose: 
//   Returns a pointer to the viewer's methods.
//
// Returns:    A pointer to the viewer's methods.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 11:48:05 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods *
GetViewerMethods()
{
    return viewer->GetViewerMethods();
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
//   Brad Whitlock, Wed May 4 19:15:05 PST 2005
//   Added code to get a pointer to the main thread state. I also added the
//   VisItInterrupt exception.
//
//   Brad Whitlock, Mon Jul 11 09:43:20 PDT 2005
//   Removed code to release the interpreter lock because it was unnecessary
//   and it was preventing code from running on Windows.
//
//   Brad Whitlock, Wed Nov 22 14:00:57 PST 2006
//   I removed the code to print an error and exit.
//
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Assyme PyErr_NewException won't modify its string argument and cast
//   literals to char*'s before passing in.
//
// ****************************************************************************

void
initvisit()
{
    int initCode = 0;

    // save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();

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
    VisItError = PyErr_NewException((char*)"visit.VisItException", NULL, NULL);
    PyDict_SetItemString(d, "VisItException", VisItError);
    VisItInterrupt = PyErr_NewException((char*)"visit.VisItInterrupt", NULL, NULL);
    PyDict_SetItemString(d, "VisItInterrupt", VisItInterrupt);

    // Define builtin visit functions that are written in python.
    initscriptfunctions();
}

// ****************************************************************************
// Method: initvisit2
//
// Purpose: 
//   Same as initvisit except that if the "visit" module is not already
//   available then we add functions to the global namespace instead of
//   to the "visit" module.
//
// Note:       This method is only called from the "front end" visit module
//             that we use from a standard Python interpreter. It differs
//             from initvisit because we may be doing "import visit" or
//             "from visit import *" in the standard Python interpreter and
//             since we're doing something a little funny (clobbering the
//             front end interface's methods with these) we need to know
//             which namespace to use. If the front end VisIt module was
//             imported using "import visit" then there will be a module
//             entry called "visit" in the global symbols and we want to
//             append functions to it. Otherwise, just append the functions
//             to the global namespace.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 11:37:05 PDT 2006
//
// Modifications:
//   Jeremy Meredith, Thu Aug  7 15:06:45 EDT 2008
//   Assyme PyErr_NewException won't modify its string argument and cast
//   literals to char*'s before passing in.
//   
// ****************************************************************************

void
initvisit2()
{
    int initCode = 0;
    PyObject *d = NULL;

    // save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();

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

    d = PyEval_GetLocals();

    if(PyDict_GetItemString(d, "visit") != NULL)
    {
        // Add the VisIt module to Python. Note that we're passing the address
        // of the first element of a vector.
        visitModule = Py_InitModule("visit", &VisItMethods[0]);

        // Add the Python error message.
        d = PyModule_GetDict(visitModule);
    }
    else
    {
        // Add the VisIt module's functions to the global namespace.
        localNameSpace = true;
        d = PyEval_GetLocals();
        for(int i = 0; i < VisItMethods.size(); ++i)
        {
            if(VisItMethods[i].ml_name != NULL)
            {
                PyObject *v = PyCFunction_New(&VisItMethods[i], Py_None);
                if(v == NULL)
                    continue;
                if(PyDict_SetItemString(d, VisItMethods[i].ml_name, v) != 0)
                    continue;
                Py_DECREF(v);
            }
        }
    }

    VisItError = PyErr_NewException((char*)"visit.VisItException", NULL, NULL);
    PyDict_SetItemString(d, "VisItException", VisItError);
    VisItInterrupt = PyErr_NewException((char*)"visit.VisItInterrupt", NULL, NULL);
    PyDict_SetItemString(d, "VisItInterrupt", VisItInterrupt);

    // Define builtin visit functions that are written in python.
    initscriptfunctions();
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
//   Brad Whitlock, Tue Jan 10 11:57:03 PDT 2006
//   I made it use LogFile_Close.
//
// ****************************************************************************

static void
terminatevisit()
{
    // Delete the VisIt objects.
    CloseModule();

    // Close the log file
    LogFile_Close();

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
//   Brad Whitlock, Thu Feb 2 13:21:41 PST 2006
//   I set the viewerBlockingRead flag when the 2nd thread is doing nothing
//   but waiting for input from the viewer. This helps us terminate the
//   thread later if we need to.
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
        viewerBlockingRead = true;
        if(GetViewerProxy()->GetWriteConnection()->NeedsRead(true))
        {
            viewerBlockingRead = false;
            TRY
            {
                // Process input.
                MUTEX_LOCK();
                    GetViewerProxy()->ProcessInput();
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

    viewerBlockingRead = false;

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
//   Brad Whitlock, Tue Jun 21 10:59:04 PDT 2005
//   I changed the synchronization code so it uses the original viewer mutex
//   instead of 2 mutexes so the code won't deadlock on some systems. I also
//   added support for script interruption.
//
//   Brad Whitlock, Tue Jan 10 12:06:50 PDT 2006
//   I changed logging.
//
//   Brad Whitlock, Mon Mar 20 12:57:13 PDT 2006
//   I made mutex unlocking be part of SYNC_COND_WAIT because the win32
//   implementation has to have the mutex be unlocked at the start while the
//   UNIX version unlocks it at the end. Making the mutex unlocking part of 
//   the macro allows us to put it where we want.
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

    // Disable logging.
    bool logEnabled = LogFile_GetEnabled();
    if(logEnabled)
        LogFile_SetEnabled(false);

    // Send the syncAtts to the viewer and then set the proxy's syncAtts tag
    // to -1. This will allow us to loop until the viewer sends back the
    // correct value.
    MUTEX_LOCK();
    ++syncCount;
    SyncAttributes *syncAtts = GetViewerState()->GetSyncAttributes();
    syncAtts->SetSyncTag(syncCount);
#ifndef POLLING_SYNCHRONIZE
    synchronizeCallback->SetUpdate(false);
#endif
    syncAtts->Notify();
    syncAtts->SetSyncTag(-1);

#ifndef POLLING_SYNCHRONIZE
    SYNC_COND_WAIT();
#else
    MUTEX_UNLOCK();
    while((syncAtts->GetSyncTag() != syncCount) && keepGoing)
    {
        // Nothing here.
    }
#endif

    // Enable logging.
    if(logEnabled)
        LogFile_SetEnabled(true);

    // If the viewer has terminated while we were waiting for the sync tag
    // then call the error function now that we're in thread 1.
    int retval = keepGoing ? messageObserver->ErrorFlag() : -1;
    if(!keepGoing)
    {
        VisItErrorFunc(terminationMsg);
    }

    // If we interrupted the script via the Interrupt client method then
    // reset the interruptScript flag and make sure we return an error value.
    if(interruptScript)
    {
        PyErr_SetString(VisItInterrupt, "Interrupted script");
        interruptScript = false;
        retval = -1;
    }

    // Return whether or not there is an error in the message observer.
    // Also indicate an error if the viewer is dead.
    return retval;
}
