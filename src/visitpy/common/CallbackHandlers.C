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
#include <CallbackHandlers.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <ViewerRPCCallbacks.h>

extern PyObject *args_ViewerRPC(ViewerRPC *rpc);

//
// These are the supported state objects on which we can register callbacks.
// It's not the whole list from ViewerState because we don't have Python bindings
// for every state object. This macro loosely follows the one in ViewerState.h.
// The first argument is the name of the function that gets an object from 
// ViewerState, minus the "Get". The second argument is the name of the type.
//
#define SUPPORTED_STATE_OBJECTS \
CALLBACK_ACTION(AnimationAttributes,      AnimationAttributes,      PyAnimationAttributes_Wrap) \
CALLBACK_ACTION(AnnotationAttributes,     AnnotationAttributes,     PyAnnotationAttributes_Wrap) \
CALLBACK_ACTION(ColorTableAttributes,     ColorTableAttributes,     PyColorTableAttributes_Wrap) \
CALLBACK_ACTION(ConstructDDFAttributes,   ConstructDDFAttributes,   PyConstructDDFAttributes_Wrap) \
CALLBACK_ACTION(ExportDBAttributes,       ExportDBAttributes,       PyExportDBAttributes_Wrap) \
CALLBACK_ACTION(ExpressionList,           ExpressionList,           PyExpressionList_Wrap) \
CALLBACK_ACTION(FileOpenOptions,          FileOpenOptions,          PyFileOpenOptions_Wrap) \
CALLBACK_ACTION(GlobalAttributes,         GlobalAttributes,         PyGlobalAttributes_Wrap) \
CALLBACK_ACTION(GlobalLineoutAttributes,  GlobalLineoutAttributes,  PyGlobalLineoutAttributes_Wrap) \
CALLBACK_ACTION(InteractorAttributes,     InteractorAttributes,     PyInteractorAttributes_Wrap) \
CALLBACK_ACTION(KeyframeAttributes,       KeyframeAttributes,       PyKeyframeAttributes_Wrap) \
CALLBACK_ACTION(MaterialAttributes,       MaterialAttributes,       PyMaterialAttributes_Wrap) \
CALLBACK_ACTION(MeshManagementAttributes, MeshManagementAttributes, PyMeshManagementAttributes_Wrap) \
CALLBACK_ACTION(PickAttributes,           PickAttributes,           PyPickAttributes_Wrap) \
CALLBACK_ACTION(PlotList,                 PlotList,                 PyPlotList_Wrap) \
CALLBACK_ACTION(PrinterAttributes,        PrinterAttributes,        PyPrinterAttributes_Wrap) \
CALLBACK_ACTION(ProcessAttributes,        ProcessAttributes,        PyProcessAttributes_Wrap) \
CALLBACK_ACTION(QueryAttributes,          QueryAttributes,          PyQueryAttributes_Wrap) \
CALLBACK_ACTION(QueryOverTimeAttributes,  QueryOverTimeAttributes,  PyQueryOverTimeAttributes_Wrap) \
CALLBACK_ACTION(RenderingAttributes,      RenderingAttributes,      PyRenderingAttributes_Wrap) \
CALLBACK_ACTION(SaveWindowAttributes,     SaveWindowAttributes,     PySaveWindowAttributes_Wrap) \
CALLBACK_ACTION(View2DAttributes,         View2DAttributes,         PyView2DAttributes_Wrap) \
CALLBACK_ACTION(View3DAttributes,         View3DAttributes,         PyView3DAttributes_Wrap) \
CALLBACK_ACTION(ViewCurveAttributes,      ViewCurveAttributes,      PyViewCurveAttributes_Wrap) \
CALLBACK_ACTION(WindowInformation,        WindowInformation,        PyWindowInformation_Wrap) \
CALLBACK_ACTION(DatabaseMetaData,         avtDatabaseMetaData,      PyavtDatabaseMetaData_Wrap)

// Includes for above.
#include <PyAnimationAttributes.h>
#include <PyAnnotationAttributes.h>
#include <PyColorTableAttributes.h>
#include <PyConstructDDFAttributes.h>
#include <PyExportDBAttributes.h>
#include <PyExpressionList.h>
#include <PyFileOpenOptions.h>
#include <PyGlobalAttributes.h>
#include <PyGlobalLineoutAttributes.h>
#include <PyInteractorAttributes.h>
#include <PyKeyframeAttributes.h>
#include <PyMaterialAttributes.h>
#include <PyMeshManagementAttributes.h>
#include <PyPickAttributes.h>
#include <PyPlotList.h>
#include <PyPrinterAttributes.h>
#include <PyProcessAttributes.h>
#include <PyQueryAttributes.h>
#include <PyQueryOverTimeAttributes.h>
#include <PyRenderingAttributes.h>
#include <PySaveWindowAttributes.h>
#include <PyView2DAttributes.h>
#include <PyView3DAttributes.h>
#include <PyViewCurveAttributes.h>
#include <PyViewerRPC.h>
#include <PyWindowInformation.h>
#include <PyavtDatabaseMetaData.h>

//
// Define some default handler functions for various state objects so we can
// associate Python callbacks with those state objects.
//
#define CALLBACK_ACTION(Obj, T, PyWrap) \
static void \
default_handler_##Obj(Subject *subj, void *data) \
{ \
    CallbackManager::CallbackHandlerData *cbData = (CallbackManager::CallbackHandlerData *)data; \
    if(cbData->pycb != 0) \
    { \
        PyObject *tuple = PyTuple_New(1);\
        PyObject *state = PyWrap((const T *)cbData->data); \
        PyTuple_SET_ITEM(tuple, 0, state); \
        PyObject *ret = PyObject_Call(cbData->pycb, tuple, NULL); \
        Py_DECREF(tuple); \
        Py_DECREF(ret); \
    } \
}
SUPPORTED_STATE_OBJECTS
#undef CALLBACK_ACTION

// ****************************************************************************
// Method: GetPlotConstructorFunction
//
// Purpose: 
//   Returns the constructor function for a plot subject from its Python
//   scripting interface.
//
// Arguments:
//   subj : The subject for which we want a Python constructor function.
//
// Returns:    A PyObject that is a callable Python function.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:18:13 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static PyObject *
GetPlotConstructorFunction(AttributeSubject *subj)
{
    PyObject *retval = 0;
    PlotPluginManager *pluginManager = PlotPluginManager::Instance();

    int pluginIndex = -1;
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingPlotPluginInfo *info=pluginManager->GetScriptingPluginInfo(id);
        if(info == 0)
            continue;
        std::string attName(info->GetName());
        attName +=  "Attributes";
        if(attName == subj->TypeName())
        {
            pluginIndex = i;
            break;
        }
    }

    if(pluginIndex != -1)
    {
        // Get a pointer to the scripting portion of the plot plugin information.
        std::string id(pluginManager->GetEnabledID(pluginIndex));
        ScriptingPlotPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        // Get the plugin's method table.
        int nMethods = 0;
        PyMethodDef *method = (PyMethodDef *)info->GetMethodTable(&nMethods);;
        for(int m = 0; m < nMethods; ++m, ++method)
        {
            // Can we locate the constructor function? It should be first.
            if(subj->TypeName() == method->ml_name)
            {
                retval = PyCFunction_New(method, Py_None);
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: GetOperatorConstructorFunction
//
// Purpose: 
//   Returns the constructor function for an operator subject from its Python
//   scripting interface.
//
// Arguments:
//   subj : The subject for which we want a Python constructor function.
//
// Returns:    A PyObject that is a callable Python function.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:18:13 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static PyObject *
GetOperatorConstructorFunction(AttributeSubject *subj)
{
    PyObject *retval = 0;
    OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();

    int pluginIndex = -1;
    for(int i = 0; i < pluginManager->GetNEnabledPlugins(); ++i)
    {
        std::string id(pluginManager->GetEnabledID(i));
        ScriptingOperatorPluginInfo *info=pluginManager->GetScriptingPluginInfo(id);
        if(info == 0)
            continue;
        std::string attName(info->GetName());
        attName +=  "Attributes";
        if(attName == subj->TypeName())
        {
            pluginIndex = i;
            break;
        }
    }

    if(pluginIndex != -1)
    {
        OperatorPluginManager *pluginManager = OperatorPluginManager::Instance();
        // Get a pointer to the scripting portion of the Operator plugin information.
        std::string id(pluginManager->GetEnabledID(pluginIndex));
        ScriptingOperatorPluginInfo *info = pluginManager->GetScriptingPluginInfo(id);

        // Get the plugin's method table.
        int nMethods = 0;
        PyMethodDef *method = (PyMethodDef *)info->GetMethodTable(&nMethods);;
        for(int m = 0; m < nMethods; ++m, ++method)
        {
            // Can we locate the constructor function? It should be first.
            if(subj->TypeName() == method->ml_name)
            {
                retval = PyCFunction_New(method, Py_None);
                break;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: plugin_state_callback_handler
//
// Purpose: 
//   This is the handler function that gets called when plot and operator
//   state objects update. This function dispatches the update to the user-defined
//   Python callback function for the appropriate state object.
//
// Arguments:
//   s    : The subject that updated.
//   data : CallbackHandlerData from the CallbackManager.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:20:33 PST 2008
//
// Modifications:
//   
// ****************************************************************************

//
// This struct follows the plan of all of the generated VisIt state objects. 
// This is for a HACK!
//
struct PyVisItStateObject
{
    PyObject_HEAD
    AttributeSubject *data;
    bool owns;
};

static void
plugin_state_callback_handler(Subject *s, void *data)
{
    AttributeSubject *subj = (AttributeSubject *)s;
    CallbackManager::CallbackHandlerData *cbData = (CallbackManager::CallbackHandlerData *)data;
    if(cbData->pycb != 0)
    {
        PyObject *ctor = GetPlotConstructorFunction(subj);
        if(ctor == 0)
            ctor = GetOperatorConstructorFunction(subj);
        if(ctor == 0)
            return;

        // We have a contructor function by now. Let's call it with a 0
        // as the argument so we instantiate a new object based on the
        // default attributes.
        PyObject *tuple = PyTuple_New(1);
        PyTuple_SET_ITEM(tuple, 0, PyLong_FromLong(0L));
        PyObject *state = PyObject_Call(ctor, tuple, NULL);
        Py_DECREF(tuple);
        Py_DECREF(ctor);

        // HACK! We had to save off the value of the subject that generated the 
        //       callback because who knows what those values are right now. We're
        //       on a different thread here, after all. Since we have that object
        //       but no plugin-formalized way of wrapping it to PyObject, we need
        //       to be able to poke the object that we care about into the 
        //       PyObject that we pass to the user callback function. To do so, 
        //       we define PyVisItStateObject, which is for all intensive purposes
        //       identical to the object declarations in all of our VisIt Python
        //       bindings.
        PyVisItStateObject *s = (PyVisItStateObject *)state;
        AttributeSubject *oldState = s->data;
        s->data = cbData->data;

        // Now that we've wrapped the state object, call the user's
        // Python callback function.
        tuple = PyTuple_New(1);
        PyTuple_SET_ITEM(tuple, 0, state);
        PyObject *ret = PyObject_Call(cbData->pycb, tuple, NULL);
        // Restore the old state object
        s->data = oldState;
        // Delete the tuple (includes state)
        Py_DECREF(tuple);
    }
}

// ****************************************************************************
// Method: ViewerRPC_callback
//
// Purpose: 
//   The handler that gets called when we get a ViewerRPC.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  4 09:48:11 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static void
ViewerRPC_callback(Subject *subj, void *data)
{
    CallbackManager::CallbackHandlerData *cbData = (CallbackManager::CallbackHandlerData *)data;
    // If we have a handler for the particular ViewerRPC then call it.
    ViewerRPC *rpc = (ViewerRPC *)cbData->data;
    ViewerRPCCallbacks *rpcCB = (ViewerRPCCallbacks *)cbData->userdata;
    PyObject *pycb = rpcCB->GetCallback(rpc->GetRPCType());
    if(pycb != 0)
    {
        // Get the arguments for the rpc so we can pass them to the user's callback.
        PyObject *args = args_ViewerRPC(rpc);

        // Call the user's callback function.
        PyObject *ret = PyObject_Call(pycb, args, NULL);

        // Delete the args
        Py_DECREF(args);

        // Delete the return value.
        Py_DECREF(ret);
    }
}

// ****************************************************************************
// Method: ViewerRPC_addwork_callback
//
// Purpose: 
//   Called by the callback manager during an update in order to determine
//   whether the update should generate callback function work.
//
// Arguments:
//   subj : The ViewerRPC.
//   ptr  : User data provided when this function was registered with the
//          callback manager. In this case, ptr aliases ViewerRPCCallbacks,
//          which is the object that lets us install handlers for individual
//          ViewerRPC values.
//
// Returns:    True if work should be added (if there's a Python callback 
//             installed for the rpc in question). False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:22:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static bool
ViewerRPC_addwork_callback(Subject *subj, void *ptr)
{
    ViewerRPC *rpc = (ViewerRPC *)subj;
    ViewerRPCCallbacks *cb = (ViewerRPCCallbacks *)ptr;
    // Only add work if we have a Python callback for the rpc.
    return cb->GetCallback(rpc->GetRPCType()) != 0;
}

// ****************************************************************************
// Method: StateObject_addwork_callback
//
// Purpose: 
//   Called by the callback manager during an update in order to determine
//   whether the update should generate callback function work.
//
// Arguments:
//   subj : The subject that generated the update.
//   ptr  : User data provided when this function was registered with the
//          callback manager. In this case, ptr provides us with a pointer
//          to the callback manager itself.
//
// Returns:    True if work should be added (if there's a Python callback 
//             installed for the state object in question). False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:22:14 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static bool
StateObject_addwork_callback(Subject *subj, void *ptr)
{
    CallbackManager *mgr = (CallbackManager *)ptr;
    // Only add work if we have a Python callback for the state object.
    return mgr->GetCallback(subj) != 0;
}

// ****************************************************************************
// Method: RegisterCallbackHandlers
//
// Purpose: 
//   This function installs the callback handler functions for the supported
//   state objects.
//
// Arguments:
//   cb    : The callback manager.
//   vs    : The viewer state.
//   rpcCB : The ViewerRPC callback object.
//
// Returns:    
//
// Note:   This function gets called from thread 1 in the CLI    
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:25:43 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
RegisterCallbackHandlers(CallbackManager *cb, ViewerState *vs, ViewerRPCCallbacks *rpcCB)
{
    // Register a special handler for ViewerRPC since it will dispatch to 
    // further Python callbacks. We don't give it a name so the user can't
    // register a handler to override the one that we provide.
    cb->RegisterHandler((Subject*)vs->GetLogRPC(), 
                         "",
                         ViewerRPC_callback, (void*)rpcCB, 
                         ViewerRPC_addwork_callback, (void*)rpcCB);

    // Register handlers for the supported state objects.
#define CALLBACK_ACTION(Obj, T, PyWrap) \
    cb->RegisterHandler((Subject *)vs->Get##Obj(), \
                        #Obj, \
                        default_handler_##Obj, NULL, \
                        StateObject_addwork_callback, (void*)cb);
SUPPORTED_STATE_OBJECTS
#undef CALLBACK_ACTION

    // Register a handler for the plot state objects.
    for(int i = 0; i < vs->GetNumPlotStateObjects(); ++i)
    {
        cb->RegisterHandler((Subject*)vs->GetPlotAttributes(i),
                            vs->GetPlotAttributes(i)->TypeName(),
                            plugin_state_callback_handler, NULL,
                            StateObject_addwork_callback, (void*)cb);
    }

    // Register a handler for the operator state objects.
    for(int i = 0; i < vs->GetNumOperatorStateObjects(); ++i)
    {
        cb->RegisterHandler((Subject*)vs->GetOperatorAttributes(i),
                            vs->GetOperatorAttributes(i)->TypeName(),
                            plugin_state_callback_handler, NULL,
                            StateObject_addwork_callback, (void*)cb);
    }
}
